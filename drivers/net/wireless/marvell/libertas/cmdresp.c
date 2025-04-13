// SPDX-License-Identifier: GPL-2.0
/*
 * This file contains the handling of command
 * responses as well as events generated by firmware.
 */

#include <linux/hardirq.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/unaligned.h>
#include <net/cfg80211.h>

#include "cfg.h"
#include "cmd.h"

/**
 * lbs_mac_event_disconnected - handles disconnect event. It
 * reports disconnect to upper layer, clean tx/rx packets,
 * reset link state etc.
 *
 * @priv:	A pointer to struct lbs_private structure
 * @locally_generated: indicates disconnect was requested locally
 *		(usually by userspace)
 *
 * returns:	n/a
 */
void lbs_mac_event_disconnected(struct lbs_private *priv,
				bool locally_generated)
{
	unsigned long flags;

	if (priv->connect_status != LBS_CONNECTED)
		return;

	/*
	 * Cisco AP sends EAP failure and de-auth in less than 0.5 ms.
	 * It causes problem in the Supplicant
	 */
	msleep_interruptible(1000);

	if (priv->wdev->iftype == NL80211_IFTYPE_STATION)
		lbs_send_disconnect_notification(priv, locally_generated);

	/* report disconnect to upper layer */
	netif_stop_queue(priv->dev);
	netif_carrier_off(priv->dev);

	/* Free Tx and Rx packets */
	spin_lock_irqsave(&priv->driver_lock, flags);
	dev_kfree_skb_irq(priv->currenttxskb);
	priv->currenttxskb = NULL;
	priv->tx_pending_len = 0;
	spin_unlock_irqrestore(&priv->driver_lock, flags);

	priv->connect_status = LBS_DISCONNECTED;

	if (priv->psstate != PS_STATE_FULL_POWER) {
		/* make firmware to exit PS mode */
		lbs_deb_cmd("disconnected, so exit PS mode\n");
		lbs_set_ps_mode(priv, PS_MODE_ACTION_EXIT_PS, false);
	}
}

int lbs_process_command_response(struct lbs_private *priv, u8 *data, u32 len)
{
	uint16_t respcmd, curcmd;
	struct cmd_header *resp;
	int ret = 0;
	unsigned long flags;
	uint16_t result;

	mutex_lock(&priv->lock);
	spin_lock_irqsave(&priv->driver_lock, flags);

	if (!priv->cur_cmd) {
		lbs_deb_host("CMD_RESP: cur_cmd is NULL\n");
		ret = -1;
		spin_unlock_irqrestore(&priv->driver_lock, flags);
		goto done;
	}

	resp = (void *)data;
	curcmd = le16_to_cpu(priv->cur_cmd->cmdbuf->command);
	respcmd = le16_to_cpu(resp->command);
	result = le16_to_cpu(resp->result);

	lbs_deb_cmd("CMD_RESP: response 0x%04x, seq %d, size %d\n",
		     respcmd, le16_to_cpu(resp->seqnum), len);
	lbs_deb_hex(LBS_DEB_CMD, "CMD_RESP", (void *) resp, len);

	if (resp->seqnum != priv->cur_cmd->cmdbuf->seqnum) {
		netdev_info(priv->dev,
			    "Received CMD_RESP with invalid sequence %d (expected %d)\n",
			    le16_to_cpu(resp->seqnum),
			    le16_to_cpu(priv->cur_cmd->cmdbuf->seqnum));
		spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}
	if (respcmd != CMD_RET(curcmd) &&
	    respcmd != CMD_RET_802_11_ASSOCIATE && curcmd != CMD_802_11_ASSOCIATE) {
		netdev_info(priv->dev, "Invalid CMD_RESP %x to command %x!\n",
			    respcmd, curcmd);
		spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	if (resp->result == cpu_to_le16(0x0004)) {
		/* 0x0004 means -EAGAIN. Drop the response, let it time out
		   and be resubmitted */
		netdev_info(priv->dev,
			    "Firmware returns DEFER to command %x. Will let it time out...\n",
			    le16_to_cpu(resp->command));
		spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	/* Now we got response from FW, cancel the command timer */
	timer_delete(&priv->command_timer);
	priv->cmd_timed_out = 0;

	if (respcmd == CMD_RET(CMD_802_11_PS_MODE)) {
		/* struct cmd_ds_802_11_ps_mode also contains
		 * the header
		 */
		struct cmd_ds_802_11_ps_mode *psmode = (void *)resp;
		u16 action = le16_to_cpu(psmode->action);

		lbs_deb_host(
		       "CMD_RESP: PS_MODE cmd reply result 0x%x, action 0x%x\n",
		       result, action);

		if (result) {
			lbs_deb_host("CMD_RESP: PS command failed with 0x%x\n",
				    result);
			/*
			 * We should not re-try enter-ps command in
			 * ad-hoc mode. It takes place in
			 * lbs_execute_next_command().
			 */
			if (priv->wdev->iftype == NL80211_IFTYPE_MONITOR &&
			    action == PS_MODE_ACTION_ENTER_PS)
				priv->psmode = LBS802_11POWERMODECAM;
		} else if (action == PS_MODE_ACTION_ENTER_PS) {
			priv->needtowakeup = 0;
			priv->psstate = PS_STATE_AWAKE;

			lbs_deb_host("CMD_RESP: ENTER_PS command response\n");
			if (priv->connect_status != LBS_CONNECTED) {
				/*
				 * When Deauth Event received before Enter_PS command
				 * response, We need to wake up the firmware.
				 */
				lbs_deb_host(
				       "disconnected, invoking lbs_ps_wakeup\n");

				spin_unlock_irqrestore(&priv->driver_lock, flags);
				mutex_unlock(&priv->lock);
				lbs_set_ps_mode(priv, PS_MODE_ACTION_EXIT_PS,
						false);
				mutex_lock(&priv->lock);
				spin_lock_irqsave(&priv->driver_lock, flags);
			}
		} else if (action == PS_MODE_ACTION_EXIT_PS) {
			priv->needtowakeup = 0;
			priv->psstate = PS_STATE_FULL_POWER;
			lbs_deb_host("CMD_RESP: EXIT_PS command response\n");
		} else {
			lbs_deb_host("CMD_RESP: PS action 0x%X\n", action);
		}

		__lbs_complete_command(priv, priv->cur_cmd, result);
		spin_unlock_irqrestore(&priv->driver_lock, flags);

		ret = 0;
		goto done;
	}

	/* If the command is not successful, cleanup and return failure */
	if ((result != 0 || !(respcmd & 0x8000))) {
		lbs_deb_host("CMD_RESP: error 0x%04x in command reply 0x%04x\n",
		       result, respcmd);
		/*
		 * Handling errors here
		 */
		switch (respcmd) {
		case CMD_RET(CMD_GET_HW_SPEC):
		case CMD_RET(CMD_802_11_RESET):
			lbs_deb_host("CMD_RESP: reset failed\n");
			break;

		}
		__lbs_complete_command(priv, priv->cur_cmd, result);
		spin_unlock_irqrestore(&priv->driver_lock, flags);

		ret = -1;
		goto done;
	}

	spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (priv->cur_cmd && priv->cur_cmd->callback) {
		ret = priv->cur_cmd->callback(priv, priv->cur_cmd->callback_arg,
				resp);
	}

	spin_lock_irqsave(&priv->driver_lock, flags);

	if (priv->cur_cmd) {
		/* Clean up and Put current command back to cmdfreeq */
		__lbs_complete_command(priv, priv->cur_cmd, result);
	}
	spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	mutex_unlock(&priv->lock);
	return ret;
}

void lbs_process_event(struct lbs_private *priv, u32 event)
{
	struct cmd_header cmd;

	switch (event) {
	case MACREG_INT_CODE_LINK_SENSED:
		lbs_deb_cmd("EVENT: link sensed\n");
		break;

	case MACREG_INT_CODE_DEAUTHENTICATED:
		lbs_deb_cmd("EVENT: deauthenticated\n");
		lbs_mac_event_disconnected(priv, false);
		break;

	case MACREG_INT_CODE_DISASSOCIATED:
		lbs_deb_cmd("EVENT: disassociated\n");
		lbs_mac_event_disconnected(priv, false);
		break;

	case MACREG_INT_CODE_LINK_LOST_NO_SCAN:
		lbs_deb_cmd("EVENT: link lost\n");
		lbs_mac_event_disconnected(priv, true);
		break;

	case MACREG_INT_CODE_PS_SLEEP:
		lbs_deb_cmd("EVENT: ps sleep\n");

		/* handle unexpected PS SLEEP event */
		if (priv->psstate == PS_STATE_FULL_POWER) {
			lbs_deb_cmd(
			       "EVENT: in FULL POWER mode, ignoring PS_SLEEP\n");
			break;
		}
		if (!list_empty(&priv->cmdpendingq)) {
			lbs_deb_cmd("EVENT: commands in queue, do not sleep\n");
			break;
		}
		priv->psstate = PS_STATE_PRE_SLEEP;

		lbs_ps_confirm_sleep(priv);

		break;

	case MACREG_INT_CODE_HOST_AWAKE:
		lbs_deb_cmd("EVENT: host awake\n");
		if (priv->reset_deep_sleep_wakeup)
			priv->reset_deep_sleep_wakeup(priv);
		priv->is_deep_sleep = 0;
		lbs_cmd_async(priv, CMD_802_11_WAKEUP_CONFIRM, &cmd,
				sizeof(cmd));
		priv->is_host_sleep_activated = 0;
		wake_up_interruptible(&priv->host_sleep_q);
		break;

	case MACREG_INT_CODE_DEEP_SLEEP_AWAKE:
		if (priv->reset_deep_sleep_wakeup)
			priv->reset_deep_sleep_wakeup(priv);
		lbs_deb_cmd("EVENT: ds awake\n");
		priv->is_deep_sleep = 0;
		wake_up_interruptible(&priv->ds_awake_q);
		break;

	case MACREG_INT_CODE_PS_AWAKE:
		lbs_deb_cmd("EVENT: ps awake\n");
		/* handle unexpected PS AWAKE event */
		if (priv->psstate == PS_STATE_FULL_POWER) {
			lbs_deb_cmd(
			       "EVENT: In FULL POWER mode - ignore PS AWAKE\n");
			break;
		}

		priv->psstate = PS_STATE_AWAKE;

		if (priv->needtowakeup) {
			/*
			 * wait for the command processing to finish
			 * before resuming sending
			 * priv->needtowakeup will be set to FALSE
			 * in lbs_ps_wakeup()
			 */
			lbs_deb_cmd("waking up ...\n");
			lbs_set_ps_mode(priv, PS_MODE_ACTION_EXIT_PS, false);
		}
		break;

	case MACREG_INT_CODE_MIC_ERR_UNICAST:
		lbs_deb_cmd("EVENT: UNICAST MIC ERROR\n");
		lbs_send_mic_failureevent(priv, event);
		break;

	case MACREG_INT_CODE_MIC_ERR_MULTICAST:
		lbs_deb_cmd("EVENT: MULTICAST MIC ERROR\n");
		lbs_send_mic_failureevent(priv, event);
		break;

	case MACREG_INT_CODE_MIB_CHANGED:
		lbs_deb_cmd("EVENT: MIB CHANGED\n");
		break;
	case MACREG_INT_CODE_INIT_DONE:
		lbs_deb_cmd("EVENT: INIT DONE\n");
		break;
	case MACREG_INT_CODE_ADHOC_BCN_LOST:
		lbs_deb_cmd("EVENT: ADHOC beacon lost\n");
		break;
	case MACREG_INT_CODE_RSSI_LOW:
		netdev_alert(priv->dev, "EVENT: rssi low\n");
		break;
	case MACREG_INT_CODE_SNR_LOW:
		netdev_alert(priv->dev, "EVENT: snr low\n");
		break;
	case MACREG_INT_CODE_MAX_FAIL:
		netdev_alert(priv->dev, "EVENT: max fail\n");
		break;
	case MACREG_INT_CODE_RSSI_HIGH:
		netdev_alert(priv->dev, "EVENT: rssi high\n");
		break;
	case MACREG_INT_CODE_SNR_HIGH:
		netdev_alert(priv->dev, "EVENT: snr high\n");
		break;

	case MACREG_INT_CODE_MESH_AUTO_STARTED:
		/* Ignore spurious autostart events */
		netdev_info(priv->dev, "EVENT: MESH_AUTO_STARTED (ignoring)\n");
		break;

	default:
		netdev_alert(priv->dev, "EVENT: unknown event id %d\n", event);
		break;
	}
}
