// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <net/mac80211.h>
#include <linux/etherdevice.h>

#include "mac.h"
#include "core.h"
#include "debug.h"
#include "wmi.h"
#include "hw.h"
#include "dp_tx.h"
#include "dp_rx.h"
#include "peer.h"
#include "debugfs.h"
#include "hif.h"
#include "wow.h"

#define CHAN2G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_2GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

#define CHAN5G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_5GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

#define CHAN6G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_6GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

static const struct ieee80211_channel ath12k_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static const struct ieee80211_channel ath12k_5ghz_channels[] = {
	CHAN5G(36, 5180, 0),
	CHAN5G(40, 5200, 0),
	CHAN5G(44, 5220, 0),
	CHAN5G(48, 5240, 0),
	CHAN5G(52, 5260, 0),
	CHAN5G(56, 5280, 0),
	CHAN5G(60, 5300, 0),
	CHAN5G(64, 5320, 0),
	CHAN5G(100, 5500, 0),
	CHAN5G(104, 5520, 0),
	CHAN5G(108, 5540, 0),
	CHAN5G(112, 5560, 0),
	CHAN5G(116, 5580, 0),
	CHAN5G(120, 5600, 0),
	CHAN5G(124, 5620, 0),
	CHAN5G(128, 5640, 0),
	CHAN5G(132, 5660, 0),
	CHAN5G(136, 5680, 0),
	CHAN5G(140, 5700, 0),
	CHAN5G(144, 5720, 0),
	CHAN5G(149, 5745, 0),
	CHAN5G(153, 5765, 0),
	CHAN5G(157, 5785, 0),
	CHAN5G(161, 5805, 0),
	CHAN5G(165, 5825, 0),
	CHAN5G(169, 5845, 0),
	CHAN5G(173, 5865, 0),
};

static const struct ieee80211_channel ath12k_6ghz_channels[] = {
	/* Operating Class 136 */
	CHAN6G(2, 5935, 0),

	/* Operating Classes 131-135 */
	CHAN6G(1, 5955, 0),
	CHAN6G(5, 5975, 0),
	CHAN6G(9, 5995, 0),
	CHAN6G(13, 6015, 0),
	CHAN6G(17, 6035, 0),
	CHAN6G(21, 6055, 0),
	CHAN6G(25, 6075, 0),
	CHAN6G(29, 6095, 0),
	CHAN6G(33, 6115, 0),
	CHAN6G(37, 6135, 0),
	CHAN6G(41, 6155, 0),
	CHAN6G(45, 6175, 0),
	CHAN6G(49, 6195, 0),
	CHAN6G(53, 6215, 0),
	CHAN6G(57, 6235, 0),
	CHAN6G(61, 6255, 0),
	CHAN6G(65, 6275, 0),
	CHAN6G(69, 6295, 0),
	CHAN6G(73, 6315, 0),
	CHAN6G(77, 6335, 0),
	CHAN6G(81, 6355, 0),
	CHAN6G(85, 6375, 0),
	CHAN6G(89, 6395, 0),
	CHAN6G(93, 6415, 0),
	CHAN6G(97, 6435, 0),
	CHAN6G(101, 6455, 0),
	CHAN6G(105, 6475, 0),
	CHAN6G(109, 6495, 0),
	CHAN6G(113, 6515, 0),
	CHAN6G(117, 6535, 0),
	CHAN6G(121, 6555, 0),
	CHAN6G(125, 6575, 0),
	CHAN6G(129, 6595, 0),
	CHAN6G(133, 6615, 0),
	CHAN6G(137, 6635, 0),
	CHAN6G(141, 6655, 0),
	CHAN6G(145, 6675, 0),
	CHAN6G(149, 6695, 0),
	CHAN6G(153, 6715, 0),
	CHAN6G(157, 6735, 0),
	CHAN6G(161, 6755, 0),
	CHAN6G(165, 6775, 0),
	CHAN6G(169, 6795, 0),
	CHAN6G(173, 6815, 0),
	CHAN6G(177, 6835, 0),
	CHAN6G(181, 6855, 0),
	CHAN6G(185, 6875, 0),
	CHAN6G(189, 6895, 0),
	CHAN6G(193, 6915, 0),
	CHAN6G(197, 6935, 0),
	CHAN6G(201, 6955, 0),
	CHAN6G(205, 6975, 0),
	CHAN6G(209, 6995, 0),
	CHAN6G(213, 7015, 0),
	CHAN6G(217, 7035, 0),
	CHAN6G(221, 7055, 0),
	CHAN6G(225, 7075, 0),
	CHAN6G(229, 7095, 0),
	CHAN6G(233, 7115, 0),
};

static struct ieee80211_rate ath12k_legacy_rates[] = {
	{ .bitrate = 10,
	  .hw_value = ATH12K_HW_RATE_CCK_LP_1M },
	{ .bitrate = 20,
	  .hw_value = ATH12K_HW_RATE_CCK_LP_2M,
	  .hw_value_short = ATH12K_HW_RATE_CCK_SP_2M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 55,
	  .hw_value = ATH12K_HW_RATE_CCK_LP_5_5M,
	  .hw_value_short = ATH12K_HW_RATE_CCK_SP_5_5M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 110,
	  .hw_value = ATH12K_HW_RATE_CCK_LP_11M,
	  .hw_value_short = ATH12K_HW_RATE_CCK_SP_11M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },

	{ .bitrate = 60, .hw_value = ATH12K_HW_RATE_OFDM_6M },
	{ .bitrate = 90, .hw_value = ATH12K_HW_RATE_OFDM_9M },
	{ .bitrate = 120, .hw_value = ATH12K_HW_RATE_OFDM_12M },
	{ .bitrate = 180, .hw_value = ATH12K_HW_RATE_OFDM_18M },
	{ .bitrate = 240, .hw_value = ATH12K_HW_RATE_OFDM_24M },
	{ .bitrate = 360, .hw_value = ATH12K_HW_RATE_OFDM_36M },
	{ .bitrate = 480, .hw_value = ATH12K_HW_RATE_OFDM_48M },
	{ .bitrate = 540, .hw_value = ATH12K_HW_RATE_OFDM_54M },
};

static const int
ath12k_phymodes[NUM_NL80211_BANDS][ATH12K_CHAN_WIDTH_NUM] = {
	[NL80211_BAND_2GHZ] = {
			[NL80211_CHAN_WIDTH_5] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_10] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_20_NOHT] = MODE_11BE_EHT20_2G,
			[NL80211_CHAN_WIDTH_20] = MODE_11BE_EHT20_2G,
			[NL80211_CHAN_WIDTH_40] = MODE_11BE_EHT40_2G,
			[NL80211_CHAN_WIDTH_80] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_80P80] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_160] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_320] = MODE_UNKNOWN,
	},
	[NL80211_BAND_5GHZ] = {
			[NL80211_CHAN_WIDTH_5] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_10] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_20_NOHT] = MODE_11BE_EHT20,
			[NL80211_CHAN_WIDTH_20] = MODE_11BE_EHT20,
			[NL80211_CHAN_WIDTH_40] = MODE_11BE_EHT40,
			[NL80211_CHAN_WIDTH_80] = MODE_11BE_EHT80,
			[NL80211_CHAN_WIDTH_160] = MODE_11BE_EHT160,
			[NL80211_CHAN_WIDTH_80P80] = MODE_11BE_EHT80_80,
			[NL80211_CHAN_WIDTH_320] = MODE_11BE_EHT320,
	},
	[NL80211_BAND_6GHZ] = {
			[NL80211_CHAN_WIDTH_5] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_10] = MODE_UNKNOWN,
			[NL80211_CHAN_WIDTH_20_NOHT] = MODE_11BE_EHT20,
			[NL80211_CHAN_WIDTH_20] = MODE_11BE_EHT20,
			[NL80211_CHAN_WIDTH_40] = MODE_11BE_EHT40,
			[NL80211_CHAN_WIDTH_80] = MODE_11BE_EHT80,
			[NL80211_CHAN_WIDTH_160] = MODE_11BE_EHT160,
			[NL80211_CHAN_WIDTH_80P80] = MODE_11BE_EHT80_80,
			[NL80211_CHAN_WIDTH_320] = MODE_11BE_EHT320,
	},

};

const struct htt_rx_ring_tlv_filter ath12k_mac_mon_status_filter_default = {
	.rx_filter = HTT_RX_FILTER_TLV_FLAGS_MPDU_START |
		     HTT_RX_FILTER_TLV_FLAGS_PPDU_END |
		     HTT_RX_FILTER_TLV_FLAGS_PPDU_END_STATUS_DONE,
	.pkt_filter_flags0 = HTT_RX_FP_MGMT_FILTER_FLAGS0,
	.pkt_filter_flags1 = HTT_RX_FP_MGMT_FILTER_FLAGS1,
	.pkt_filter_flags2 = HTT_RX_FP_CTRL_FILTER_FLASG2,
	.pkt_filter_flags3 = HTT_RX_FP_DATA_FILTER_FLASG3 |
			     HTT_RX_FP_CTRL_FILTER_FLASG3
};

#define ATH12K_MAC_FIRST_OFDM_RATE_IDX 4
#define ath12k_g_rates ath12k_legacy_rates
#define ath12k_g_rates_size (ARRAY_SIZE(ath12k_legacy_rates))
#define ath12k_a_rates (ath12k_legacy_rates + 4)
#define ath12k_a_rates_size (ARRAY_SIZE(ath12k_legacy_rates) - 4)

#define ATH12K_MAC_SCAN_TIMEOUT_MSECS 200 /* in msecs */

static const u32 ath12k_smps_map[] = {
	[WLAN_HT_CAP_SM_PS_STATIC] = WMI_PEER_SMPS_STATIC,
	[WLAN_HT_CAP_SM_PS_DYNAMIC] = WMI_PEER_SMPS_DYNAMIC,
	[WLAN_HT_CAP_SM_PS_INVALID] = WMI_PEER_SMPS_PS_NONE,
	[WLAN_HT_CAP_SM_PS_DISABLED] = WMI_PEER_SMPS_PS_NONE,
};

static int ath12k_start_vdev_delay(struct ath12k *ar,
				   struct ath12k_vif *arvif);
static void ath12k_mac_stop(struct ath12k *ar);
static int ath12k_mac_vdev_create(struct ath12k *ar, struct ieee80211_vif *vif);
static int ath12k_mac_vdev_delete(struct ath12k *ar, struct ieee80211_vif *vif);

static const char *ath12k_mac_phymode_str(enum wmi_phy_mode mode)
{
	switch (mode) {
	case MODE_11A:
		return "11a";
	case MODE_11G:
		return "11g";
	case MODE_11B:
		return "11b";
	case MODE_11GONLY:
		return "11gonly";
	case MODE_11NA_HT20:
		return "11na-ht20";
	case MODE_11NG_HT20:
		return "11ng-ht20";
	case MODE_11NA_HT40:
		return "11na-ht40";
	case MODE_11NG_HT40:
		return "11ng-ht40";
	case MODE_11AC_VHT20:
		return "11ac-vht20";
	case MODE_11AC_VHT40:
		return "11ac-vht40";
	case MODE_11AC_VHT80:
		return "11ac-vht80";
	case MODE_11AC_VHT160:
		return "11ac-vht160";
	case MODE_11AC_VHT80_80:
		return "11ac-vht80+80";
	case MODE_11AC_VHT20_2G:
		return "11ac-vht20-2g";
	case MODE_11AC_VHT40_2G:
		return "11ac-vht40-2g";
	case MODE_11AC_VHT80_2G:
		return "11ac-vht80-2g";
	case MODE_11AX_HE20:
		return "11ax-he20";
	case MODE_11AX_HE40:
		return "11ax-he40";
	case MODE_11AX_HE80:
		return "11ax-he80";
	case MODE_11AX_HE80_80:
		return "11ax-he80+80";
	case MODE_11AX_HE160:
		return "11ax-he160";
	case MODE_11AX_HE20_2G:
		return "11ax-he20-2g";
	case MODE_11AX_HE40_2G:
		return "11ax-he40-2g";
	case MODE_11AX_HE80_2G:
		return "11ax-he80-2g";
	case MODE_11BE_EHT20:
		return "11be-eht20";
	case MODE_11BE_EHT40:
		return "11be-eht40";
	case MODE_11BE_EHT80:
		return "11be-eht80";
	case MODE_11BE_EHT80_80:
		return "11be-eht80+80";
	case MODE_11BE_EHT160:
		return "11be-eht160";
	case MODE_11BE_EHT160_160:
		return "11be-eht160+160";
	case MODE_11BE_EHT320:
		return "11be-eht320";
	case MODE_11BE_EHT20_2G:
		return "11be-eht20-2g";
	case MODE_11BE_EHT40_2G:
		return "11be-eht40-2g";
	case MODE_UNKNOWN:
		/* skip */
		break;

		/* no default handler to allow compiler to check that the
		 * enum is fully handled
		 */
	}

	return "<unknown>";
}

enum rate_info_bw
ath12k_mac_bw_to_mac80211_bw(enum ath12k_supported_bw bw)
{
	u8 ret = RATE_INFO_BW_20;

	switch (bw) {
	case ATH12K_BW_20:
		ret = RATE_INFO_BW_20;
		break;
	case ATH12K_BW_40:
		ret = RATE_INFO_BW_40;
		break;
	case ATH12K_BW_80:
		ret = RATE_INFO_BW_80;
		break;
	case ATH12K_BW_160:
		ret = RATE_INFO_BW_160;
		break;
	case ATH12K_BW_320:
		ret = RATE_INFO_BW_320;
		break;
	}

	return ret;
}

enum ath12k_supported_bw ath12k_mac_mac80211_bw_to_ath12k_bw(enum rate_info_bw bw)
{
	switch (bw) {
	case RATE_INFO_BW_20:
		return ATH12K_BW_20;
	case RATE_INFO_BW_40:
		return ATH12K_BW_40;
	case RATE_INFO_BW_80:
		return ATH12K_BW_80;
	case RATE_INFO_BW_160:
		return ATH12K_BW_160;
	case RATE_INFO_BW_320:
		return ATH12K_BW_320;
	default:
		return ATH12K_BW_20;
	}
}

int ath12k_mac_hw_ratecode_to_legacy_rate(u8 hw_rc, u8 preamble, u8 *rateidx,
					  u16 *rate)
{
	/* As default, it is OFDM rates */
	int i = ATH12K_MAC_FIRST_OFDM_RATE_IDX;
	int max_rates_idx = ath12k_g_rates_size;

	if (preamble == WMI_RATE_PREAMBLE_CCK) {
		hw_rc &= ~ATH12K_HW_RATECODE_CCK_SHORT_PREAM_MASK;
		i = 0;
		max_rates_idx = ATH12K_MAC_FIRST_OFDM_RATE_IDX;
	}

	while (i < max_rates_idx) {
		if (hw_rc == ath12k_legacy_rates[i].hw_value) {
			*rateidx = i;
			*rate = ath12k_legacy_rates[i].bitrate;
			return 0;
		}
		i++;
	}

	return -EINVAL;
}

u8 ath12k_mac_bitrate_to_idx(const struct ieee80211_supported_band *sband,
			     u32 bitrate)
{
	int i;

	for (i = 0; i < sband->n_bitrates; i++)
		if (sband->bitrates[i].bitrate == bitrate)
			return i;

	return 0;
}

static u32
ath12k_mac_max_ht_nss(const u8 *ht_mcs_mask)
{
	int nss;

	for (nss = IEEE80211_HT_MCS_MASK_LEN - 1; nss >= 0; nss--)
		if (ht_mcs_mask[nss])
			return nss + 1;

	return 1;
}

static u32
ath12k_mac_max_vht_nss(const u16 *vht_mcs_mask)
{
	int nss;

	for (nss = NL80211_VHT_NSS_MAX - 1; nss >= 0; nss--)
		if (vht_mcs_mask[nss])
			return nss + 1;

	return 1;
}

static u8 ath12k_parse_mpdudensity(u8 mpdudensity)
{
/*  From IEEE Std 802.11-2020 defined values for "Minimum MPDU Start Spacing":
 *   0 for no restriction
 *   1 for 1/4 us
 *   2 for 1/2 us
 *   3 for 1 us
 *   4 for 2 us
 *   5 for 4 us
 *   6 for 8 us
 *   7 for 16 us
 */
	switch (mpdudensity) {
	case 0:
		return 0;
	case 1:
	case 2:
	case 3:
	/* Our lower layer calculations limit our precision to
	 * 1 microsecond
	 */
		return 1;
	case 4:
		return 2;
	case 5:
		return 4;
	case 6:
		return 8;
	case 7:
		return 16;
	default:
		return 0;
	}
}

static int ath12k_mac_vif_chan(struct ieee80211_vif *vif,
			       struct cfg80211_chan_def *def)
{
	struct ieee80211_chanctx_conf *conf;

	rcu_read_lock();
	conf = rcu_dereference(vif->bss_conf.chanctx_conf);
	if (!conf) {
		rcu_read_unlock();
		return -ENOENT;
	}

	*def = conf->def;
	rcu_read_unlock();

	return 0;
}

static bool ath12k_mac_bitrate_is_cck(int bitrate)
{
	switch (bitrate) {
	case 10:
	case 20:
	case 55:
	case 110:
		return true;
	}

	return false;
}

u8 ath12k_mac_hw_rate_to_idx(const struct ieee80211_supported_band *sband,
			     u8 hw_rate, bool cck)
{
	const struct ieee80211_rate *rate;
	int i;

	for (i = 0; i < sband->n_bitrates; i++) {
		rate = &sband->bitrates[i];

		if (ath12k_mac_bitrate_is_cck(rate->bitrate) != cck)
			continue;

		if (rate->hw_value == hw_rate)
			return i;
		else if (rate->flags & IEEE80211_RATE_SHORT_PREAMBLE &&
			 rate->hw_value_short == hw_rate)
			return i;
	}

	return 0;
}

static u8 ath12k_mac_bitrate_to_rate(int bitrate)
{
	return DIV_ROUND_UP(bitrate, 5) |
	       (ath12k_mac_bitrate_is_cck(bitrate) ? BIT(7) : 0);
}

static void ath12k_get_arvif_iter(void *data, u8 *mac,
				  struct ieee80211_vif *vif)
{
	struct ath12k_vif_iter *arvif_iter = data;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);

	if (arvif->vdev_id == arvif_iter->vdev_id &&
	    arvif->ar == arvif_iter->ar)
		arvif_iter->arvif = arvif;
}

struct ath12k_vif *ath12k_mac_get_arvif(struct ath12k *ar, u32 vdev_id)
{
	struct ath12k_vif_iter arvif_iter = {};
	u32 flags;

	arvif_iter.vdev_id = vdev_id;
	arvif_iter.ar = ar;

	flags = IEEE80211_IFACE_ITER_RESUME_ALL;
	ieee80211_iterate_active_interfaces_atomic(ath12k_ar_to_hw(ar),
						   flags,
						   ath12k_get_arvif_iter,
						   &arvif_iter);
	if (!arvif_iter.arvif) {
		ath12k_warn(ar->ab, "No VIF found for vdev %d\n", vdev_id);
		return NULL;
	}

	return arvif_iter.arvif;
}

struct ath12k_vif *ath12k_mac_get_arvif_by_vdev_id(struct ath12k_base *ab,
						   u32 vdev_id)
{
	int i;
	struct ath12k_pdev *pdev;
	struct ath12k_vif *arvif;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = rcu_dereference(ab->pdevs_active[i]);
		if (pdev && pdev->ar &&
		    (pdev->ar->allocated_vdev_map & (1LL << vdev_id))) {
			arvif = ath12k_mac_get_arvif(pdev->ar, vdev_id);
			if (arvif)
				return arvif;
		}
	}

	return NULL;
}

struct ath12k *ath12k_mac_get_ar_by_vdev_id(struct ath12k_base *ab, u32 vdev_id)
{
	int i;
	struct ath12k_pdev *pdev;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = rcu_dereference(ab->pdevs_active[i]);
		if (pdev && pdev->ar) {
			if (pdev->ar->allocated_vdev_map & (1LL << vdev_id))
				return pdev->ar;
		}
	}

	return NULL;
}

struct ath12k *ath12k_mac_get_ar_by_pdev_id(struct ath12k_base *ab, u32 pdev_id)
{
	int i;
	struct ath12k_pdev *pdev;

	if (ab->hw_params->single_pdev_only) {
		pdev = rcu_dereference(ab->pdevs_active[0]);
		return pdev ? pdev->ar : NULL;
	}

	if (WARN_ON(pdev_id > ab->num_radios))
		return NULL;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = rcu_dereference(ab->pdevs_active[i]);

		if (pdev && pdev->pdev_id == pdev_id)
			return (pdev->ar ? pdev->ar : NULL);
	}

	return NULL;
}

static struct ath12k *ath12k_mac_get_ar_by_chan(struct ieee80211_hw *hw,
						struct ieee80211_channel *channel)
{
	struct ath12k_hw *ah = hw->priv;
	struct ath12k *ar;
	int i;

	ar = ah->radio;

	if (ah->num_radio == 1)
		return ar;

	for_each_ar(ah, ar, i) {
		if (channel->center_freq >= ar->freq_low &&
		    channel->center_freq <= ar->freq_high)
			return ar;
	}
	return NULL;
}

static struct ath12k *ath12k_get_ar_by_ctx(struct ieee80211_hw *hw,
					   struct ieee80211_chanctx_conf *ctx)
{
	if (!ctx)
		return NULL;

	return ath12k_mac_get_ar_by_chan(hw, ctx->def.chan);
}

static struct ath12k *ath12k_get_ar_by_vif(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);

	/* If there is one pdev within ah, then we return
	 * ar directly.
	 */
	if (ah->num_radio == 1)
		return ah->radio;

	if (arvif->is_created)
		return arvif->ar;

	return NULL;
}

static struct ath12k_vif *ath12k_mac_get_vif_up(struct ath12k *ar)
{
	struct ath12k_vif *arvif;

	lockdep_assert_held(&ar->conf_mutex);
	list_for_each_entry(arvif, &ar->arvifs, list) {
		if (arvif->is_up)
			return arvif;
	}

	return NULL;
}

static bool ath12k_mac_band_match(enum nl80211_band band1, enum WMI_HOST_WLAN_BAND band2)
{
	switch (band1) {
	case NL80211_BAND_2GHZ:
		if (band2 & WMI_HOST_WLAN_2G_CAP)
			return true;
		break;
	case NL80211_BAND_5GHZ:
	case NL80211_BAND_6GHZ:
		if (band2 & WMI_HOST_WLAN_5G_CAP)
			return true;
		break;
	default:
		return false;
	}

	return false;
}

static u8 ath12k_mac_get_target_pdev_id_from_vif(struct ath12k_vif *arvif)
{
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_vif *vif = arvif->vif;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	u8 pdev_id = ab->fw_pdev[0].pdev_id;
	int i;

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return pdev_id;

	band = def.chan->band;

	for (i = 0; i < ab->fw_pdev_count; i++) {
		if (ath12k_mac_band_match(band, ab->fw_pdev[i].supported_bands))
			return ab->fw_pdev[i].pdev_id;
	}

	return pdev_id;
}

u8 ath12k_mac_get_target_pdev_id(struct ath12k *ar)
{
	struct ath12k_vif *arvif;
	struct ath12k_base *ab = ar->ab;

	if (!ab->hw_params->single_pdev_only)
		return ar->pdev->pdev_id;

	arvif = ath12k_mac_get_vif_up(ar);

	/* fw_pdev array has pdev ids derived from phy capability
	 * service ready event (pdev_and_hw_link_ids).
	 * If no vif is active, return default first index.
	 */
	if (!arvif)
		return ar->ab->fw_pdev[0].pdev_id;

	/* If active vif is found, return the pdev id matching chandef band */
	return ath12k_mac_get_target_pdev_id_from_vif(arvif);
}

static void ath12k_pdev_caps_update(struct ath12k *ar)
{
	struct ath12k_base *ab = ar->ab;

	ar->max_tx_power = ab->target_caps.hw_max_tx_power;

	/* FIXME: Set min_tx_power to ab->target_caps.hw_min_tx_power.
	 * But since the received value in svcrdy is same as hw_max_tx_power,
	 * we can set ar->min_tx_power to 0 currently until
	 * this is fixed in firmware
	 */
	ar->min_tx_power = 0;

	ar->txpower_limit_2g = ar->max_tx_power;
	ar->txpower_limit_5g = ar->max_tx_power;
	ar->txpower_scale = WMI_HOST_TP_SCALE_MAX;
}

static int ath12k_mac_txpower_recalc(struct ath12k *ar)
{
	struct ath12k_pdev *pdev = ar->pdev;
	struct ath12k_vif *arvif;
	int ret, txpower = -1;
	u32 param;

	lockdep_assert_held(&ar->conf_mutex);

	list_for_each_entry(arvif, &ar->arvifs, list) {
		if (arvif->txpower <= 0)
			continue;

		if (txpower == -1)
			txpower = arvif->txpower;
		else
			txpower = min(txpower, arvif->txpower);
	}

	if (txpower == -1)
		return 0;

	/* txpwr is set as 2 units per dBm in FW*/
	txpower = min_t(u32, max_t(u32, ar->min_tx_power, txpower),
			ar->max_tx_power) * 2;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "txpower to set in hw %d\n",
		   txpower / 2);

	if ((pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP) &&
	    ar->txpower_limit_2g != txpower) {
		param = WMI_PDEV_PARAM_TXPOWER_LIMIT2G;
		ret = ath12k_wmi_pdev_set_param(ar, param,
						txpower, ar->pdev->pdev_id);
		if (ret)
			goto fail;
		ar->txpower_limit_2g = txpower;
	}

	if ((pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP) &&
	    ar->txpower_limit_5g != txpower) {
		param = WMI_PDEV_PARAM_TXPOWER_LIMIT5G;
		ret = ath12k_wmi_pdev_set_param(ar, param,
						txpower, ar->pdev->pdev_id);
		if (ret)
			goto fail;
		ar->txpower_limit_5g = txpower;
	}

	return 0;

fail:
	ath12k_warn(ar->ab, "failed to recalc txpower limit %d using pdev param %d: %d\n",
		    txpower / 2, param, ret);
	return ret;
}

static int ath12k_recalc_rtscts_prot(struct ath12k_vif *arvif)
{
	struct ath12k *ar = arvif->ar;
	u32 vdev_param, rts_cts;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	vdev_param = WMI_VDEV_PARAM_ENABLE_RTSCTS;

	/* Enable RTS/CTS protection for sw retries (when legacy stations
	 * are in BSS) or by default only for second rate series.
	 * TODO: Check if we need to enable CTS 2 Self in any case
	 */
	rts_cts = WMI_USE_RTS_CTS;

	if (arvif->num_legacy_stations > 0)
		rts_cts |= WMI_RTSCTS_ACROSS_SW_RETRIES << 4;
	else
		rts_cts |= WMI_RTSCTS_FOR_SECOND_RATESERIES << 4;

	/* Need not send duplicate param value to firmware */
	if (arvif->rtscts_prot_mode == rts_cts)
		return 0;

	arvif->rtscts_prot_mode = rts_cts;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev %d recalc rts/cts prot %d\n",
		   arvif->vdev_id, rts_cts);

	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, rts_cts);
	if (ret)
		ath12k_warn(ar->ab, "failed to recalculate rts/cts prot for vdev %d: %d\n",
			    arvif->vdev_id, ret);

	return ret;
}

static int ath12k_mac_set_kickout(struct ath12k_vif *arvif)
{
	struct ath12k *ar = arvif->ar;
	u32 param;
	int ret;

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_STA_KICKOUT_TH,
					ATH12K_KICKOUT_THRESHOLD,
					ar->pdev->pdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set kickout threshold on vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	param = WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id, param,
					    ATH12K_KEEPALIVE_MIN_IDLE);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set keepalive minimum idle time on vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	param = WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id, param,
					    ATH12K_KEEPALIVE_MAX_IDLE);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set keepalive maximum idle time on vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	param = WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id, param,
					    ATH12K_KEEPALIVE_MAX_UNRESPONSIVE);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set keepalive maximum unresponsive time on vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	return 0;
}

void ath12k_mac_peer_cleanup_all(struct ath12k *ar)
{
	struct ath12k_peer *peer, *tmp;
	struct ath12k_base *ab = ar->ab;

	lockdep_assert_held(&ar->conf_mutex);

	spin_lock_bh(&ab->base_lock);
	list_for_each_entry_safe(peer, tmp, &ab->peers, list) {
		/* Skip Rx TID cleanup for self peer */
		if (peer->sta)
			ath12k_dp_rx_peer_tid_cleanup(ar, peer);

		list_del(&peer->list);
		kfree(peer);
	}
	spin_unlock_bh(&ab->base_lock);

	ar->num_peers = 0;
	ar->num_stations = 0;
}

static int ath12k_mac_vdev_setup_sync(struct ath12k *ar)
{
	lockdep_assert_held(&ar->conf_mutex);

	if (test_bit(ATH12K_FLAG_CRASH_FLUSH, &ar->ab->dev_flags))
		return -ESHUTDOWN;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "vdev setup timeout %d\n",
		   ATH12K_VDEV_SETUP_TIMEOUT_HZ);

	if (!wait_for_completion_timeout(&ar->vdev_setup_done,
					 ATH12K_VDEV_SETUP_TIMEOUT_HZ))
		return -ETIMEDOUT;

	return ar->last_wmi_vdev_start_status ? -EINVAL : 0;
}

static int ath12k_monitor_vdev_up(struct ath12k *ar, int vdev_id)
{
	struct ath12k_wmi_vdev_up_params params = {};
	int ret;

	params.vdev_id = vdev_id;
	params.bssid = ar->mac_addr;
	ret = ath12k_wmi_vdev_up(ar, &params);
	if (ret) {
		ath12k_warn(ar->ab, "failed to put up monitor vdev %i: %d\n",
			    vdev_id, ret);
		return ret;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor vdev %i started\n",
		   vdev_id);
	return 0;
}

static int ath12k_mac_monitor_vdev_start(struct ath12k *ar, int vdev_id,
					 struct cfg80211_chan_def *chandef)
{
	struct ieee80211_channel *channel;
	struct wmi_vdev_start_req_arg arg = {};
	struct ath12k_wmi_vdev_up_params params = {};
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	channel = chandef->chan;
	arg.vdev_id = vdev_id;
	arg.freq = channel->center_freq;
	arg.band_center_freq1 = chandef->center_freq1;
	arg.band_center_freq2 = chandef->center_freq2;
	arg.mode = ath12k_phymodes[chandef->chan->band][chandef->width];
	arg.chan_radar = !!(channel->flags & IEEE80211_CHAN_RADAR);

	arg.min_power = 0;
	arg.max_power = channel->max_power;
	arg.max_reg_power = channel->max_reg_power;
	arg.max_antenna_gain = channel->max_antenna_gain;

	arg.pref_tx_streams = ar->num_tx_chains;
	arg.pref_rx_streams = ar->num_rx_chains;
	arg.punct_bitmap = 0xFFFFFFFF;

	arg.passive |= !!(chandef->chan->flags & IEEE80211_CHAN_NO_IR);

	reinit_completion(&ar->vdev_setup_done);
	reinit_completion(&ar->vdev_delete_done);

	ret = ath12k_wmi_vdev_start(ar, &arg, false);
	if (ret) {
		ath12k_warn(ar->ab, "failed to request monitor vdev %i start: %d\n",
			    vdev_id, ret);
		return ret;
	}

	ret = ath12k_mac_vdev_setup_sync(ar);
	if (ret) {
		ath12k_warn(ar->ab, "failed to synchronize setup for monitor vdev %i start: %d\n",
			    vdev_id, ret);
		return ret;
	}

	params.vdev_id = vdev_id;
	params.bssid = ar->mac_addr;
	ret = ath12k_wmi_vdev_up(ar, &params);
	if (ret) {
		ath12k_warn(ar->ab, "failed to put up monitor vdev %i: %d\n",
			    vdev_id, ret);
		goto vdev_stop;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor vdev %i started\n",
		   vdev_id);
	return 0;

vdev_stop:
	ret = ath12k_wmi_vdev_stop(ar, vdev_id);
	if (ret)
		ath12k_warn(ar->ab, "failed to stop monitor vdev %i after start failure: %d\n",
			    vdev_id, ret);
	return ret;
}

static int ath12k_mac_monitor_vdev_stop(struct ath12k *ar)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	reinit_completion(&ar->vdev_setup_done);

	ret = ath12k_wmi_vdev_stop(ar, ar->monitor_vdev_id);
	if (ret)
		ath12k_warn(ar->ab, "failed to request monitor vdev %i stop: %d\n",
			    ar->monitor_vdev_id, ret);

	ret = ath12k_mac_vdev_setup_sync(ar);
	if (ret)
		ath12k_warn(ar->ab, "failed to synchronize monitor vdev %i stop: %d\n",
			    ar->monitor_vdev_id, ret);

	ret = ath12k_wmi_vdev_down(ar, ar->monitor_vdev_id);
	if (ret)
		ath12k_warn(ar->ab, "failed to put down monitor vdev %i: %d\n",
			    ar->monitor_vdev_id, ret);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor vdev %i stopped\n",
		   ar->monitor_vdev_id);
	return ret;
}

static int ath12k_mac_monitor_vdev_create(struct ath12k *ar)
{
	struct ath12k_pdev *pdev = ar->pdev;
	struct ath12k_wmi_vdev_create_arg arg = {};
	int bit, ret;
	u8 tmp_addr[6];
	u16 nss;

	lockdep_assert_held(&ar->conf_mutex);

	if (ar->monitor_vdev_created)
		return 0;

	if (ar->ab->free_vdev_map == 0) {
		ath12k_warn(ar->ab, "failed to find free vdev id for monitor vdev\n");
		return -ENOMEM;
	}

	bit = __ffs64(ar->ab->free_vdev_map);

	ar->monitor_vdev_id = bit;

	arg.if_id = ar->monitor_vdev_id;
	arg.type = WMI_VDEV_TYPE_MONITOR;
	arg.subtype = WMI_VDEV_SUBTYPE_NONE;
	arg.pdev_id = pdev->pdev_id;
	arg.if_stats_id = ATH12K_INVAL_VDEV_STATS_ID;

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP) {
		arg.chains[NL80211_BAND_2GHZ].tx = ar->num_tx_chains;
		arg.chains[NL80211_BAND_2GHZ].rx = ar->num_rx_chains;
	}

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP) {
		arg.chains[NL80211_BAND_5GHZ].tx = ar->num_tx_chains;
		arg.chains[NL80211_BAND_5GHZ].rx = ar->num_rx_chains;
	}

	ret = ath12k_wmi_vdev_create(ar, tmp_addr, &arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to request monitor vdev %i creation: %d\n",
			    ar->monitor_vdev_id, ret);
		ar->monitor_vdev_id = -1;
		return ret;
	}

	nss = hweight32(ar->cfg_tx_chainmask) ? : 1;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, ar->monitor_vdev_id,
					    WMI_VDEV_PARAM_NSS, nss);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set vdev %d chainmask 0x%x, nss %d :%d\n",
			    ar->monitor_vdev_id, ar->cfg_tx_chainmask, nss, ret);
		return ret;
	}

	ret = ath12k_mac_txpower_recalc(ar);
	if (ret)
		return ret;

	ar->allocated_vdev_map |= 1LL << ar->monitor_vdev_id;
	ar->ab->free_vdev_map &= ~(1LL << ar->monitor_vdev_id);
	ar->num_created_vdevs++;
	ar->monitor_vdev_created = true;
	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor vdev %d created\n",
		   ar->monitor_vdev_id);

	return 0;
}

static int ath12k_mac_monitor_vdev_delete(struct ath12k *ar)
{
	int ret;
	unsigned long time_left;

	lockdep_assert_held(&ar->conf_mutex);

	if (!ar->monitor_vdev_created)
		return 0;

	reinit_completion(&ar->vdev_delete_done);

	ret = ath12k_wmi_vdev_delete(ar, ar->monitor_vdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to request wmi monitor vdev %i removal: %d\n",
			    ar->monitor_vdev_id, ret);
		return ret;
	}

	time_left = wait_for_completion_timeout(&ar->vdev_delete_done,
						ATH12K_VDEV_DELETE_TIMEOUT_HZ);
	if (time_left == 0) {
		ath12k_warn(ar->ab, "Timeout in receiving vdev delete response\n");
	} else {
		ar->allocated_vdev_map &= ~(1LL << ar->monitor_vdev_id);
		ar->ab->free_vdev_map |= 1LL << (ar->monitor_vdev_id);
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor vdev %d deleted\n",
			   ar->monitor_vdev_id);
		ar->num_created_vdevs--;
		ar->monitor_vdev_id = -1;
		ar->monitor_vdev_created = false;
	}

	return ret;
}

static void
ath12k_mac_get_any_chandef_iter(struct ieee80211_hw *hw,
				struct ieee80211_chanctx_conf *conf,
				void *data)
{
	struct cfg80211_chan_def **def = data;

	*def = &conf->def;
}

static int ath12k_mac_monitor_start(struct ath12k *ar)
{
	struct cfg80211_chan_def *chandef = NULL;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (ar->monitor_started)
		return 0;

	ieee80211_iter_chan_contexts_atomic(ath12k_ar_to_hw(ar),
					    ath12k_mac_get_any_chandef_iter,
					    &chandef);
	if (!chandef)
		return 0;

	ret = ath12k_mac_monitor_vdev_start(ar, ar->monitor_vdev_id, chandef);
	if (ret) {
		ath12k_warn(ar->ab, "failed to start monitor vdev: %d\n", ret);
		ath12k_mac_monitor_vdev_delete(ar);
		return ret;
	}

	ar->monitor_started = true;
	ar->num_started_vdevs++;
	ret = ath12k_dp_tx_htt_monitor_mode_ring_config(ar, false);
	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor started ret %d\n", ret);

	return ret;
}

static int ath12k_mac_monitor_stop(struct ath12k *ar)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (!ar->monitor_started)
		return 0;

	ret = ath12k_mac_monitor_vdev_stop(ar);
	if (ret) {
		ath12k_warn(ar->ab, "failed to stop monitor vdev: %d\n", ret);
		return ret;
	}

	ar->monitor_started = false;
	ar->num_started_vdevs--;
	ret = ath12k_dp_tx_htt_monitor_mode_ring_config(ar, true);
	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac monitor stopped ret %d\n", ret);
	return ret;
}

static int ath12k_mac_vdev_stop(struct ath12k_vif *arvif)
{
	struct ath12k *ar = arvif->ar;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	reinit_completion(&ar->vdev_setup_done);

	ret = ath12k_wmi_vdev_stop(ar, arvif->vdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to stop WMI vdev %i: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	ret = ath12k_mac_vdev_setup_sync(ar);
	if (ret) {
		ath12k_warn(ar->ab, "failed to synchronize setup for vdev %i: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	WARN_ON(ar->num_started_vdevs == 0);

	ar->num_started_vdevs--;
	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "vdev %pM stopped, vdev_id %d\n",
		   arvif->vif->addr, arvif->vdev_id);

	if (test_bit(ATH12K_CAC_RUNNING, &ar->dev_flags)) {
		clear_bit(ATH12K_CAC_RUNNING, &ar->dev_flags);
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "CAC Stopped for vdev %d\n",
			   arvif->vdev_id);
	}

	return 0;
err:
	return ret;
}

static int ath12k_mac_config(struct ath12k *ar, u32 changed)
{
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	struct ieee80211_conf *conf = &hw->conf;
	int ret = 0;

	mutex_lock(&ar->conf_mutex);

	if (changed & IEEE80211_CONF_CHANGE_MONITOR) {
		ar->monitor_conf_enabled = conf->flags & IEEE80211_CONF_MONITOR;
		if (ar->monitor_conf_enabled) {
			if (ar->monitor_vdev_created)
				goto exit;
			ret = ath12k_mac_monitor_vdev_create(ar);
			if (ret)
				goto exit;
			ret = ath12k_mac_monitor_start(ar);
			if (ret)
				goto err_mon_del;
		} else {
			if (!ar->monitor_vdev_created)
				goto exit;
			ret = ath12k_mac_monitor_stop(ar);
			if (ret)
				goto exit;
			ath12k_mac_monitor_vdev_delete(ar);
		}
	}

exit:
	mutex_unlock(&ar->conf_mutex);
	return ret;

err_mon_del:
	ath12k_mac_monitor_vdev_delete(ar);
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static int ath12k_mac_op_config(struct ieee80211_hw *hw, u32 changed)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int ret;

	ar = ath12k_ah_to_ar(ah, 0);

	ret = ath12k_mac_config(ar, changed);
	if (ret)
		ath12k_warn(ar->ab, "failed to update config pdev idx %d: %d\n",
			    ar->pdev_idx, ret);

	return ret;
}

static int ath12k_mac_setup_bcn_p2p_ie(struct ath12k_vif *arvif,
				       struct sk_buff *bcn)
{
	struct ath12k *ar = arvif->ar;
	struct ieee80211_mgmt *mgmt;
	const u8 *p2p_ie;
	int ret;

	mgmt = (void *)bcn->data;
	p2p_ie = cfg80211_find_vendor_ie(WLAN_OUI_WFA, WLAN_OUI_TYPE_WFA_P2P,
					 mgmt->u.beacon.variable,
					 bcn->len - (mgmt->u.beacon.variable -
						     bcn->data));
	if (!p2p_ie) {
		ath12k_warn(ar->ab, "no P2P ie found in beacon\n");
		return -ENOENT;
	}

	ret = ath12k_wmi_p2p_go_bcn_ie(ar, arvif->vdev_id, p2p_ie);
	if (ret) {
		ath12k_warn(ar->ab, "failed to submit P2P GO bcn ie for vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	return 0;
}

static int ath12k_mac_remove_vendor_ie(struct sk_buff *skb, unsigned int oui,
				       u8 oui_type, size_t ie_offset)
{
	const u8 *next, *end;
	size_t len;
	u8 *ie;

	if (WARN_ON(skb->len < ie_offset))
		return -EINVAL;

	ie = (u8 *)cfg80211_find_vendor_ie(oui, oui_type,
					   skb->data + ie_offset,
					   skb->len - ie_offset);
	if (!ie)
		return -ENOENT;

	len = ie[1] + 2;
	end = skb->data + skb->len;
	next = ie + len;

	if (WARN_ON(next > end))
		return -EINVAL;

	memmove(ie, next, end - next);
	skb_trim(skb, skb->len - len);

	return 0;
}

static void ath12k_mac_set_arvif_ies(struct ath12k_vif *arvif, struct sk_buff *bcn,
				     u8 bssid_index, bool *nontx_profile_found)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)bcn->data;
	const struct element *elem, *nontx, *index, *nie;
	const u8 *start, *tail;
	u16 rem_len;
	u8 i;

	start = bcn->data + ieee80211_get_hdrlen_from_skb(bcn) + sizeof(mgmt->u.beacon);
	tail = skb_tail_pointer(bcn);
	rem_len = tail - start;

	arvif->rsnie_present = false;
	arvif->wpaie_present = false;

	if (cfg80211_find_ie(WLAN_EID_RSN, start, rem_len))
		arvif->rsnie_present = true;
	if (cfg80211_find_vendor_ie(WLAN_OUI_MICROSOFT, WLAN_OUI_TYPE_MICROSOFT_WPA,
				    start, rem_len))
		arvif->wpaie_present = true;

	/* Return from here for the transmitted profile */
	if (!bssid_index)
		return;

	/* Initial rsnie_present for the nontransmitted profile is set to be same as that
	 * of the transmitted profile. It will be changed if security configurations are
	 * different.
	 */
	*nontx_profile_found = false;
	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, start, rem_len) {
		/* Fixed minimum MBSSID element length with at least one
		 * nontransmitted BSSID profile is 12 bytes as given below;
		 * 1 (max BSSID indicator) +
		 * 2 (Nontransmitted BSSID profile: Subelement ID + length) +
		 * 4 (Nontransmitted BSSID Capabilities: tag + length + info)
		 * 2 (Nontransmitted BSSID SSID: tag + length)
		 * 3 (Nontransmitted BSSID Index: tag + length + BSSID index
		 */
		if (elem->datalen < 12 || elem->data[0] < 1)
			continue; /* Max BSSID indicator must be >=1 */

		for_each_element(nontx, elem->data + 1, elem->datalen - 1) {
			start = nontx->data;

			if (nontx->id != 0 || nontx->datalen < 4)
				continue; /* Invalid nontransmitted profile */

			if (nontx->data[0] != WLAN_EID_NON_TX_BSSID_CAP ||
			    nontx->data[1] != 2) {
				continue; /* Missing nontransmitted BSS capabilities */
			}

			if (nontx->data[4] != WLAN_EID_SSID)
				continue; /* Missing SSID for nontransmitted BSS */

			index = cfg80211_find_elem(WLAN_EID_MULTI_BSSID_IDX,
						   start, nontx->datalen);
			if (!index || index->datalen < 1 || index->data[0] == 0)
				continue; /* Invalid MBSSID Index element */

			if (index->data[0] == bssid_index) {
				*nontx_profile_found = true;
				if (cfg80211_find_ie(WLAN_EID_RSN,
						     nontx->data,
						     nontx->datalen)) {
					arvif->rsnie_present = true;
					return;
				} else if (!arvif->rsnie_present) {
					return; /* Both tx and nontx BSS are open */
				}

				nie = cfg80211_find_ext_elem(WLAN_EID_EXT_NON_INHERITANCE,
							     nontx->data,
							     nontx->datalen);
				if (!nie || nie->datalen < 2)
					return; /* Invalid non-inheritance element */

				for (i = 1; i < nie->datalen - 1; i++) {
					if (nie->data[i] == WLAN_EID_RSN) {
						arvif->rsnie_present = false;
						break;
					}
				}

				return;
			}
		}
	}
}

static int ath12k_mac_setup_bcn_tmpl_ema(struct ath12k_vif *arvif)
{
	struct ieee80211_bss_conf *bss_conf = &arvif->vif->bss_conf;
	struct ath12k_wmi_bcn_tmpl_ema_arg ema_args;
	struct ieee80211_ema_beacons *beacons;
	struct ath12k_vif *tx_arvif;
	bool nontx_profile_found = false;
	int ret = 0;
	u8 i;

	tx_arvif = ath12k_vif_to_arvif(arvif->vif->mbssid_tx_vif);
	beacons = ieee80211_beacon_get_template_ema_list(ath12k_ar_to_hw(tx_arvif->ar),
							 tx_arvif->vif, 0);
	if (!beacons || !beacons->cnt) {
		ath12k_warn(arvif->ar->ab,
			    "failed to get ema beacon templates from mac80211\n");
		return -EPERM;
	}

	if (tx_arvif == arvif)
		ath12k_mac_set_arvif_ies(arvif, beacons->bcn[0].skb, 0, NULL);

	for (i = 0; i < beacons->cnt; i++) {
		if (tx_arvif != arvif && !nontx_profile_found)
			ath12k_mac_set_arvif_ies(arvif, beacons->bcn[i].skb,
						 bss_conf->bssid_index,
						 &nontx_profile_found);

		ema_args.bcn_cnt = beacons->cnt;
		ema_args.bcn_index = i;
		ret = ath12k_wmi_bcn_tmpl(tx_arvif->ar, tx_arvif->vdev_id,
					  &beacons->bcn[i].offs,
					  beacons->bcn[i].skb, &ema_args);
		if (ret) {
			ath12k_warn(tx_arvif->ar->ab,
				    "failed to set ema beacon template id %i error %d\n",
				    i, ret);
			break;
		}
	}

	if (tx_arvif != arvif && !nontx_profile_found)
		ath12k_warn(arvif->ar->ab,
			    "nontransmitted bssid index %u not found in beacon template\n",
			    bss_conf->bssid_index);

	ieee80211_beacon_free_ema_list(beacons);
	return ret;
}

static int ath12k_mac_setup_bcn_tmpl(struct ath12k_vif *arvif)
{
	struct ath12k_vif *tx_arvif = arvif;
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_vif *vif = arvif->vif;
	struct ieee80211_mutable_offsets offs = {};
	bool nontx_profile_found = false;
	struct sk_buff *bcn;
	int ret;

	if (arvif->vdev_type != WMI_VDEV_TYPE_AP)
		return 0;

	if (vif->mbssid_tx_vif) {
		tx_arvif = ath12k_vif_to_arvif(vif->mbssid_tx_vif);
		if (tx_arvif != arvif && arvif->is_up)
			return 0;

		if (vif->bss_conf.ema_ap)
			return ath12k_mac_setup_bcn_tmpl_ema(arvif);
	}

	bcn = ieee80211_beacon_get_template(ath12k_ar_to_hw(tx_arvif->ar), tx_arvif->vif,
					    &offs, 0);
	if (!bcn) {
		ath12k_warn(ab, "failed to get beacon template from mac80211\n");
		return -EPERM;
	}

	if (tx_arvif == arvif) {
		ath12k_mac_set_arvif_ies(arvif, bcn, 0, NULL);
	} else {
		ath12k_mac_set_arvif_ies(arvif, bcn,
					 arvif->vif->bss_conf.bssid_index,
					 &nontx_profile_found);
		if (!nontx_profile_found)
			ath12k_warn(ab,
				    "nontransmitted profile not found in beacon template\n");
	}

	if (arvif->vif->type == NL80211_IFTYPE_AP && arvif->vif->p2p) {
		ret = ath12k_mac_setup_bcn_p2p_ie(arvif, bcn);
		if (ret) {
			ath12k_warn(ab, "failed to setup P2P GO bcn ie: %d\n",
				    ret);
			goto free_bcn_skb;
		}

		/* P2P IE is inserted by firmware automatically (as
		 * configured above) so remove it from the base beacon
		 * template to avoid duplicate P2P IEs in beacon frames.
		 */
		ret = ath12k_mac_remove_vendor_ie(bcn, WLAN_OUI_WFA,
						  WLAN_OUI_TYPE_WFA_P2P,
						  offsetof(struct ieee80211_mgmt,
							   u.beacon.variable));
		if (ret) {
			ath12k_warn(ab, "failed to remove P2P vendor ie: %d\n",
				    ret);
			goto free_bcn_skb;
		}
	}

	ret = ath12k_wmi_bcn_tmpl(ar, arvif->vdev_id, &offs, bcn, NULL);

	if (ret)
		ath12k_warn(ab, "failed to submit beacon template command: %d\n",
			    ret);

free_bcn_skb:
	kfree_skb(bcn);
	return ret;
}

static void ath12k_control_beaconing(struct ath12k_vif *arvif,
				     struct ieee80211_bss_conf *info)
{
	struct ath12k_wmi_vdev_up_params params = {};
	struct ath12k *ar = arvif->ar;
	int ret;

	lockdep_assert_held(&arvif->ar->conf_mutex);

	if (!info->enable_beacon) {
		ret = ath12k_wmi_vdev_down(ar, arvif->vdev_id);
		if (ret)
			ath12k_warn(ar->ab, "failed to down vdev_id %i: %d\n",
				    arvif->vdev_id, ret);

		arvif->is_up = false;
		return;
	}

	/* Install the beacon template to the FW */
	ret = ath12k_mac_setup_bcn_tmpl(arvif);
	if (ret) {
		ath12k_warn(ar->ab, "failed to update bcn tmpl during vdev up: %d\n",
			    ret);
		return;
	}

	arvif->aid = 0;

	ether_addr_copy(arvif->bssid, info->bssid);

	params.vdev_id = arvif->vdev_id;
	params.aid = arvif->aid;
	params.bssid = arvif->bssid;
	if (arvif->vif->mbssid_tx_vif) {
		params.tx_bssid = ath12k_vif_to_arvif(arvif->vif->mbssid_tx_vif)->bssid;
		params.nontx_profile_idx = info->bssid_index;
		params.nontx_profile_cnt = 1 << info->bssid_indicator;
	}
	ret = ath12k_wmi_vdev_up(arvif->ar, &params);
	if (ret) {
		ath12k_warn(ar->ab, "failed to bring up vdev %d: %i\n",
			    arvif->vdev_id, ret);
		return;
	}

	arvif->is_up = true;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev %d up\n", arvif->vdev_id);
}

static void ath12k_mac_handle_beacon_iter(void *data, u8 *mac,
					  struct ieee80211_vif *vif)
{
	struct sk_buff *skb = data;
	struct ieee80211_mgmt *mgmt = (void *)skb->data;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);

	if (vif->type != NL80211_IFTYPE_STATION)
		return;

	if (!ether_addr_equal(mgmt->bssid, vif->bss_conf.bssid))
		return;

	cancel_delayed_work(&arvif->connection_loss_work);
}

void ath12k_mac_handle_beacon(struct ath12k *ar, struct sk_buff *skb)
{
	ieee80211_iterate_active_interfaces_atomic(ath12k_ar_to_hw(ar),
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath12k_mac_handle_beacon_iter,
						   skb);
}

static void ath12k_mac_handle_beacon_miss_iter(void *data, u8 *mac,
					       struct ieee80211_vif *vif)
{
	u32 *vdev_id = data;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k *ar = arvif->ar;
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);

	if (arvif->vdev_id != *vdev_id)
		return;

	if (!arvif->is_up)
		return;

	ieee80211_beacon_loss(vif);

	/* Firmware doesn't report beacon loss events repeatedly. If AP probe
	 * (done by mac80211) succeeds but beacons do not resume then it
	 * doesn't make sense to continue operation. Queue connection loss work
	 * which can be cancelled when beacon is received.
	 */
	ieee80211_queue_delayed_work(hw, &arvif->connection_loss_work,
				     ATH12K_CONNECTION_LOSS_HZ);
}

void ath12k_mac_handle_beacon_miss(struct ath12k *ar, u32 vdev_id)
{
	ieee80211_iterate_active_interfaces_atomic(ath12k_ar_to_hw(ar),
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath12k_mac_handle_beacon_miss_iter,
						   &vdev_id);
}

static void ath12k_mac_vif_sta_connection_loss_work(struct work_struct *work)
{
	struct ath12k_vif *arvif = container_of(work, struct ath12k_vif,
						connection_loss_work.work);
	struct ieee80211_vif *vif = arvif->vif;

	if (!arvif->is_up)
		return;

	ieee80211_connection_loss(vif);
}

static void ath12k_peer_assoc_h_basic(struct ath12k *ar,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ath12k_wmi_peer_assoc_arg *arg)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	u32 aid;

	lockdep_assert_held(&ar->conf_mutex);

	if (vif->type == NL80211_IFTYPE_STATION)
		aid = vif->cfg.aid;
	else
		aid = sta->aid;

	ether_addr_copy(arg->peer_mac, sta->addr);
	arg->vdev_id = arvif->vdev_id;
	arg->peer_associd = aid;
	arg->auth_flag = true;
	/* TODO: STA WAR in ath10k for listen interval required? */
	arg->peer_listen_intval = hw->conf.listen_interval;
	arg->peer_nss = 1;
	arg->peer_caps = vif->bss_conf.assoc_capability;
}

static void ath12k_peer_assoc_h_crypto(struct ath12k *ar,
				       struct ieee80211_vif *vif,
				       struct ieee80211_sta *sta,
				       struct ath12k_wmi_peer_assoc_arg *arg)
{
	struct ieee80211_bss_conf *info = &vif->bss_conf;
	struct cfg80211_chan_def def;
	struct cfg80211_bss *bss;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	const u8 *rsnie = NULL;
	const u8 *wpaie = NULL;

	lockdep_assert_held(&ar->conf_mutex);

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	bss = cfg80211_get_bss(hw->wiphy, def.chan, info->bssid, NULL, 0,
			       IEEE80211_BSS_TYPE_ANY, IEEE80211_PRIVACY_ANY);

	if (arvif->rsnie_present || arvif->wpaie_present) {
		arg->need_ptk_4_way = true;
		if (arvif->wpaie_present)
			arg->need_gtk_2_way = true;
	} else if (bss) {
		const struct cfg80211_bss_ies *ies;

		rcu_read_lock();
		rsnie = ieee80211_bss_get_ie(bss, WLAN_EID_RSN);

		ies = rcu_dereference(bss->ies);

		wpaie = cfg80211_find_vendor_ie(WLAN_OUI_MICROSOFT,
						WLAN_OUI_TYPE_MICROSOFT_WPA,
						ies->data,
						ies->len);
		rcu_read_unlock();
		cfg80211_put_bss(hw->wiphy, bss);
	}

	/* FIXME: base on RSN IE/WPA IE is a correct idea? */
	if (rsnie || wpaie) {
		ath12k_dbg(ar->ab, ATH12K_DBG_WMI,
			   "%s: rsn ie found\n", __func__);
		arg->need_ptk_4_way = true;
	}

	if (wpaie) {
		ath12k_dbg(ar->ab, ATH12K_DBG_WMI,
			   "%s: wpa ie found\n", __func__);
		arg->need_gtk_2_way = true;
	}

	if (sta->mfp) {
		/* TODO: Need to check if FW supports PMF? */
		arg->is_pmf_enabled = true;
	}

	/* TODO: safe_mode_enabled (bypass 4-way handshake) flag req? */
}

static void ath12k_peer_assoc_h_rates(struct ath12k *ar,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ath12k_wmi_peer_assoc_arg *arg)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct wmi_rate_set_arg *rateset = &arg->peer_legacy_rates;
	struct cfg80211_chan_def def;
	const struct ieee80211_supported_band *sband;
	const struct ieee80211_rate *rates;
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	enum nl80211_band band;
	u32 ratemask;
	u8 rate;
	int i;

	lockdep_assert_held(&ar->conf_mutex);

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	band = def.chan->band;
	sband = hw->wiphy->bands[band];
	ratemask = sta->deflink.supp_rates[band];
	ratemask &= arvif->bitrate_mask.control[band].legacy;
	rates = sband->bitrates;

	rateset->num_rates = 0;

	for (i = 0; i < 32; i++, ratemask >>= 1, rates++) {
		if (!(ratemask & 1))
			continue;

		rate = ath12k_mac_bitrate_to_rate(rates->bitrate);
		rateset->rates[rateset->num_rates] = rate;
		rateset->num_rates++;
	}
}

static bool
ath12k_peer_assoc_h_ht_masked(const u8 *ht_mcs_mask)
{
	int nss;

	for (nss = 0; nss < IEEE80211_HT_MCS_MASK_LEN; nss++)
		if (ht_mcs_mask[nss])
			return false;

	return true;
}

static bool
ath12k_peer_assoc_h_vht_masked(const u16 *vht_mcs_mask)
{
	int nss;

	for (nss = 0; nss < NL80211_VHT_NSS_MAX; nss++)
		if (vht_mcs_mask[nss])
			return false;

	return true;
}

static void ath12k_peer_assoc_h_ht(struct ath12k *ar,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_sta_ht_cap *ht_cap = &sta->deflink.ht_cap;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	int i, n;
	u8 max_nss;
	u32 stbc;

	lockdep_assert_held(&ar->conf_mutex);

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	if (!ht_cap->ht_supported)
		return;

	band = def.chan->band;
	ht_mcs_mask = arvif->bitrate_mask.control[band].ht_mcs;

	if (ath12k_peer_assoc_h_ht_masked(ht_mcs_mask))
		return;

	arg->ht_flag = true;

	arg->peer_max_mpdu = (1 << (IEEE80211_HT_MAX_AMPDU_FACTOR +
				    ht_cap->ampdu_factor)) - 1;

	arg->peer_mpdu_density =
		ath12k_parse_mpdudensity(ht_cap->ampdu_density);

	arg->peer_ht_caps = ht_cap->cap;
	arg->peer_rate_caps |= WMI_HOST_RC_HT_FLAG;

	if (ht_cap->cap & IEEE80211_HT_CAP_LDPC_CODING)
		arg->ldpc_flag = true;

	if (sta->deflink.bandwidth >= IEEE80211_STA_RX_BW_40) {
		arg->bw_40 = true;
		arg->peer_rate_caps |= WMI_HOST_RC_CW40_FLAG;
	}

	if (arvif->bitrate_mask.control[band].gi != NL80211_TXRATE_FORCE_LGI) {
		if (ht_cap->cap & (IEEE80211_HT_CAP_SGI_20 |
		    IEEE80211_HT_CAP_SGI_40))
			arg->peer_rate_caps |= WMI_HOST_RC_SGI_FLAG;
	}

	if (ht_cap->cap & IEEE80211_HT_CAP_TX_STBC) {
		arg->peer_rate_caps |= WMI_HOST_RC_TX_STBC_FLAG;
		arg->stbc_flag = true;
	}

	if (ht_cap->cap & IEEE80211_HT_CAP_RX_STBC) {
		stbc = ht_cap->cap & IEEE80211_HT_CAP_RX_STBC;
		stbc = stbc >> IEEE80211_HT_CAP_RX_STBC_SHIFT;
		stbc = stbc << WMI_HOST_RC_RX_STBC_FLAG_S;
		arg->peer_rate_caps |= stbc;
		arg->stbc_flag = true;
	}

	if (ht_cap->mcs.rx_mask[1] && ht_cap->mcs.rx_mask[2])
		arg->peer_rate_caps |= WMI_HOST_RC_TS_FLAG;
	else if (ht_cap->mcs.rx_mask[1])
		arg->peer_rate_caps |= WMI_HOST_RC_DS_FLAG;

	for (i = 0, n = 0, max_nss = 0; i < IEEE80211_HT_MCS_MASK_LEN * 8; i++)
		if ((ht_cap->mcs.rx_mask[i / 8] & BIT(i % 8)) &&
		    (ht_mcs_mask[i / 8] & BIT(i % 8))) {
			max_nss = (i / 8) + 1;
			arg->peer_ht_rates.rates[n++] = i;
		}

	/* This is a workaround for HT-enabled STAs which break the spec
	 * and have no HT capabilities RX mask (no HT RX MCS map).
	 *
	 * As per spec, in section 20.3.5 Modulation and coding scheme (MCS),
	 * MCS 0 through 7 are mandatory in 20MHz with 800 ns GI at all STAs.
	 *
	 * Firmware asserts if such situation occurs.
	 */
	if (n == 0) {
		arg->peer_ht_rates.num_rates = 8;
		for (i = 0; i < arg->peer_ht_rates.num_rates; i++)
			arg->peer_ht_rates.rates[i] = i;
	} else {
		arg->peer_ht_rates.num_rates = n;
		arg->peer_nss = min(sta->deflink.rx_nss, max_nss);
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac ht peer %pM mcs cnt %d nss %d\n",
		   arg->peer_mac,
		   arg->peer_ht_rates.num_rates,
		   arg->peer_nss);
}

static int ath12k_mac_get_max_vht_mcs_map(u16 mcs_map, int nss)
{
	switch ((mcs_map >> (2 * nss)) & 0x3) {
	case IEEE80211_VHT_MCS_SUPPORT_0_7: return BIT(8) - 1;
	case IEEE80211_VHT_MCS_SUPPORT_0_8: return BIT(9) - 1;
	case IEEE80211_VHT_MCS_SUPPORT_0_9: return BIT(10) - 1;
	}
	return 0;
}

static u16
ath12k_peer_assoc_h_vht_limit(u16 tx_mcs_set,
			      const u16 vht_mcs_limit[NL80211_VHT_NSS_MAX])
{
	int idx_limit;
	int nss;
	u16 mcs_map;
	u16 mcs;

	for (nss = 0; nss < NL80211_VHT_NSS_MAX; nss++) {
		mcs_map = ath12k_mac_get_max_vht_mcs_map(tx_mcs_set, nss) &
			  vht_mcs_limit[nss];

		if (mcs_map)
			idx_limit = fls(mcs_map) - 1;
		else
			idx_limit = -1;

		switch (idx_limit) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			mcs = IEEE80211_VHT_MCS_SUPPORT_0_7;
			break;
		case 8:
			mcs = IEEE80211_VHT_MCS_SUPPORT_0_8;
			break;
		case 9:
			mcs = IEEE80211_VHT_MCS_SUPPORT_0_9;
			break;
		default:
			WARN_ON(1);
			fallthrough;
		case -1:
			mcs = IEEE80211_VHT_MCS_NOT_SUPPORTED;
			break;
		}

		tx_mcs_set &= ~(0x3 << (nss * 2));
		tx_mcs_set |= mcs << (nss * 2);
	}

	return tx_mcs_set;
}

static void ath12k_peer_assoc_h_vht(struct ath12k *ar,
				    struct ieee80211_vif *vif,
				    struct ieee80211_sta *sta,
				    struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_sta_vht_cap *vht_cap = &sta->deflink.vht_cap;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	const u16 *vht_mcs_mask;
	u16 tx_mcs_map;
	u8 ampdu_factor;
	u8 max_nss, vht_mcs;
	int i;

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	if (!vht_cap->vht_supported)
		return;

	band = def.chan->band;
	vht_mcs_mask = arvif->bitrate_mask.control[band].vht_mcs;

	if (ath12k_peer_assoc_h_vht_masked(vht_mcs_mask))
		return;

	arg->vht_flag = true;

	/* TODO: similar flags required? */
	arg->vht_capable = true;

	if (def.chan->band == NL80211_BAND_2GHZ)
		arg->vht_ng_flag = true;

	arg->peer_vht_caps = vht_cap->cap;

	ampdu_factor = (vht_cap->cap &
			IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK) >>
		       IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT;

	/* Workaround: Some Netgear/Linksys 11ac APs set Rx A-MPDU factor to
	 * zero in VHT IE. Using it would result in degraded throughput.
	 * arg->peer_max_mpdu at this point contains HT max_mpdu so keep
	 * it if VHT max_mpdu is smaller.
	 */
	arg->peer_max_mpdu = max(arg->peer_max_mpdu,
				 (1U << (IEEE80211_HT_MAX_AMPDU_FACTOR +
					ampdu_factor)) - 1);

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
		arg->bw_80 = true;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_160)
		arg->bw_160 = true;

	/* Calculate peer NSS capability from VHT capabilities if STA
	 * supports VHT.
	 */
	for (i = 0, max_nss = 0, vht_mcs = 0; i < NL80211_VHT_NSS_MAX; i++) {
		vht_mcs = __le16_to_cpu(vht_cap->vht_mcs.rx_mcs_map) >>
			  (2 * i) & 3;

		if (vht_mcs != IEEE80211_VHT_MCS_NOT_SUPPORTED &&
		    vht_mcs_mask[i])
			max_nss = i + 1;
	}
	arg->peer_nss = min(sta->deflink.rx_nss, max_nss);
	arg->rx_max_rate = __le16_to_cpu(vht_cap->vht_mcs.rx_highest);
	arg->rx_mcs_set = __le16_to_cpu(vht_cap->vht_mcs.rx_mcs_map);
	arg->tx_max_rate = __le16_to_cpu(vht_cap->vht_mcs.tx_highest);

	tx_mcs_map = __le16_to_cpu(vht_cap->vht_mcs.tx_mcs_map);
	arg->tx_mcs_set = ath12k_peer_assoc_h_vht_limit(tx_mcs_map, vht_mcs_mask);

	/* In QCN9274 platform, VHT MCS rate 10 and 11 is enabled by default.
	 * VHT MCS rate 10 and 11 is not supported in 11ac standard.
	 * so explicitly disable the VHT MCS rate 10 and 11 in 11ac mode.
	 */
	arg->tx_mcs_set &= ~IEEE80211_VHT_MCS_SUPPORT_0_11_MASK;
	arg->tx_mcs_set |= IEEE80211_DISABLE_VHT_MCS_SUPPORT_0_11;

	if ((arg->tx_mcs_set & IEEE80211_VHT_MCS_NOT_SUPPORTED) ==
			IEEE80211_VHT_MCS_NOT_SUPPORTED)
		arg->peer_vht_caps &= ~IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;

	/* TODO:  Check */
	arg->tx_max_mcs_nss = 0xFF;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vht peer %pM max_mpdu %d flags 0x%x\n",
		   sta->addr, arg->peer_max_mpdu, arg->peer_flags);

	/* TODO: rxnss_override */
}

static void ath12k_peer_assoc_h_he(struct ath12k *ar,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_sta_he_cap *he_cap = &sta->deflink.he_cap;
	int i;
	u8 ampdu_factor, max_nss;
	u8 rx_mcs_80 = IEEE80211_HE_MCS_NOT_SUPPORTED;
	u8 rx_mcs_160 = IEEE80211_HE_MCS_NOT_SUPPORTED;
	u16 mcs_160_map, mcs_80_map;
	bool support_160;
	u16 v;

	if (!he_cap->has_he)
		return;

	arg->he_flag = true;

	support_160 = !!(he_cap->he_cap_elem.phy_cap_info[0] &
		  IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G);

	/* Supported HE-MCS and NSS Set of peer he_cap is intersection with self he_cp */
	mcs_160_map = le16_to_cpu(he_cap->he_mcs_nss_supp.rx_mcs_160);
	mcs_80_map = le16_to_cpu(he_cap->he_mcs_nss_supp.rx_mcs_80);

	if (support_160) {
		for (i = 7; i >= 0; i--) {
			u8 mcs_160 = (mcs_160_map >> (2 * i)) & 3;

			if (mcs_160 != IEEE80211_HE_MCS_NOT_SUPPORTED) {
				rx_mcs_160 = i + 1;
				break;
			}
		}
	}

	for (i = 7; i >= 0; i--) {
		u8 mcs_80 = (mcs_80_map >> (2 * i)) & 3;

		if (mcs_80 != IEEE80211_HE_MCS_NOT_SUPPORTED) {
			rx_mcs_80 = i + 1;
			break;
		}
	}

	if (support_160)
		max_nss = min(rx_mcs_80, rx_mcs_160);
	else
		max_nss = rx_mcs_80;

	arg->peer_nss = min(sta->deflink.rx_nss, max_nss);

	memcpy(&arg->peer_he_cap_macinfo, he_cap->he_cap_elem.mac_cap_info,
	       sizeof(he_cap->he_cap_elem.mac_cap_info));
	memcpy(&arg->peer_he_cap_phyinfo, he_cap->he_cap_elem.phy_cap_info,
	       sizeof(he_cap->he_cap_elem.phy_cap_info));
	arg->peer_he_ops = vif->bss_conf.he_oper.params;

	/* the top most byte is used to indicate BSS color info */
	arg->peer_he_ops &= 0xffffff;

	/* As per section 26.6.1 IEEE Std 802.11ax‐2022, if the Max AMPDU
	 * Exponent Extension in HE cap is zero, use the arg->peer_max_mpdu
	 * as calculated while parsing VHT caps(if VHT caps is present)
	 * or HT caps (if VHT caps is not present).
	 *
	 * For non-zero value of Max AMPDU Exponent Extension in HE MAC caps,
	 * if a HE STA sends VHT cap and HE cap IE in assoc request then, use
	 * MAX_AMPDU_LEN_FACTOR as 20 to calculate max_ampdu length.
	 * If a HE STA that does not send VHT cap, but HE and HT cap in assoc
	 * request, then use MAX_AMPDU_LEN_FACTOR as 16 to calculate max_ampdu
	 * length.
	 */
	ampdu_factor = u8_get_bits(he_cap->he_cap_elem.mac_cap_info[3],
				   IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_MASK);

	if (ampdu_factor) {
		if (sta->deflink.vht_cap.vht_supported)
			arg->peer_max_mpdu = (1 << (IEEE80211_HE_VHT_MAX_AMPDU_FACTOR +
						    ampdu_factor)) - 1;
		else if (sta->deflink.ht_cap.ht_supported)
			arg->peer_max_mpdu = (1 << (IEEE80211_HE_HT_MAX_AMPDU_FACTOR +
						    ampdu_factor)) - 1;
	}

	if (he_cap->he_cap_elem.phy_cap_info[6] &
	    IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT) {
		int bit = 7;
		int nss, ru;

		arg->peer_ppet.numss_m1 = he_cap->ppe_thres[0] &
					  IEEE80211_PPE_THRES_NSS_MASK;
		arg->peer_ppet.ru_bit_mask =
			(he_cap->ppe_thres[0] &
			 IEEE80211_PPE_THRES_RU_INDEX_BITMASK_MASK) >>
			IEEE80211_PPE_THRES_RU_INDEX_BITMASK_POS;

		for (nss = 0; nss <= arg->peer_ppet.numss_m1; nss++) {
			for (ru = 0; ru < 4; ru++) {
				u32 val = 0;
				int i;

				if ((arg->peer_ppet.ru_bit_mask & BIT(ru)) == 0)
					continue;
				for (i = 0; i < 6; i++) {
					val >>= 1;
					val |= ((he_cap->ppe_thres[bit / 8] >>
						 (bit % 8)) & 0x1) << 5;
					bit++;
				}
				arg->peer_ppet.ppet16_ppet8_ru3_ru0[nss] |=
								val << (ru * 6);
			}
		}
	}

	if (he_cap->he_cap_elem.mac_cap_info[0] & IEEE80211_HE_MAC_CAP0_TWT_RES)
		arg->twt_responder = true;
	if (he_cap->he_cap_elem.mac_cap_info[0] & IEEE80211_HE_MAC_CAP0_TWT_REQ)
		arg->twt_requester = true;

	switch (sta->deflink.bandwidth) {
	case IEEE80211_STA_RX_BW_160:
		if (he_cap->he_cap_elem.phy_cap_info[0] &
		    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G) {
			v = le16_to_cpu(he_cap->he_mcs_nss_supp.rx_mcs_80p80);
			arg->peer_he_rx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_80_80] = v;

			v = le16_to_cpu(he_cap->he_mcs_nss_supp.tx_mcs_80p80);
			arg->peer_he_tx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_80_80] = v;

			arg->peer_he_mcs_count++;
		}
		v = le16_to_cpu(he_cap->he_mcs_nss_supp.rx_mcs_160);
		arg->peer_he_rx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_160] = v;

		v = le16_to_cpu(he_cap->he_mcs_nss_supp.tx_mcs_160);
		arg->peer_he_tx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_160] = v;

		arg->peer_he_mcs_count++;
		fallthrough;

	default:
		v = le16_to_cpu(he_cap->he_mcs_nss_supp.rx_mcs_80);
		arg->peer_he_rx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_80] = v;

		v = le16_to_cpu(he_cap->he_mcs_nss_supp.tx_mcs_80);
		arg->peer_he_tx_mcs_set[WMI_HECAP_TXRX_MCS_NSS_IDX_80] = v;

		arg->peer_he_mcs_count++;
		break;
	}
}

static void ath12k_peer_assoc_h_he_6ghz(struct ath12k *ar,
					struct ieee80211_vif *vif,
					struct ieee80211_sta *sta,
					struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_sta_he_cap *he_cap = &sta->deflink.he_cap;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	u8 ampdu_factor, mpdu_density;

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	band = def.chan->band;

	if (!arg->he_flag || band != NL80211_BAND_6GHZ || !sta->deflink.he_6ghz_capa.capa)
		return;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
		arg->bw_40 = true;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
		arg->bw_80 = true;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_160)
		arg->bw_160 = true;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_320)
		arg->bw_320 = true;

	arg->peer_he_caps_6ghz = le16_to_cpu(sta->deflink.he_6ghz_capa.capa);

	mpdu_density = u32_get_bits(arg->peer_he_caps_6ghz,
				    IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START);
	arg->peer_mpdu_density = ath12k_parse_mpdudensity(mpdu_density);

	/* From IEEE Std 802.11ax-2021 - Section 10.12.2: An HE STA shall be capable of
	 * receiving A-MPDU where the A-MPDU pre-EOF padding length is up to the value
	 * indicated by the Maximum A-MPDU Length Exponent Extension field in the HE
	 * Capabilities element and the Maximum A-MPDU Length Exponent field in HE 6 GHz
	 * Band Capabilities element in the 6 GHz band.
	 *
	 * Here, we are extracting the Max A-MPDU Exponent Extension from HE caps and
	 * factor is the Maximum A-MPDU Length Exponent from HE 6 GHZ Band capability.
	 */
	ampdu_factor = u8_get_bits(he_cap->he_cap_elem.mac_cap_info[3],
				   IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_MASK) +
			u32_get_bits(arg->peer_he_caps_6ghz,
				     IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP);

	arg->peer_max_mpdu = (1u << (IEEE80211_HE_6GHZ_MAX_AMPDU_FACTOR +
				     ampdu_factor)) - 1;
}

static int ath12k_get_smps_from_capa(const struct ieee80211_sta_ht_cap *ht_cap,
				     const struct ieee80211_he_6ghz_capa *he_6ghz_capa,
				     int *smps)
{
	if (ht_cap->ht_supported)
		*smps = u16_get_bits(ht_cap->cap, IEEE80211_HT_CAP_SM_PS);
	else
		*smps = le16_get_bits(he_6ghz_capa->capa,
				      IEEE80211_HE_6GHZ_CAP_SM_PS);

	if (*smps >= ARRAY_SIZE(ath12k_smps_map))
		return -EINVAL;

	return 0;
}

static void ath12k_peer_assoc_h_smps(struct ieee80211_sta *sta,
				     struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_he_6ghz_capa *he_6ghz_capa = &sta->deflink.he_6ghz_capa;
	const struct ieee80211_sta_ht_cap *ht_cap = &sta->deflink.ht_cap;
	int smps;

	if (!ht_cap->ht_supported && !he_6ghz_capa->capa)
		return;

	if (ath12k_get_smps_from_capa(ht_cap, he_6ghz_capa, &smps))
		return;

	switch (smps) {
	case WLAN_HT_CAP_SM_PS_STATIC:
		arg->static_mimops_flag = true;
		break;
	case WLAN_HT_CAP_SM_PS_DYNAMIC:
		arg->dynamic_mimops_flag = true;
		break;
	case WLAN_HT_CAP_SM_PS_DISABLED:
		arg->spatial_mux_flag = true;
		break;
	default:
		break;
	}
}

static void ath12k_peer_assoc_h_qos(struct ath12k *ar,
				    struct ieee80211_vif *vif,
				    struct ieee80211_sta *sta,
				    struct ath12k_wmi_peer_assoc_arg *arg)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);

	switch (arvif->vdev_type) {
	case WMI_VDEV_TYPE_AP:
		if (sta->wme) {
			/* TODO: Check WME vs QoS */
			arg->is_wme_set = true;
			arg->qos_flag = true;
		}

		if (sta->wme && sta->uapsd_queues) {
			/* TODO: Check WME vs QoS */
			arg->is_wme_set = true;
			arg->apsd_flag = true;
			arg->peer_rate_caps |= WMI_HOST_RC_UAPSD_FLAG;
		}
		break;
	case WMI_VDEV_TYPE_STA:
		if (sta->wme) {
			arg->is_wme_set = true;
			arg->qos_flag = true;
		}
		break;
	default:
		break;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac peer %pM qos %d\n",
		   sta->addr, arg->qos_flag);
}

static int ath12k_peer_assoc_qos_ap(struct ath12k *ar,
				    struct ath12k_vif *arvif,
				    struct ieee80211_sta *sta)
{
	struct ath12k_wmi_ap_ps_arg arg;
	u32 max_sp;
	u32 uapsd;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	arg.vdev_id = arvif->vdev_id;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac uapsd_queues 0x%x max_sp %d\n",
		   sta->uapsd_queues, sta->max_sp);

	uapsd = 0;
	if (sta->uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_VO)
		uapsd |= WMI_AP_PS_UAPSD_AC3_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC3_TRIGGER_EN;
	if (sta->uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_VI)
		uapsd |= WMI_AP_PS_UAPSD_AC2_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC2_TRIGGER_EN;
	if (sta->uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_BK)
		uapsd |= WMI_AP_PS_UAPSD_AC1_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC1_TRIGGER_EN;
	if (sta->uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_BE)
		uapsd |= WMI_AP_PS_UAPSD_AC0_DELIVERY_EN |
			 WMI_AP_PS_UAPSD_AC0_TRIGGER_EN;

	max_sp = 0;
	if (sta->max_sp < MAX_WMI_AP_PS_PEER_PARAM_MAX_SP)
		max_sp = sta->max_sp;

	arg.param = WMI_AP_PS_PEER_PARAM_UAPSD;
	arg.value = uapsd;
	ret = ath12k_wmi_send_set_ap_ps_param_cmd(ar, sta->addr, &arg);
	if (ret)
		goto err;

	arg.param = WMI_AP_PS_PEER_PARAM_MAX_SP;
	arg.value = max_sp;
	ret = ath12k_wmi_send_set_ap_ps_param_cmd(ar, sta->addr, &arg);
	if (ret)
		goto err;

	/* TODO: revisit during testing */
	arg.param = WMI_AP_PS_PEER_PARAM_SIFS_RESP_FRMTYPE;
	arg.value = DISABLE_SIFS_RESPONSE_TRIGGER;
	ret = ath12k_wmi_send_set_ap_ps_param_cmd(ar, sta->addr, &arg);
	if (ret)
		goto err;

	arg.param = WMI_AP_PS_PEER_PARAM_SIFS_RESP_UAPSD;
	arg.value = DISABLE_SIFS_RESPONSE_TRIGGER;
	ret = ath12k_wmi_send_set_ap_ps_param_cmd(ar, sta->addr, &arg);
	if (ret)
		goto err;

	return 0;

err:
	ath12k_warn(ar->ab, "failed to set ap ps peer param %d for vdev %i: %d\n",
		    arg.param, arvif->vdev_id, ret);
	return ret;
}

static bool ath12k_mac_sta_has_ofdm_only(struct ieee80211_sta *sta)
{
	return sta->deflink.supp_rates[NL80211_BAND_2GHZ] >>
	       ATH12K_MAC_FIRST_OFDM_RATE_IDX;
}

static enum wmi_phy_mode ath12k_mac_get_phymode_vht(struct ath12k *ar,
						    struct ieee80211_sta *sta)
{
	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_160) {
		switch (sta->deflink.vht_cap.cap &
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK) {
		case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ:
			return MODE_11AC_VHT160;
		case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ:
			return MODE_11AC_VHT80_80;
		default:
			/* not sure if this is a valid case? */
			return MODE_11AC_VHT160;
		}
	}

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
		return MODE_11AC_VHT80;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
		return MODE_11AC_VHT40;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_20)
		return MODE_11AC_VHT20;

	return MODE_UNKNOWN;
}

static enum wmi_phy_mode ath12k_mac_get_phymode_he(struct ath12k *ar,
						   struct ieee80211_sta *sta)
{
	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_160) {
		if (sta->deflink.he_cap.he_cap_elem.phy_cap_info[0] &
		     IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G)
			return MODE_11AX_HE160;
		else if (sta->deflink.he_cap.he_cap_elem.phy_cap_info[0] &
		     IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)
			return MODE_11AX_HE80_80;
		/* not sure if this is a valid case? */
		return MODE_11AX_HE160;
	}

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
		return MODE_11AX_HE80;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
		return MODE_11AX_HE40;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_20)
		return MODE_11AX_HE20;

	return MODE_UNKNOWN;
}

static enum wmi_phy_mode ath12k_mac_get_phymode_eht(struct ath12k *ar,
						    struct ieee80211_sta *sta)
{
	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_320)
		if (sta->deflink.eht_cap.eht_cap_elem.phy_cap_info[0] &
		    IEEE80211_EHT_PHY_CAP0_320MHZ_IN_6GHZ)
			return MODE_11BE_EHT320;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_160) {
		if (sta->deflink.he_cap.he_cap_elem.phy_cap_info[0] &
		    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G)
			return MODE_11BE_EHT160;

		if (sta->deflink.he_cap.he_cap_elem.phy_cap_info[0] &
			 IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)
			return MODE_11BE_EHT80_80;

		ath12k_warn(ar->ab, "invalid EHT PHY capability info for 160 Mhz: %d\n",
			    sta->deflink.he_cap.he_cap_elem.phy_cap_info[0]);

		return MODE_11BE_EHT160;
	}

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
		return MODE_11BE_EHT80;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
		return MODE_11BE_EHT40;

	if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_20)
		return MODE_11BE_EHT20;

	return MODE_UNKNOWN;
}

static void ath12k_peer_assoc_h_phymode(struct ath12k *ar,
					struct ieee80211_vif *vif,
					struct ieee80211_sta *sta,
					struct ath12k_wmi_peer_assoc_arg *arg)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	const u16 *vht_mcs_mask;
	enum wmi_phy_mode phymode = MODE_UNKNOWN;

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return;

	band = def.chan->band;
	ht_mcs_mask = arvif->bitrate_mask.control[band].ht_mcs;
	vht_mcs_mask = arvif->bitrate_mask.control[band].vht_mcs;

	switch (band) {
	case NL80211_BAND_2GHZ:
		if (sta->deflink.eht_cap.has_eht) {
			if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
				phymode = MODE_11BE_EHT40_2G;
			else
				phymode = MODE_11BE_EHT20_2G;
		} else if (sta->deflink.he_cap.has_he) {
			if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_80)
				phymode = MODE_11AX_HE80_2G;
			else if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
				phymode = MODE_11AX_HE40_2G;
			else
				phymode = MODE_11AX_HE20_2G;
		} else if (sta->deflink.vht_cap.vht_supported &&
		    !ath12k_peer_assoc_h_vht_masked(vht_mcs_mask)) {
			if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
				phymode = MODE_11AC_VHT40;
			else
				phymode = MODE_11AC_VHT20;
		} else if (sta->deflink.ht_cap.ht_supported &&
			   !ath12k_peer_assoc_h_ht_masked(ht_mcs_mask)) {
			if (sta->deflink.bandwidth == IEEE80211_STA_RX_BW_40)
				phymode = MODE_11NG_HT40;
			else
				phymode = MODE_11NG_HT20;
		} else if (ath12k_mac_sta_has_ofdm_only(sta)) {
			phymode = MODE_11G;
		} else {
			phymode = MODE_11B;
		}
		break;
	case NL80211_BAND_5GHZ:
	case NL80211_BAND_6GHZ:
		/* Check EHT first */
		if (sta->deflink.eht_cap.has_eht) {
			phymode = ath12k_mac_get_phymode_eht(ar, sta);
		} else if (sta->deflink.he_cap.has_he) {
			phymode = ath12k_mac_get_phymode_he(ar, sta);
		} else if (sta->deflink.vht_cap.vht_supported &&
		    !ath12k_peer_assoc_h_vht_masked(vht_mcs_mask)) {
			phymode = ath12k_mac_get_phymode_vht(ar, sta);
		} else if (sta->deflink.ht_cap.ht_supported &&
			   !ath12k_peer_assoc_h_ht_masked(ht_mcs_mask)) {
			if (sta->deflink.bandwidth >= IEEE80211_STA_RX_BW_40)
				phymode = MODE_11NA_HT40;
			else
				phymode = MODE_11NA_HT20;
		} else {
			phymode = MODE_11A;
		}
		break;
	default:
		break;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac peer %pM phymode %s\n",
		   sta->addr, ath12k_mac_phymode_str(phymode));

	arg->peer_phymode = phymode;
	WARN_ON(phymode == MODE_UNKNOWN);
}

static void ath12k_mac_set_eht_mcs(u8 rx_tx_mcs7, u8 rx_tx_mcs9,
				   u8 rx_tx_mcs11, u8 rx_tx_mcs13,
				   u32 *rx_mcs, u32 *tx_mcs)
{
	*rx_mcs = 0;
	u32p_replace_bits(rx_mcs,
			  u8_get_bits(rx_tx_mcs7, IEEE80211_EHT_MCS_NSS_RX),
			  WMI_EHT_MCS_NSS_0_7);
	u32p_replace_bits(rx_mcs,
			  u8_get_bits(rx_tx_mcs9, IEEE80211_EHT_MCS_NSS_RX),
			  WMI_EHT_MCS_NSS_8_9);
	u32p_replace_bits(rx_mcs,
			  u8_get_bits(rx_tx_mcs11, IEEE80211_EHT_MCS_NSS_RX),
			  WMI_EHT_MCS_NSS_10_11);
	u32p_replace_bits(rx_mcs,
			  u8_get_bits(rx_tx_mcs13, IEEE80211_EHT_MCS_NSS_RX),
			  WMI_EHT_MCS_NSS_12_13);

	*tx_mcs = 0;
	u32p_replace_bits(tx_mcs,
			  u8_get_bits(rx_tx_mcs7, IEEE80211_EHT_MCS_NSS_TX),
			  WMI_EHT_MCS_NSS_0_7);
	u32p_replace_bits(tx_mcs,
			  u8_get_bits(rx_tx_mcs9, IEEE80211_EHT_MCS_NSS_TX),
			  WMI_EHT_MCS_NSS_8_9);
	u32p_replace_bits(tx_mcs,
			  u8_get_bits(rx_tx_mcs11, IEEE80211_EHT_MCS_NSS_TX),
			  WMI_EHT_MCS_NSS_10_11);
	u32p_replace_bits(tx_mcs,
			  u8_get_bits(rx_tx_mcs13, IEEE80211_EHT_MCS_NSS_TX),
			  WMI_EHT_MCS_NSS_12_13);
}

static void ath12k_mac_set_eht_ppe_threshold(const u8 *ppe_thres,
					     struct ath12k_wmi_ppe_threshold_arg *ppet)
{
	u32 bit_pos = IEEE80211_EHT_PPE_THRES_INFO_HEADER_SIZE, val;
	u8 nss, ru, i;
	u8 ppet_bit_len_per_ru = IEEE80211_EHT_PPE_THRES_INFO_PPET_SIZE * 2;

	ppet->numss_m1 = u8_get_bits(ppe_thres[0], IEEE80211_EHT_PPE_THRES_NSS_MASK);
	ppet->ru_bit_mask = u16_get_bits(get_unaligned_le16(ppe_thres),
					 IEEE80211_EHT_PPE_THRES_RU_INDEX_BITMASK_MASK);

	for (nss = 0; nss <= ppet->numss_m1; nss++) {
		for (ru = 0;
		     ru < hweight16(IEEE80211_EHT_PPE_THRES_RU_INDEX_BITMASK_MASK);
		     ru++) {
			if ((ppet->ru_bit_mask & BIT(ru)) == 0)
				continue;

			val = 0;
			for (i = 0; i < ppet_bit_len_per_ru; i++) {
				val |= (((ppe_thres[bit_pos / 8] >>
					  (bit_pos % 8)) & 0x1) << i);
				bit_pos++;
			}
			ppet->ppet16_ppet8_ru3_ru0[nss] |=
					(val << (ru * ppet_bit_len_per_ru));
		}
	}
}

static void ath12k_peer_assoc_h_eht(struct ath12k *ar,
				    struct ieee80211_vif *vif,
				    struct ieee80211_sta *sta,
				    struct ath12k_wmi_peer_assoc_arg *arg)
{
	const struct ieee80211_sta_eht_cap *eht_cap = &sta->deflink.eht_cap;
	const struct ieee80211_sta_he_cap *he_cap = &sta->deflink.he_cap;
	const struct ieee80211_eht_mcs_nss_supp_20mhz_only *bw_20;
	const struct ieee80211_eht_mcs_nss_supp_bw *bw;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	u32 *rx_mcs, *tx_mcs;

	if (!sta->deflink.he_cap.has_he || !eht_cap->has_eht)
		return;

	arg->eht_flag = true;

	if ((eht_cap->eht_cap_elem.phy_cap_info[5] &
	     IEEE80211_EHT_PHY_CAP5_PPE_THRESHOLD_PRESENT) &&
	    eht_cap->eht_ppe_thres[0] != 0)
		ath12k_mac_set_eht_ppe_threshold(eht_cap->eht_ppe_thres,
						 &arg->peer_eht_ppet);

	memcpy(arg->peer_eht_cap_mac, eht_cap->eht_cap_elem.mac_cap_info,
	       sizeof(eht_cap->eht_cap_elem.mac_cap_info));
	memcpy(arg->peer_eht_cap_phy, eht_cap->eht_cap_elem.phy_cap_info,
	       sizeof(eht_cap->eht_cap_elem.phy_cap_info));

	rx_mcs = arg->peer_eht_rx_mcs_set;
	tx_mcs = arg->peer_eht_tx_mcs_set;

	switch (sta->deflink.bandwidth) {
	case IEEE80211_STA_RX_BW_320:
		bw = &eht_cap->eht_mcs_nss_supp.bw._320;
		ath12k_mac_set_eht_mcs(bw->rx_tx_mcs9_max_nss,
				       bw->rx_tx_mcs9_max_nss,
				       bw->rx_tx_mcs11_max_nss,
				       bw->rx_tx_mcs13_max_nss,
				       &rx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_320],
				       &tx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_320]);
		arg->peer_eht_mcs_count++;
		fallthrough;
	case IEEE80211_STA_RX_BW_160:
		bw = &eht_cap->eht_mcs_nss_supp.bw._160;
		ath12k_mac_set_eht_mcs(bw->rx_tx_mcs9_max_nss,
				       bw->rx_tx_mcs9_max_nss,
				       bw->rx_tx_mcs11_max_nss,
				       bw->rx_tx_mcs13_max_nss,
				       &rx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_160],
				       &tx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_160]);
		arg->peer_eht_mcs_count++;
		fallthrough;
	default:
		if ((he_cap->he_cap_elem.phy_cap_info[0] &
		     (IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G |
		      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G |
		      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G |
		      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)) == 0) {
			bw_20 = &eht_cap->eht_mcs_nss_supp.only_20mhz;

			ath12k_mac_set_eht_mcs(bw_20->rx_tx_mcs7_max_nss,
					       bw_20->rx_tx_mcs9_max_nss,
					       bw_20->rx_tx_mcs11_max_nss,
					       bw_20->rx_tx_mcs13_max_nss,
					       &rx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_80],
					       &tx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_80]);
		} else {
			bw = &eht_cap->eht_mcs_nss_supp.bw._80;
			ath12k_mac_set_eht_mcs(bw->rx_tx_mcs9_max_nss,
					       bw->rx_tx_mcs9_max_nss,
					       bw->rx_tx_mcs11_max_nss,
					       bw->rx_tx_mcs13_max_nss,
					       &rx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_80],
					       &tx_mcs[WMI_EHTCAP_TXRX_MCS_NSS_IDX_80]);
		}

		arg->peer_eht_mcs_count++;
		break;
	}

	arg->punct_bitmap = ~arvif->punct_bitmap;
}

static void ath12k_peer_assoc_prepare(struct ath12k *ar,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ath12k_wmi_peer_assoc_arg *arg,
				      bool reassoc)
{
	lockdep_assert_held(&ar->conf_mutex);

	memset(arg, 0, sizeof(*arg));

	reinit_completion(&ar->peer_assoc_done);

	arg->peer_new_assoc = !reassoc;
	ath12k_peer_assoc_h_basic(ar, vif, sta, arg);
	ath12k_peer_assoc_h_crypto(ar, vif, sta, arg);
	ath12k_peer_assoc_h_rates(ar, vif, sta, arg);
	ath12k_peer_assoc_h_ht(ar, vif, sta, arg);
	ath12k_peer_assoc_h_vht(ar, vif, sta, arg);
	ath12k_peer_assoc_h_he(ar, vif, sta, arg);
	ath12k_peer_assoc_h_he_6ghz(ar, vif, sta, arg);
	ath12k_peer_assoc_h_eht(ar, vif, sta, arg);
	ath12k_peer_assoc_h_qos(ar, vif, sta, arg);
	ath12k_peer_assoc_h_phymode(ar, vif, sta, arg);
	ath12k_peer_assoc_h_smps(sta, arg);

	/* TODO: amsdu_disable req? */
}

static int ath12k_setup_peer_smps(struct ath12k *ar, struct ath12k_vif *arvif,
				  const u8 *addr,
				  const struct ieee80211_sta_ht_cap *ht_cap,
				  const struct ieee80211_he_6ghz_capa *he_6ghz_capa)
{
	int smps, ret = 0;

	if (!ht_cap->ht_supported && !he_6ghz_capa)
		return 0;

	ret = ath12k_get_smps_from_capa(ht_cap, he_6ghz_capa, &smps);
	if (ret < 0)
		return ret;

	return ath12k_wmi_set_peer_param(ar, addr, arvif->vdev_id,
					 WMI_PEER_MIMO_PS_STATE,
					 ath12k_smps_map[smps]);
}

static void ath12k_bss_assoc(struct ath12k *ar,
			     struct ath12k_vif *arvif,
			     struct ieee80211_bss_conf *bss_conf)
{
	struct ieee80211_vif *vif = arvif->vif;
	struct ath12k_wmi_vdev_up_params params = {};
	struct ath12k_wmi_peer_assoc_arg peer_arg;
	struct ieee80211_sta *ap_sta;
	struct ath12k_peer *peer;
	bool is_auth = false;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev %i assoc bssid %pM aid %d\n",
		   arvif->vdev_id, arvif->bssid, arvif->aid);

	rcu_read_lock();

	ap_sta = ieee80211_find_sta(vif, bss_conf->bssid);
	if (!ap_sta) {
		ath12k_warn(ar->ab, "failed to find station entry for bss %pM vdev %i\n",
			    bss_conf->bssid, arvif->vdev_id);
		rcu_read_unlock();
		return;
	}

	ath12k_peer_assoc_prepare(ar, vif, ap_sta, &peer_arg, false);

	rcu_read_unlock();

	ret = ath12k_wmi_send_peer_assoc_cmd(ar, &peer_arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to run peer assoc for %pM vdev %i: %d\n",
			    bss_conf->bssid, arvif->vdev_id, ret);
		return;
	}

	if (!wait_for_completion_timeout(&ar->peer_assoc_done, 1 * HZ)) {
		ath12k_warn(ar->ab, "failed to get peer assoc conf event for %pM vdev %i\n",
			    bss_conf->bssid, arvif->vdev_id);
		return;
	}

	ret = ath12k_setup_peer_smps(ar, arvif, bss_conf->bssid,
				     &ap_sta->deflink.ht_cap,
				     &ap_sta->deflink.he_6ghz_capa);
	if (ret) {
		ath12k_warn(ar->ab, "failed to setup peer SMPS for vdev %d: %d\n",
			    arvif->vdev_id, ret);
		return;
	}

	WARN_ON(arvif->is_up);

	arvif->aid = vif->cfg.aid;
	ether_addr_copy(arvif->bssid, bss_conf->bssid);

	params.vdev_id = arvif->vdev_id;
	params.aid = arvif->aid;
	params.bssid = arvif->bssid;
	ret = ath12k_wmi_vdev_up(ar, &params);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set vdev %d up: %d\n",
			    arvif->vdev_id, ret);
		return;
	}

	arvif->is_up = true;
	arvif->rekey_data.enable_offload = false;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "mac vdev %d up (associated) bssid %pM aid %d\n",
		   arvif->vdev_id, bss_conf->bssid, vif->cfg.aid);

	spin_lock_bh(&ar->ab->base_lock);

	peer = ath12k_peer_find(ar->ab, arvif->vdev_id, arvif->bssid);
	if (peer && peer->is_authorized)
		is_auth = true;

	spin_unlock_bh(&ar->ab->base_lock);

	/* Authorize BSS Peer */
	if (is_auth) {
		ret = ath12k_wmi_set_peer_param(ar, arvif->bssid,
						arvif->vdev_id,
						WMI_PEER_AUTHORIZE,
						1);
		if (ret)
			ath12k_warn(ar->ab, "Unable to authorize BSS peer: %d\n", ret);
	}

	ret = ath12k_wmi_send_obss_spr_cmd(ar, arvif->vdev_id,
					   &bss_conf->he_obss_pd);
	if (ret)
		ath12k_warn(ar->ab, "failed to set vdev %i OBSS PD parameters: %d\n",
			    arvif->vdev_id, ret);
}

static void ath12k_bss_disassoc(struct ath12k *ar,
				struct ath12k_vif *arvif)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev %i disassoc bssid %pM\n",
		   arvif->vdev_id, arvif->bssid);

	ret = ath12k_wmi_vdev_down(ar, arvif->vdev_id);
	if (ret)
		ath12k_warn(ar->ab, "failed to down vdev %i: %d\n",
			    arvif->vdev_id, ret);

	arvif->is_up = false;

	memset(&arvif->rekey_data, 0, sizeof(arvif->rekey_data));

	cancel_delayed_work(&arvif->connection_loss_work);
}

static u32 ath12k_mac_get_rate_hw_value(int bitrate)
{
	u32 preamble;
	u16 hw_value;
	int rate;
	size_t i;

	if (ath12k_mac_bitrate_is_cck(bitrate))
		preamble = WMI_RATE_PREAMBLE_CCK;
	else
		preamble = WMI_RATE_PREAMBLE_OFDM;

	for (i = 0; i < ARRAY_SIZE(ath12k_legacy_rates); i++) {
		if (ath12k_legacy_rates[i].bitrate != bitrate)
			continue;

		hw_value = ath12k_legacy_rates[i].hw_value;
		rate = ATH12K_HW_RATE_CODE(hw_value, 0, preamble);

		return rate;
	}

	return -EINVAL;
}

static void ath12k_recalculate_mgmt_rate(struct ath12k *ar,
					 struct ieee80211_vif *vif,
					 struct cfg80211_chan_def *def)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	const struct ieee80211_supported_band *sband;
	u8 basic_rate_idx;
	int hw_rate_code;
	u32 vdev_param;
	u16 bitrate;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	sband = hw->wiphy->bands[def->chan->band];
	basic_rate_idx = ffs(vif->bss_conf.basic_rates) - 1;
	bitrate = sband->bitrates[basic_rate_idx].bitrate;

	hw_rate_code = ath12k_mac_get_rate_hw_value(bitrate);
	if (hw_rate_code < 0) {
		ath12k_warn(ar->ab, "bitrate not supported %d\n", bitrate);
		return;
	}

	vdev_param = WMI_VDEV_PARAM_MGMT_RATE;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id, vdev_param,
					    hw_rate_code);
	if (ret)
		ath12k_warn(ar->ab, "failed to set mgmt tx rate %d\n", ret);

	vdev_param = WMI_VDEV_PARAM_BEACON_RATE;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id, vdev_param,
					    hw_rate_code);
	if (ret)
		ath12k_warn(ar->ab, "failed to set beacon tx rate %d\n", ret);
}

static int ath12k_mac_fils_discovery(struct ath12k_vif *arvif,
				     struct ieee80211_bss_conf *info)
{
	struct ath12k *ar = arvif->ar;
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);
	struct sk_buff *tmpl;
	int ret;
	u32 interval;
	bool unsol_bcast_probe_resp_enabled = false;

	if (info->fils_discovery.max_interval) {
		interval = info->fils_discovery.max_interval;

		tmpl = ieee80211_get_fils_discovery_tmpl(hw, arvif->vif);
		if (tmpl)
			ret = ath12k_wmi_fils_discovery_tmpl(ar, arvif->vdev_id,
							     tmpl);
	} else if (info->unsol_bcast_probe_resp_interval) {
		unsol_bcast_probe_resp_enabled = 1;
		interval = info->unsol_bcast_probe_resp_interval;

		tmpl = ieee80211_get_unsol_bcast_probe_resp_tmpl(hw,
								 arvif->vif);
		if (tmpl)
			ret = ath12k_wmi_probe_resp_tmpl(ar, arvif->vdev_id,
							 tmpl);
	} else { /* Disable */
		return ath12k_wmi_fils_discovery(ar, arvif->vdev_id, 0, false);
	}

	if (!tmpl) {
		ath12k_warn(ar->ab,
			    "mac vdev %i failed to retrieve %s template\n",
			    arvif->vdev_id, (unsol_bcast_probe_resp_enabled ?
			    "unsolicited broadcast probe response" :
			    "FILS discovery"));
		return -EPERM;
	}
	kfree_skb(tmpl);

	if (!ret)
		ret = ath12k_wmi_fils_discovery(ar, arvif->vdev_id, interval,
						unsol_bcast_probe_resp_enabled);

	return ret;
}

static void ath12k_mac_vif_setup_ps(struct ath12k_vif *arvif)
{
	struct ath12k *ar = arvif->ar;
	struct ieee80211_vif *vif = arvif->vif;
	struct ieee80211_conf *conf = &ath12k_ar_to_hw(ar)->conf;
	enum wmi_sta_powersave_param param;
	enum wmi_sta_ps_mode psmode;
	int ret;
	int timeout;
	bool enable_ps;

	lockdep_assert_held(&ar->conf_mutex);

	if (vif->type != NL80211_IFTYPE_STATION)
		return;

	enable_ps = arvif->ps;
	if (enable_ps) {
		psmode = WMI_STA_PS_MODE_ENABLED;
		param = WMI_STA_PS_PARAM_INACTIVITY_TIME;

		timeout = conf->dynamic_ps_timeout;
		if (timeout == 0) {
			/* firmware doesn't like 0 */
			timeout = ieee80211_tu_to_usec(vif->bss_conf.beacon_int) / 1000;
		}

		ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id, param,
						  timeout);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set inactivity time for vdev %d: %i\n",
				    arvif->vdev_id, ret);
			return;
		}
	} else {
		psmode = WMI_STA_PS_MODE_DISABLED;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev %d psmode %s\n",
		   arvif->vdev_id, psmode ? "enable" : "disable");

	ret = ath12k_wmi_pdev_set_ps_mode(ar, arvif->vdev_id, psmode);
	if (ret)
		ath12k_warn(ar->ab, "failed to set sta power save mode %d for vdev %d: %d\n",
			    psmode, arvif->vdev_id, ret);
}

static void ath12k_mac_bss_info_changed(struct ath12k *ar,
					struct ath12k_vif *arvif,
					struct ieee80211_bss_conf *info,
					u64 changed)
{
	struct ieee80211_vif *vif = arvif->vif;
	struct ieee80211_vif_cfg *vif_cfg = &vif->cfg;
	struct cfg80211_chan_def def;
	u32 param_id, param_value;
	enum nl80211_band band;
	u32 vdev_param;
	int mcast_rate;
	u32 preamble;
	u16 hw_value;
	u16 bitrate;
	int ret;
	u8 rateidx;
	u32 rate;

	lockdep_assert_held(&ar->conf_mutex);

	if (changed & BSS_CHANGED_BEACON_INT) {
		arvif->beacon_interval = info->beacon_int;

		param_id = WMI_VDEV_PARAM_BEACON_INTERVAL;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param_id,
						    arvif->beacon_interval);
		if (ret)
			ath12k_warn(ar->ab, "Failed to set beacon interval for VDEV: %d\n",
				    arvif->vdev_id);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "Beacon interval: %d set for VDEV: %d\n",
				   arvif->beacon_interval, arvif->vdev_id);
	}

	if (changed & BSS_CHANGED_BEACON) {
		param_id = WMI_PDEV_PARAM_BEACON_TX_MODE;
		param_value = WMI_BEACON_BURST_MODE;
		ret = ath12k_wmi_pdev_set_param(ar, param_id,
						param_value, ar->pdev->pdev_id);
		if (ret)
			ath12k_warn(ar->ab, "Failed to set beacon mode for VDEV: %d\n",
				    arvif->vdev_id);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "Set burst beacon mode for VDEV: %d\n",
				   arvif->vdev_id);

		ret = ath12k_mac_setup_bcn_tmpl(arvif);
		if (ret)
			ath12k_warn(ar->ab, "failed to update bcn template: %d\n",
				    ret);
	}

	if (changed & (BSS_CHANGED_BEACON_INFO | BSS_CHANGED_BEACON)) {
		arvif->dtim_period = info->dtim_period;

		param_id = WMI_VDEV_PARAM_DTIM_PERIOD;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param_id,
						    arvif->dtim_period);

		if (ret)
			ath12k_warn(ar->ab, "Failed to set dtim period for VDEV %d: %i\n",
				    arvif->vdev_id, ret);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "DTIM period: %d set for VDEV: %d\n",
				   arvif->dtim_period, arvif->vdev_id);
	}

	if (changed & BSS_CHANGED_SSID &&
	    vif->type == NL80211_IFTYPE_AP) {
		arvif->u.ap.ssid_len = vif->cfg.ssid_len;
		if (vif->cfg.ssid_len)
			memcpy(arvif->u.ap.ssid, vif->cfg.ssid, vif->cfg.ssid_len);
		arvif->u.ap.hidden_ssid = info->hidden_ssid;
	}

	if (changed & BSS_CHANGED_BSSID && !is_zero_ether_addr(info->bssid))
		ether_addr_copy(arvif->bssid, info->bssid);

	if (changed & BSS_CHANGED_BEACON_ENABLED) {
		ath12k_control_beaconing(arvif, info);

		if (arvif->is_up && vif->bss_conf.he_support &&
		    vif->bss_conf.he_oper.params) {
			/* TODO: Extend to support 1024 BA Bitmap size */
			ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
							    WMI_VDEV_PARAM_BA_MODE,
							    WMI_BA_MODE_BUFFER_SIZE_256);
			if (ret)
				ath12k_warn(ar->ab,
					    "failed to set BA BUFFER SIZE 256 for vdev: %d\n",
					    arvif->vdev_id);

			param_id = WMI_VDEV_PARAM_HEOPS_0_31;
			param_value = vif->bss_conf.he_oper.params;
			ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
							    param_id, param_value);
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "he oper param: %x set for VDEV: %d\n",
				   param_value, arvif->vdev_id);

			if (ret)
				ath12k_warn(ar->ab, "Failed to set he oper params %x for VDEV %d: %i\n",
					    param_value, arvif->vdev_id, ret);
		}
	}

	if (changed & BSS_CHANGED_ERP_CTS_PROT) {
		u32 cts_prot;

		cts_prot = !!(info->use_cts_prot);
		param_id = WMI_VDEV_PARAM_PROTECTION_MODE;

		if (arvif->is_started) {
			ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
							    param_id, cts_prot);
			if (ret)
				ath12k_warn(ar->ab, "Failed to set CTS prot for VDEV: %d\n",
					    arvif->vdev_id);
			else
				ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "Set CTS prot: %d for VDEV: %d\n",
					   cts_prot, arvif->vdev_id);
		} else {
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "defer protection mode setup, vdev is not ready yet\n");
		}
	}

	if (changed & BSS_CHANGED_ERP_SLOT) {
		u32 slottime;

		if (info->use_short_slot)
			slottime = WMI_VDEV_SLOT_TIME_SHORT; /* 9us */

		else
			slottime = WMI_VDEV_SLOT_TIME_LONG; /* 20us */

		param_id = WMI_VDEV_PARAM_SLOT_TIME;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param_id, slottime);
		if (ret)
			ath12k_warn(ar->ab, "Failed to set erp slot for VDEV: %d\n",
				    arvif->vdev_id);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "Set slottime: %d for VDEV: %d\n",
				   slottime, arvif->vdev_id);
	}

	if (changed & BSS_CHANGED_ERP_PREAMBLE) {
		u32 preamble;

		if (info->use_short_preamble)
			preamble = WMI_VDEV_PREAMBLE_SHORT;
		else
			preamble = WMI_VDEV_PREAMBLE_LONG;

		param_id = WMI_VDEV_PARAM_PREAMBLE;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param_id, preamble);
		if (ret)
			ath12k_warn(ar->ab, "Failed to set preamble for VDEV: %d\n",
				    arvif->vdev_id);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
				   "Set preamble: %d for VDEV: %d\n",
				   preamble, arvif->vdev_id);
	}

	if (changed & BSS_CHANGED_ASSOC) {
		if (vif->cfg.assoc)
			ath12k_bss_assoc(ar, arvif, info);
		else
			ath12k_bss_disassoc(ar, arvif);
	}

	if (changed & BSS_CHANGED_TXPOWER) {
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev_id %i txpower %d\n",
			   arvif->vdev_id, info->txpower);

		arvif->txpower = info->txpower;
		ath12k_mac_txpower_recalc(ar);
	}

	if (changed & BSS_CHANGED_MCAST_RATE &&
	    !ath12k_mac_vif_chan(arvif->vif, &def)) {
		band = def.chan->band;
		mcast_rate = vif->bss_conf.mcast_rate[band];

		if (mcast_rate > 0)
			rateidx = mcast_rate - 1;
		else
			rateidx = ffs(vif->bss_conf.basic_rates) - 1;

		if (ar->pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP)
			rateidx += ATH12K_MAC_FIRST_OFDM_RATE_IDX;

		bitrate = ath12k_legacy_rates[rateidx].bitrate;
		hw_value = ath12k_legacy_rates[rateidx].hw_value;

		if (ath12k_mac_bitrate_is_cck(bitrate))
			preamble = WMI_RATE_PREAMBLE_CCK;
		else
			preamble = WMI_RATE_PREAMBLE_OFDM;

		rate = ATH12K_HW_RATE_CODE(hw_value, 0, preamble);

		ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
			   "mac vdev %d mcast_rate %x\n",
			   arvif->vdev_id, rate);

		vdev_param = WMI_VDEV_PARAM_MCAST_DATA_RATE;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    vdev_param, rate);
		if (ret)
			ath12k_warn(ar->ab,
				    "failed to set mcast rate on vdev %i: %d\n",
				    arvif->vdev_id,  ret);

		vdev_param = WMI_VDEV_PARAM_BCAST_DATA_RATE;
		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    vdev_param, rate);
		if (ret)
			ath12k_warn(ar->ab,
				    "failed to set bcast rate on vdev %i: %d\n",
				    arvif->vdev_id,  ret);
	}

	if (changed & BSS_CHANGED_BASIC_RATES &&
	    !ath12k_mac_vif_chan(arvif->vif, &def))
		ath12k_recalculate_mgmt_rate(ar, vif, &def);

	if (changed & BSS_CHANGED_TWT) {
		if (info->twt_requester || info->twt_responder)
			ath12k_wmi_send_twt_enable_cmd(ar, ar->pdev->pdev_id);
		else
			ath12k_wmi_send_twt_disable_cmd(ar, ar->pdev->pdev_id);
	}

	if (changed & BSS_CHANGED_HE_OBSS_PD)
		ath12k_wmi_send_obss_spr_cmd(ar, arvif->vdev_id,
					     &info->he_obss_pd);

	if (changed & BSS_CHANGED_HE_BSS_COLOR) {
		if (vif->type == NL80211_IFTYPE_AP) {
			ret = ath12k_wmi_obss_color_cfg_cmd(ar,
							    arvif->vdev_id,
							    info->he_bss_color.color,
							    ATH12K_BSS_COLOR_AP_PERIODS,
							    info->he_bss_color.enabled);
			if (ret)
				ath12k_warn(ar->ab, "failed to set bss color collision on vdev %i: %d\n",
					    arvif->vdev_id,  ret);
		} else if (vif->type == NL80211_IFTYPE_STATION) {
			ret = ath12k_wmi_send_bss_color_change_enable_cmd(ar,
									  arvif->vdev_id,
									  1);
			if (ret)
				ath12k_warn(ar->ab, "failed to enable bss color change on vdev %i: %d\n",
					    arvif->vdev_id,  ret);
			ret = ath12k_wmi_obss_color_cfg_cmd(ar,
							    arvif->vdev_id,
							    0,
							    ATH12K_BSS_COLOR_STA_PERIODS,
							    1);
			if (ret)
				ath12k_warn(ar->ab, "failed to set bss color collision on vdev %i: %d\n",
					    arvif->vdev_id,  ret);
		}
	}

	ath12k_mac_fils_discovery(arvif, info);

	if (changed & BSS_CHANGED_PS &&
	    ar->ab->hw_params->supports_sta_ps) {
		arvif->ps = vif_cfg->ps;
		ath12k_mac_vif_setup_ps(arvif);
	}
}

static struct ath12k_vif_cache *ath12k_arvif_get_cache(struct ath12k_vif *arvif)
{
	if (!arvif->cache)
		arvif->cache = kzalloc(sizeof(*arvif->cache), GFP_KERNEL);

	return arvif->cache;
}

static void ath12k_arvif_put_cache(struct ath12k_vif *arvif)
{
	kfree(arvif->cache);
	arvif->cache = NULL;
}

static void ath12k_mac_op_bss_info_changed(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif,
					   struct ieee80211_bss_conf *info,
					   u64 changed)
{
	struct ath12k *ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_vif_cache *cache;

	ar = ath12k_get_ar_by_vif(hw, vif);

	/* if the vdev is not created on a certain radio,
	 * cache the info to be updated later on vdev creation
	 */

	if (!ar) {
		cache = ath12k_arvif_get_cache(arvif);
		if (!cache)
			return;
		arvif->cache->bss_conf_changed |= changed;
		return;
	}

	mutex_lock(&ar->conf_mutex);

	ath12k_mac_bss_info_changed(ar, arvif, info, changed);

	mutex_unlock(&ar->conf_mutex);
}

static struct ath12k*
ath12k_mac_select_scan_device(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      u32 center_freq)
{
	struct ath12k_hw *ah = hw->priv;
	enum nl80211_band band;
	struct ath12k *ar;
	int i;

	if (ah->num_radio == 1)
		return ah->radio;

	/* Currently mac80211 supports splitting scan requests into
	 * multiple scan requests per band.
	 * Loop through first channel and determine the scan radio
	 * TODO: There could be 5 GHz low/high channels in that case
	 * split the hw request and perform multiple scans
	 */

	if (center_freq < ATH12K_MIN_5G_FREQ)
		band = NL80211_BAND_2GHZ;
	else if (center_freq < ATH12K_MIN_6G_FREQ)
		band = NL80211_BAND_5GHZ;
	else
		band = NL80211_BAND_6GHZ;

	for_each_ar(ah, ar, i) {
		/* TODO 5 GHz low high split changes */
		if (ar->mac.sbands[band].channels)
			return ar;
	}

	return NULL;
}

void __ath12k_mac_scan_finish(struct ath12k *ar)
{
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);

	lockdep_assert_held(&ar->data_lock);

	switch (ar->scan.state) {
	case ATH12K_SCAN_IDLE:
		break;
	case ATH12K_SCAN_RUNNING:
	case ATH12K_SCAN_ABORTING:
		if (ar->scan.is_roc && ar->scan.roc_notify)
			ieee80211_remain_on_channel_expired(hw);
		fallthrough;
	case ATH12K_SCAN_STARTING:
		if (!ar->scan.is_roc) {
			struct cfg80211_scan_info info = {
				.aborted = ((ar->scan.state ==
					    ATH12K_SCAN_ABORTING) ||
					    (ar->scan.state ==
					    ATH12K_SCAN_STARTING)),
			};

			ieee80211_scan_completed(hw, &info);
		}

		ar->scan.state = ATH12K_SCAN_IDLE;
		ar->scan_channel = NULL;
		ar->scan.roc_freq = 0;
		cancel_delayed_work(&ar->scan.timeout);
		complete(&ar->scan.completed);
		break;
	}
}

void ath12k_mac_scan_finish(struct ath12k *ar)
{
	spin_lock_bh(&ar->data_lock);
	__ath12k_mac_scan_finish(ar);
	spin_unlock_bh(&ar->data_lock);
}

static int ath12k_scan_stop(struct ath12k *ar)
{
	struct ath12k_wmi_scan_cancel_arg arg = {
		.req_type = WLAN_SCAN_CANCEL_SINGLE,
		.scan_id = ATH12K_SCAN_ID,
	};
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	/* TODO: Fill other STOP Params */
	arg.pdev_id = ar->pdev->pdev_id;

	ret = ath12k_wmi_send_scan_stop_cmd(ar, &arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to stop wmi scan: %d\n", ret);
		goto out;
	}

	ret = wait_for_completion_timeout(&ar->scan.completed, 3 * HZ);
	if (ret == 0) {
		ath12k_warn(ar->ab,
			    "failed to receive scan abort comple: timed out\n");
		ret = -ETIMEDOUT;
	} else if (ret > 0) {
		ret = 0;
	}

out:
	/* Scan state should be updated upon scan completion but in case
	 * firmware fails to deliver the event (for whatever reason) it is
	 * desired to clean up scan state anyway. Firmware may have just
	 * dropped the scan completion event delivery due to transport pipe
	 * being overflown with data and/or it can recover on its own before
	 * next scan request is submitted.
	 */
	spin_lock_bh(&ar->data_lock);
	if (ar->scan.state != ATH12K_SCAN_IDLE)
		__ath12k_mac_scan_finish(ar);
	spin_unlock_bh(&ar->data_lock);

	return ret;
}

static void ath12k_scan_abort(struct ath12k *ar)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);

	switch (ar->scan.state) {
	case ATH12K_SCAN_IDLE:
		/* This can happen if timeout worker kicked in and called
		 * abortion while scan completion was being processed.
		 */
		break;
	case ATH12K_SCAN_STARTING:
	case ATH12K_SCAN_ABORTING:
		ath12k_warn(ar->ab, "refusing scan abortion due to invalid scan state: %d\n",
			    ar->scan.state);
		break;
	case ATH12K_SCAN_RUNNING:
		ar->scan.state = ATH12K_SCAN_ABORTING;
		spin_unlock_bh(&ar->data_lock);

		ret = ath12k_scan_stop(ar);
		if (ret)
			ath12k_warn(ar->ab, "failed to abort scan: %d\n", ret);

		spin_lock_bh(&ar->data_lock);
		break;
	}

	spin_unlock_bh(&ar->data_lock);
}

static void ath12k_scan_timeout_work(struct work_struct *work)
{
	struct ath12k *ar = container_of(work, struct ath12k,
					 scan.timeout.work);

	mutex_lock(&ar->conf_mutex);
	ath12k_scan_abort(ar);
	mutex_unlock(&ar->conf_mutex);
}

static int ath12k_start_scan(struct ath12k *ar,
			     struct ath12k_wmi_scan_req_arg *arg)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ret = ath12k_wmi_send_scan_start_cmd(ar, arg);
	if (ret)
		return ret;

	ret = wait_for_completion_timeout(&ar->scan.started, 1 * HZ);
	if (ret == 0) {
		ret = ath12k_scan_stop(ar);
		if (ret)
			ath12k_warn(ar->ab, "failed to stop scan: %d\n", ret);

		return -ETIMEDOUT;
	}

	/* If we failed to start the scan, return error code at
	 * this point.  This is probably due to some issue in the
	 * firmware, but no need to wedge the driver due to that...
	 */
	spin_lock_bh(&ar->data_lock);
	if (ar->scan.state == ATH12K_SCAN_IDLE) {
		spin_unlock_bh(&ar->data_lock);
		return -EINVAL;
	}
	spin_unlock_bh(&ar->data_lock);

	return 0;
}

static int ath12k_mac_op_hw_scan(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_scan_request *hw_req)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar, *prev_ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct cfg80211_scan_request *req = &hw_req->req;
	struct ath12k_wmi_scan_req_arg *arg = NULL;
	int ret;
	int i;
	bool create = true;

	if (ah->num_radio == 1) {
		WARN_ON(!arvif->is_created);
		ar = ath12k_ah_to_ar(ah, 0);
		goto scan;
	}

	/* Since the targeted scan device could depend on the frequency
	 * requested in the hw_req, select the corresponding radio
	 */
	ar = ath12k_mac_select_scan_device(hw, vif, hw_req->req.channels[0]->center_freq);
	if (!ar)
		return -EINVAL;

	/* If the vif is already assigned to a specific vdev of an ar,
	 * check whether its already started, vdev which is started
	 * are not allowed to switch to a new radio.
	 * If the vdev is not started, but was earlier created on a
	 * different ar, delete that vdev and create a new one. We don't
	 * delete at the scan stop as an optimization to avoid redundant
	 * delete-create vdev's for the same ar, in case the request is
	 * always on the same band for the vif
	 */
	if (arvif->is_created) {
		if (WARN_ON(!arvif->ar))
			return -EINVAL;

		if (ar != arvif->ar && arvif->is_started)
			return -EINVAL;

		if (ar != arvif->ar) {
			/* backup the previously used ar ptr, since the vdev delete
			 * would assign the arvif->ar to NULL after the call
			 */
			prev_ar = arvif->ar;
			mutex_lock(&prev_ar->conf_mutex);
			ret = ath12k_mac_vdev_delete(prev_ar, vif);
			mutex_unlock(&prev_ar->conf_mutex);
			if (ret)
				ath12k_warn(prev_ar->ab,
					    "unable to delete scan vdev %d\n", ret);
		} else {
			create = false;
		}
	}
	if (create) {
		mutex_lock(&ar->conf_mutex);
		ret = ath12k_mac_vdev_create(ar, vif);
		mutex_unlock(&ar->conf_mutex);
		if (ret) {
			ath12k_warn(ar->ab, "unable to create scan vdev %d\n", ret);
			return -EINVAL;
		}
	}
scan:
	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	switch (ar->scan.state) {
	case ATH12K_SCAN_IDLE:
		reinit_completion(&ar->scan.started);
		reinit_completion(&ar->scan.completed);
		ar->scan.state = ATH12K_SCAN_STARTING;
		ar->scan.is_roc = false;
		ar->scan.vdev_id = arvif->vdev_id;
		ret = 0;
		break;
	case ATH12K_SCAN_STARTING:
	case ATH12K_SCAN_RUNNING:
	case ATH12K_SCAN_ABORTING:
		ret = -EBUSY;
		break;
	}
	spin_unlock_bh(&ar->data_lock);

	if (ret)
		goto exit;

	arg = kzalloc(sizeof(*arg), GFP_KERNEL);
	if (!arg) {
		ret = -ENOMEM;
		goto exit;
	}

	ath12k_wmi_start_scan_init(ar, arg);
	arg->vdev_id = arvif->vdev_id;
	arg->scan_id = ATH12K_SCAN_ID;

	if (req->ie_len) {
		arg->extraie.ptr = kmemdup(req->ie, req->ie_len, GFP_KERNEL);
		if (!arg->extraie.ptr) {
			ret = -ENOMEM;
			goto exit;
		}
		arg->extraie.len = req->ie_len;
	}

	if (req->n_ssids) {
		arg->num_ssids = req->n_ssids;
		for (i = 0; i < arg->num_ssids; i++)
			arg->ssid[i] = req->ssids[i];
	} else {
		arg->scan_f_passive = 1;
	}

	if (req->n_channels) {
		arg->num_chan = req->n_channels;
		arg->chan_list = kcalloc(arg->num_chan, sizeof(*arg->chan_list),
					 GFP_KERNEL);
		if (!arg->chan_list) {
			ret = -ENOMEM;
			goto exit;
		}

		for (i = 0; i < arg->num_chan; i++)
			arg->chan_list[i] = req->channels[i]->center_freq;
	}

	ret = ath12k_start_scan(ar, arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to start hw scan: %d\n", ret);
		spin_lock_bh(&ar->data_lock);
		ar->scan.state = ATH12K_SCAN_IDLE;
		spin_unlock_bh(&ar->data_lock);
	}

	/* Add a margin to account for event/command processing */
	ieee80211_queue_delayed_work(ath12k_ar_to_hw(ar), &ar->scan.timeout,
				     msecs_to_jiffies(arg->max_scan_time +
						      ATH12K_MAC_SCAN_TIMEOUT_MSECS));

exit:
	if (arg) {
		kfree(arg->chan_list);
		kfree(arg->extraie.ptr);
		kfree(arg);
	}

	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void ath12k_mac_op_cancel_hw_scan(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k *ar;

	if (!arvif->is_created)
		return;

	ar = arvif->ar;

	mutex_lock(&ar->conf_mutex);
	ath12k_scan_abort(ar);
	mutex_unlock(&ar->conf_mutex);

	cancel_delayed_work_sync(&ar->scan.timeout);
}

static int ath12k_install_key(struct ath12k_vif *arvif,
			      struct ieee80211_key_conf *key,
			      enum set_key_cmd cmd,
			      const u8 *macaddr, u32 flags)
{
	int ret;
	struct ath12k *ar = arvif->ar;
	struct wmi_vdev_install_key_arg arg = {
		.vdev_id = arvif->vdev_id,
		.key_idx = key->keyidx,
		.key_len = key->keylen,
		.key_data = key->key,
		.key_flags = flags,
		.macaddr = macaddr,
	};

	lockdep_assert_held(&arvif->ar->conf_mutex);

	reinit_completion(&ar->install_key_done);

	if (test_bit(ATH12K_FLAG_HW_CRYPTO_DISABLED, &ar->ab->dev_flags))
		return 0;

	if (cmd == DISABLE_KEY) {
		/* TODO: Check if FW expects  value other than NONE for del */
		/* arg.key_cipher = WMI_CIPHER_NONE; */
		arg.key_len = 0;
		arg.key_data = NULL;
		goto install;
	}

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_CCMP:
		arg.key_cipher = WMI_CIPHER_AES_CCM;
		/* TODO: Re-check if flag is valid */
		key->flags |= IEEE80211_KEY_FLAG_GENERATE_IV_MGMT;
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		arg.key_cipher = WMI_CIPHER_TKIP;
		arg.key_txmic_len = 8;
		arg.key_rxmic_len = 8;
		break;
	case WLAN_CIPHER_SUITE_CCMP_256:
		arg.key_cipher = WMI_CIPHER_AES_CCM;
		break;
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		arg.key_cipher = WMI_CIPHER_AES_GCM;
		break;
	default:
		ath12k_warn(ar->ab, "cipher %d is not supported\n", key->cipher);
		return -EOPNOTSUPP;
	}

	if (test_bit(ATH12K_FLAG_RAW_MODE, &ar->ab->dev_flags))
		key->flags |= IEEE80211_KEY_FLAG_GENERATE_IV |
			      IEEE80211_KEY_FLAG_RESERVE_TAILROOM;

install:
	ret = ath12k_wmi_vdev_install_key(arvif->ar, &arg);

	if (ret)
		return ret;

	if (!wait_for_completion_timeout(&ar->install_key_done, 1 * HZ))
		return -ETIMEDOUT;

	if (ether_addr_equal(macaddr, arvif->vif->addr))
		arvif->key_cipher = key->cipher;

	return ar->install_key_status ? -EINVAL : 0;
}

static int ath12k_clear_peer_keys(struct ath12k_vif *arvif,
				  const u8 *addr)
{
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	struct ath12k_peer *peer;
	int first_errno = 0;
	int ret;
	int i;
	u32 flags = 0;

	lockdep_assert_held(&ar->conf_mutex);

	spin_lock_bh(&ab->base_lock);
	peer = ath12k_peer_find(ab, arvif->vdev_id, addr);
	spin_unlock_bh(&ab->base_lock);

	if (!peer)
		return -ENOENT;

	for (i = 0; i < ARRAY_SIZE(peer->keys); i++) {
		if (!peer->keys[i])
			continue;

		/* key flags are not required to delete the key */
		ret = ath12k_install_key(arvif, peer->keys[i],
					 DISABLE_KEY, addr, flags);
		if (ret < 0 && first_errno == 0)
			first_errno = ret;

		if (ret < 0)
			ath12k_warn(ab, "failed to remove peer key %d: %d\n",
				    i, ret);

		spin_lock_bh(&ab->base_lock);
		peer->keys[i] = NULL;
		spin_unlock_bh(&ab->base_lock);
	}

	return first_errno;
}

static int ath12k_mac_set_key(struct ath12k *ar, enum set_key_cmd cmd,
			      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			      struct ieee80211_key_conf *key)
{
	struct ath12k_base *ab = ar->ab;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_peer *peer;
	struct ath12k_sta *arsta;
	const u8 *peer_addr;
	int ret = 0;
	u32 flags = 0;

	lockdep_assert_held(&ar->conf_mutex);

	if (test_bit(ATH12K_FLAG_HW_CRYPTO_DISABLED, &ab->dev_flags))
		return 1;

	if (sta)
		peer_addr = sta->addr;
	else if (arvif->vdev_type == WMI_VDEV_TYPE_STA)
		peer_addr = vif->bss_conf.bssid;
	else
		peer_addr = vif->addr;

	key->hw_key_idx = key->keyidx;

	/* the peer should not disappear in mid-way (unless FW goes awry) since
	 * we already hold conf_mutex. we just make sure its there now.
	 */
	spin_lock_bh(&ab->base_lock);
	peer = ath12k_peer_find(ab, arvif->vdev_id, peer_addr);
	spin_unlock_bh(&ab->base_lock);

	if (!peer) {
		if (cmd == SET_KEY) {
			ath12k_warn(ab, "cannot install key for non-existent peer %pM\n",
				    peer_addr);
			ret = -EOPNOTSUPP;
			goto exit;
		} else {
			/* if the peer doesn't exist there is no key to disable
			 * anymore
			 */
			goto exit;
		}
	}

	if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
		flags |= WMI_KEY_PAIRWISE;
	else
		flags |= WMI_KEY_GROUP;

	ret = ath12k_install_key(arvif, key, cmd, peer_addr, flags);
	if (ret) {
		ath12k_warn(ab, "ath12k_install_key failed (%d)\n", ret);
		goto exit;
	}

	ret = ath12k_dp_rx_peer_pn_replay_config(arvif, peer_addr, cmd, key);
	if (ret) {
		ath12k_warn(ab, "failed to offload PN replay detection %d\n", ret);
		goto exit;
	}

	spin_lock_bh(&ab->base_lock);
	peer = ath12k_peer_find(ab, arvif->vdev_id, peer_addr);
	if (peer && cmd == SET_KEY) {
		peer->keys[key->keyidx] = key;
		if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) {
			peer->ucast_keyidx = key->keyidx;
			peer->sec_type = ath12k_dp_tx_get_encrypt_type(key->cipher);
		} else {
			peer->mcast_keyidx = key->keyidx;
			peer->sec_type_grp = ath12k_dp_tx_get_encrypt_type(key->cipher);
		}
	} else if (peer && cmd == DISABLE_KEY) {
		peer->keys[key->keyidx] = NULL;
		if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
			peer->ucast_keyidx = 0;
		else
			peer->mcast_keyidx = 0;
	} else if (!peer)
		/* impossible unless FW goes crazy */
		ath12k_warn(ab, "peer %pM disappeared!\n", peer_addr);

	if (sta) {
		arsta = ath12k_sta_to_arsta(sta);

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_TKIP:
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_CCMP_256:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			if (cmd == SET_KEY)
				arsta->pn_type = HAL_PN_TYPE_WPA;
			else
				arsta->pn_type = HAL_PN_TYPE_NONE;
			break;
		default:
			arsta->pn_type = HAL_PN_TYPE_NONE;
			break;
		}
	}

	spin_unlock_bh(&ab->base_lock);

exit:
	return ret;
}

static int ath12k_mac_op_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
				 struct ieee80211_vif *vif, struct ieee80211_sta *sta,
				 struct ieee80211_key_conf *key)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_vif_cache *cache;
	struct ath12k *ar;
	int ret;

	/* BIP needs to be done in software */
	if (key->cipher == WLAN_CIPHER_SUITE_AES_CMAC ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_128 ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_256 ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_CMAC_256)
		return 1;

	if (key->keyidx > WMI_MAX_KEY_INDEX)
		return -ENOSPC;

	ar = ath12k_get_ar_by_vif(hw, vif);
	if (!ar) {
		/* ar is expected to be valid when sta ptr is available */
		if (sta) {
			WARN_ON_ONCE(1);
			return -EINVAL;
		}

		cache = ath12k_arvif_get_cache(arvif);
		if (!cache)
			return -ENOSPC;
		cache->key_conf.cmd = cmd;
		cache->key_conf.key = key;
		cache->key_conf.changed = true;
		return 0;
	}

	mutex_lock(&ar->conf_mutex);
	ret = ath12k_mac_set_key(ar, cmd, vif, sta, key);
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static int
ath12k_mac_bitrate_mask_num_vht_rates(struct ath12k *ar,
				      enum nl80211_band band,
				      const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++)
		num_rates += hweight16(mask->control[band].vht_mcs[i]);

	return num_rates;
}

static int
ath12k_mac_set_peer_vht_fixed_rate(struct ath12k_vif *arvif,
				   struct ieee80211_sta *sta,
				   const struct cfg80211_bitrate_mask *mask,
				   enum nl80211_band band)
{
	struct ath12k *ar = arvif->ar;
	u8 vht_rate, nss;
	u32 rate_code;
	int ret, i;

	lockdep_assert_held(&ar->conf_mutex);

	nss = 0;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++) {
		if (hweight16(mask->control[band].vht_mcs[i]) == 1) {
			nss = i + 1;
			vht_rate = ffs(mask->control[band].vht_mcs[i]) - 1;
		}
	}

	if (!nss) {
		ath12k_warn(ar->ab, "No single VHT Fixed rate found to set for %pM",
			    sta->addr);
		return -EINVAL;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "Setting Fixed VHT Rate for peer %pM. Device will not switch to any other selected rates",
		   sta->addr);

	rate_code = ATH12K_HW_RATE_CODE(vht_rate, nss - 1,
					WMI_RATE_PREAMBLE_VHT);
	ret = ath12k_wmi_set_peer_param(ar, sta->addr,
					arvif->vdev_id,
					WMI_PEER_PARAM_FIXED_RATE,
					rate_code);
	if (ret)
		ath12k_warn(ar->ab,
			    "failed to update STA %pM Fixed Rate %d: %d\n",
			     sta->addr, rate_code, ret);

	return ret;
}

static int ath12k_station_assoc(struct ath12k *ar,
				struct ieee80211_vif *vif,
				struct ieee80211_sta *sta,
				bool reassoc)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_wmi_peer_assoc_arg peer_arg;
	int ret;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	struct cfg80211_bitrate_mask *mask;
	u8 num_vht_rates;

	lockdep_assert_held(&ar->conf_mutex);

	if (WARN_ON(ath12k_mac_vif_chan(vif, &def)))
		return -EPERM;

	band = def.chan->band;
	mask = &arvif->bitrate_mask;

	ath12k_peer_assoc_prepare(ar, vif, sta, &peer_arg, reassoc);

	if (peer_arg.peer_nss < 1) {
		ath12k_warn(ar->ab,
			    "invalid peer NSS %d\n", peer_arg.peer_nss);
		return -EINVAL;
	}
	ret = ath12k_wmi_send_peer_assoc_cmd(ar, &peer_arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to run peer assoc for STA %pM vdev %i: %d\n",
			    sta->addr, arvif->vdev_id, ret);
		return ret;
	}

	if (!wait_for_completion_timeout(&ar->peer_assoc_done, 1 * HZ)) {
		ath12k_warn(ar->ab, "failed to get peer assoc conf event for %pM vdev %i\n",
			    sta->addr, arvif->vdev_id);
		return -ETIMEDOUT;
	}

	num_vht_rates = ath12k_mac_bitrate_mask_num_vht_rates(ar, band, mask);

	/* If single VHT rate is configured (by set_bitrate_mask()),
	 * peer_assoc will disable VHT. This is now enabled by a peer specific
	 * fixed param.
	 * Note that all other rates and NSS will be disabled for this peer.
	 */
	if (sta->deflink.vht_cap.vht_supported && num_vht_rates == 1) {
		ret = ath12k_mac_set_peer_vht_fixed_rate(arvif, sta, mask,
							 band);
		if (ret)
			return ret;
	}

	/* Re-assoc is run only to update supported rates for given station. It
	 * doesn't make much sense to reconfigure the peer completely.
	 */
	if (reassoc)
		return 0;

	ret = ath12k_setup_peer_smps(ar, arvif, sta->addr,
				     &sta->deflink.ht_cap,
				     &sta->deflink.he_6ghz_capa);
	if (ret) {
		ath12k_warn(ar->ab, "failed to setup peer SMPS for vdev %d: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	if (!sta->wme) {
		arvif->num_legacy_stations++;
		ret = ath12k_recalc_rtscts_prot(arvif);
		if (ret)
			return ret;
	}

	if (sta->wme && sta->uapsd_queues) {
		ret = ath12k_peer_assoc_qos_ap(ar, arvif, sta);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set qos params for STA %pM for vdev %i: %d\n",
				    sta->addr, arvif->vdev_id, ret);
			return ret;
		}
	}

	return 0;
}

static int ath12k_station_disassoc(struct ath12k *ar,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (!sta->wme) {
		arvif->num_legacy_stations--;
		ret = ath12k_recalc_rtscts_prot(arvif);
		if (ret)
			return ret;
	}

	ret = ath12k_clear_peer_keys(arvif, sta->addr);
	if (ret) {
		ath12k_warn(ar->ab, "failed to clear all peer keys for vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}
	return 0;
}

static void ath12k_sta_rc_update_wk(struct work_struct *wk)
{
	struct ath12k *ar;
	struct ath12k_vif *arvif;
	struct ath12k_sta *arsta;
	struct ieee80211_sta *sta;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	const u16 *vht_mcs_mask;
	u32 changed, bw, nss, smps, bw_prev;
	int err, num_vht_rates;
	const struct cfg80211_bitrate_mask *mask;
	struct ath12k_wmi_peer_assoc_arg peer_arg;
	enum wmi_phy_mode peer_phymode;

	arsta = container_of(wk, struct ath12k_sta, update_wk);
	sta = container_of((void *)arsta, struct ieee80211_sta, drv_priv);
	arvif = arsta->arvif;
	ar = arvif->ar;

	if (WARN_ON(ath12k_mac_vif_chan(arvif->vif, &def)))
		return;

	band = def.chan->band;
	ht_mcs_mask = arvif->bitrate_mask.control[band].ht_mcs;
	vht_mcs_mask = arvif->bitrate_mask.control[band].vht_mcs;

	spin_lock_bh(&ar->data_lock);

	changed = arsta->changed;
	arsta->changed = 0;

	bw = arsta->bw;
	bw_prev = arsta->bw_prev;
	nss = arsta->nss;
	smps = arsta->smps;

	spin_unlock_bh(&ar->data_lock);

	mutex_lock(&ar->conf_mutex);

	nss = max_t(u32, 1, nss);
	nss = min(nss, max(ath12k_mac_max_ht_nss(ht_mcs_mask),
			   ath12k_mac_max_vht_nss(vht_mcs_mask)));

	if (changed & IEEE80211_RC_BW_CHANGED) {
		ath12k_peer_assoc_h_phymode(ar, arvif->vif, sta, &peer_arg);
		peer_phymode = peer_arg.peer_phymode;

		if (bw > bw_prev) {
			/* Phymode shows maximum supported channel width, if we
			 * upgrade bandwidth then due to sanity check of firmware,
			 * we have to send WMI_PEER_PHYMODE followed by
			 * WMI_PEER_CHWIDTH
			 */
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac bandwidth upgrade for sta %pM new %d old %d\n",
				   sta->addr, bw, bw_prev);
			err = ath12k_wmi_set_peer_param(ar, sta->addr,
							arvif->vdev_id, WMI_PEER_PHYMODE,
							peer_phymode);
			if (err) {
				ath12k_warn(ar->ab, "failed to update STA %pM to peer phymode %d: %d\n",
					    sta->addr, peer_phymode, err);
				goto err_rc_bw_changed;
			}
			err = ath12k_wmi_set_peer_param(ar, sta->addr,
							arvif->vdev_id, WMI_PEER_CHWIDTH,
							bw);
			if (err)
				ath12k_warn(ar->ab, "failed to update STA %pM to peer bandwidth %d: %d\n",
					    sta->addr, bw, err);
		} else {
			/* When we downgrade bandwidth this will conflict with phymode
			 * and cause to trigger firmware crash. In this case we send
			 * WMI_PEER_CHWIDTH followed by WMI_PEER_PHYMODE
			 */
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac bandwidth downgrade for sta %pM new %d old %d\n",
				   sta->addr, bw, bw_prev);
			err = ath12k_wmi_set_peer_param(ar, sta->addr,
							arvif->vdev_id, WMI_PEER_CHWIDTH,
							bw);
			if (err) {
				ath12k_warn(ar->ab, "failed to update STA %pM peer to bandwidth %d: %d\n",
					    sta->addr, bw, err);
				goto err_rc_bw_changed;
			}
			err = ath12k_wmi_set_peer_param(ar, sta->addr,
							arvif->vdev_id, WMI_PEER_PHYMODE,
							peer_phymode);
			if (err)
				ath12k_warn(ar->ab, "failed to update STA %pM to peer phymode %d: %d\n",
					    sta->addr, peer_phymode, err);
		}
	}

	if (changed & IEEE80211_RC_NSS_CHANGED) {
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac update sta %pM nss %d\n",
			   sta->addr, nss);

		err = ath12k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
						WMI_PEER_NSS, nss);
		if (err)
			ath12k_warn(ar->ab, "failed to update STA %pM nss %d: %d\n",
				    sta->addr, nss, err);
	}

	if (changed & IEEE80211_RC_SMPS_CHANGED) {
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac update sta %pM smps %d\n",
			   sta->addr, smps);

		err = ath12k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
						WMI_PEER_MIMO_PS_STATE, smps);
		if (err)
			ath12k_warn(ar->ab, "failed to update STA %pM smps %d: %d\n",
				    sta->addr, smps, err);
	}

	if (changed & IEEE80211_RC_SUPP_RATES_CHANGED) {
		mask = &arvif->bitrate_mask;
		num_vht_rates = ath12k_mac_bitrate_mask_num_vht_rates(ar, band,
								      mask);

		/* Peer_assoc_prepare will reject vht rates in
		 * bitrate_mask if its not available in range format and
		 * sets vht tx_rateset as unsupported. So multiple VHT MCS
		 * setting(eg. MCS 4,5,6) per peer is not supported here.
		 * But, Single rate in VHT mask can be set as per-peer
		 * fixed rate. But even if any HT rates are configured in
		 * the bitrate mask, device will not switch to those rates
		 * when per-peer Fixed rate is set.
		 * TODO: Check RATEMASK_CMDID to support auto rates selection
		 * across HT/VHT and for multiple VHT MCS support.
		 */
		if (sta->deflink.vht_cap.vht_supported && num_vht_rates == 1) {
			ath12k_mac_set_peer_vht_fixed_rate(arvif, sta, mask,
							   band);
		} else {
			/* If the peer is non-VHT or no fixed VHT rate
			 * is provided in the new bitrate mask we set the
			 * other rates using peer_assoc command.
			 */
			ath12k_peer_assoc_prepare(ar, arvif->vif, sta,
						  &peer_arg, true);

			err = ath12k_wmi_send_peer_assoc_cmd(ar, &peer_arg);
			if (err)
				ath12k_warn(ar->ab, "failed to run peer assoc for STA %pM vdev %i: %d\n",
					    sta->addr, arvif->vdev_id, err);

			if (!wait_for_completion_timeout(&ar->peer_assoc_done, 1 * HZ))
				ath12k_warn(ar->ab, "failed to get peer assoc conf event for %pM vdev %i\n",
					    sta->addr, arvif->vdev_id);
		}
	}
err_rc_bw_changed:
	mutex_unlock(&ar->conf_mutex);
}

static int ath12k_mac_inc_num_stations(struct ath12k_vif *arvif,
				       struct ieee80211_sta *sta)
{
	struct ath12k *ar = arvif->ar;

	lockdep_assert_held(&ar->conf_mutex);

	if (arvif->vdev_type == WMI_VDEV_TYPE_STA && !sta->tdls)
		return 0;

	if (ar->num_stations >= ar->max_num_stations)
		return -ENOBUFS;

	ar->num_stations++;

	return 0;
}

static void ath12k_mac_dec_num_stations(struct ath12k_vif *arvif,
					struct ieee80211_sta *sta)
{
	struct ath12k *ar = arvif->ar;

	lockdep_assert_held(&ar->conf_mutex);

	if (arvif->vdev_type == WMI_VDEV_TYPE_STA && !sta->tdls)
		return;

	ar->num_stations--;
}

static int ath12k_mac_station_add(struct ath12k *ar,
				  struct ieee80211_vif *vif,
				  struct ieee80211_sta *sta)
{
	struct ath12k_base *ab = ar->ab;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);
	struct ath12k_wmi_peer_create_arg peer_param;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ret = ath12k_mac_inc_num_stations(arvif, sta);
	if (ret) {
		ath12k_warn(ab, "refusing to associate station: too many connected already (%d)\n",
			    ar->max_num_stations);
		goto exit;
	}

	arsta->rx_stats = kzalloc(sizeof(*arsta->rx_stats), GFP_KERNEL);
	if (!arsta->rx_stats) {
		ret = -ENOMEM;
		goto dec_num_station;
	}

	peer_param.vdev_id = arvif->vdev_id;
	peer_param.peer_addr = sta->addr;
	peer_param.peer_type = WMI_PEER_TYPE_DEFAULT;

	ret = ath12k_peer_create(ar, arvif, sta, &peer_param);
	if (ret) {
		ath12k_warn(ab, "Failed to add peer: %pM for VDEV: %d\n",
			    sta->addr, arvif->vdev_id);
		goto free_peer;
	}

	ath12k_dbg(ab, ATH12K_DBG_MAC, "Added peer: %pM for VDEV: %d\n",
		   sta->addr, arvif->vdev_id);

	if (ieee80211_vif_is_mesh(vif)) {
		ret = ath12k_wmi_set_peer_param(ar, sta->addr,
						arvif->vdev_id,
						WMI_PEER_USE_4ADDR, 1);
		if (ret) {
			ath12k_warn(ab, "failed to STA %pM 4addr capability: %d\n",
				    sta->addr, ret);
			goto free_peer;
		}
	}

	ret = ath12k_dp_peer_setup(ar, arvif->vdev_id, sta->addr);
	if (ret) {
		ath12k_warn(ab, "failed to setup dp for peer %pM on vdev %i (%d)\n",
			    sta->addr, arvif->vdev_id, ret);
		goto free_peer;
	}

	if (ab->hw_params->vdev_start_delay &&
	    !arvif->is_started &&
	    arvif->vdev_type != WMI_VDEV_TYPE_AP) {
		ret = ath12k_start_vdev_delay(ar, arvif);
		if (ret) {
			ath12k_warn(ab, "failed to delay vdev start: %d\n", ret);
			goto free_peer;
		}
	}

	return 0;

free_peer:
	ath12k_peer_delete(ar, arvif->vdev_id, sta->addr);
dec_num_station:
	ath12k_mac_dec_num_stations(arvif, sta);
exit:
	return ret;
}

static u32 ath12k_mac_ieee80211_sta_bw_to_wmi(struct ath12k *ar,
					      struct ieee80211_sta *sta)
{
	u32 bw = WMI_PEER_CHWIDTH_20MHZ;

	switch (sta->deflink.bandwidth) {
	case IEEE80211_STA_RX_BW_20:
		bw = WMI_PEER_CHWIDTH_20MHZ;
		break;
	case IEEE80211_STA_RX_BW_40:
		bw = WMI_PEER_CHWIDTH_40MHZ;
		break;
	case IEEE80211_STA_RX_BW_80:
		bw = WMI_PEER_CHWIDTH_80MHZ;
		break;
	case IEEE80211_STA_RX_BW_160:
		bw = WMI_PEER_CHWIDTH_160MHZ;
		break;
	case IEEE80211_STA_RX_BW_320:
		bw = WMI_PEER_CHWIDTH_320MHZ;
		break;
	default:
		ath12k_warn(ar->ab, "Invalid bandwidth %d in rc update for %pM\n",
			    sta->deflink.bandwidth, sta->addr);
		bw = WMI_PEER_CHWIDTH_20MHZ;
		break;
	}

	return bw;
}

static int ath12k_mac_op_sta_state(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   enum ieee80211_sta_state old_state,
				   enum ieee80211_sta_state new_state)
{
	struct ath12k *ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);
	struct ath12k_peer *peer;
	int ret = 0;

	/* cancel must be done outside the mutex to avoid deadlock */
	if ((old_state == IEEE80211_STA_NONE &&
	     new_state == IEEE80211_STA_NOTEXIST))
		cancel_work_sync(&arsta->update_wk);

	ar = ath12k_get_ar_by_vif(hw, vif);
	if (!ar) {
		WARN_ON_ONCE(1);
		return -EINVAL;
	}

	mutex_lock(&ar->conf_mutex);

	if (old_state == IEEE80211_STA_NOTEXIST &&
	    new_state == IEEE80211_STA_NONE) {
		memset(arsta, 0, sizeof(*arsta));
		arsta->arvif = arvif;
		INIT_WORK(&arsta->update_wk, ath12k_sta_rc_update_wk);

		ret = ath12k_mac_station_add(ar, vif, sta);
		if (ret)
			ath12k_warn(ar->ab, "Failed to add station: %pM for VDEV: %d\n",
				    sta->addr, arvif->vdev_id);
	} else if ((old_state == IEEE80211_STA_NONE &&
		    new_state == IEEE80211_STA_NOTEXIST)) {
		if (arvif->vdev_type == WMI_VDEV_TYPE_STA) {
			ath12k_bss_disassoc(ar, arvif);
			ret = ath12k_mac_vdev_stop(arvif);
			if (ret)
				ath12k_warn(ar->ab, "failed to stop vdev %i: %d\n",
					    arvif->vdev_id, ret);
		}
		ath12k_dp_peer_cleanup(ar, arvif->vdev_id, sta->addr);

		ret = ath12k_peer_delete(ar, arvif->vdev_id, sta->addr);
		if (ret)
			ath12k_warn(ar->ab, "Failed to delete peer: %pM for VDEV: %d\n",
				    sta->addr, arvif->vdev_id);
		else
			ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "Removed peer: %pM for VDEV: %d\n",
				   sta->addr, arvif->vdev_id);

		ath12k_mac_dec_num_stations(arvif, sta);
		spin_lock_bh(&ar->ab->base_lock);
		peer = ath12k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
		if (peer && peer->sta == sta) {
			ath12k_warn(ar->ab, "Found peer entry %pM n vdev %i after it was supposedly removed\n",
				    vif->addr, arvif->vdev_id);
			peer->sta = NULL;
			list_del(&peer->list);
			kfree(peer);
			ar->num_peers--;
		}
		spin_unlock_bh(&ar->ab->base_lock);

		kfree(arsta->rx_stats);
		arsta->rx_stats = NULL;
	} else if (old_state == IEEE80211_STA_AUTH &&
		   new_state == IEEE80211_STA_ASSOC &&
		   (vif->type == NL80211_IFTYPE_AP ||
		    vif->type == NL80211_IFTYPE_MESH_POINT ||
		    vif->type == NL80211_IFTYPE_ADHOC)) {
		ret = ath12k_station_assoc(ar, vif, sta, false);
		if (ret)
			ath12k_warn(ar->ab, "Failed to associate station: %pM\n",
				    sta->addr);

		spin_lock_bh(&ar->data_lock);

		arsta->bw = ath12k_mac_ieee80211_sta_bw_to_wmi(ar, sta);
		arsta->bw_prev = sta->deflink.bandwidth;

		spin_unlock_bh(&ar->data_lock);
	} else if (old_state == IEEE80211_STA_ASSOC &&
		   new_state == IEEE80211_STA_AUTHORIZED) {
		spin_lock_bh(&ar->ab->base_lock);

		peer = ath12k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
		if (peer)
			peer->is_authorized = true;

		spin_unlock_bh(&ar->ab->base_lock);

		if (vif->type == NL80211_IFTYPE_STATION && arvif->is_up) {
			ret = ath12k_wmi_set_peer_param(ar, sta->addr,
							arvif->vdev_id,
							WMI_PEER_AUTHORIZE,
							1);
			if (ret)
				ath12k_warn(ar->ab, "Unable to authorize peer %pM vdev %d: %d\n",
					    sta->addr, arvif->vdev_id, ret);
		}
	} else if (old_state == IEEE80211_STA_AUTHORIZED &&
		   new_state == IEEE80211_STA_ASSOC) {
		spin_lock_bh(&ar->ab->base_lock);

		peer = ath12k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
		if (peer)
			peer->is_authorized = false;

		spin_unlock_bh(&ar->ab->base_lock);
	} else if (old_state == IEEE80211_STA_ASSOC &&
		   new_state == IEEE80211_STA_AUTH &&
		   (vif->type == NL80211_IFTYPE_AP ||
		    vif->type == NL80211_IFTYPE_MESH_POINT ||
		    vif->type == NL80211_IFTYPE_ADHOC)) {
		ret = ath12k_station_disassoc(ar, vif, sta);
		if (ret)
			ath12k_warn(ar->ab, "Failed to disassociate station: %pM\n",
				    sta->addr);
	}

	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static int ath12k_mac_op_sta_set_txpwr(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif,
				       struct ieee80211_sta *sta)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int ret;
	s16 txpwr;

	if (sta->deflink.txpwr.type == NL80211_TX_POWER_AUTOMATIC) {
		txpwr = 0;
	} else {
		txpwr = sta->deflink.txpwr.power;
		if (!txpwr)
			return -EINVAL;
	}

	if (txpwr > ATH12K_TX_POWER_MAX_VAL || txpwr < ATH12K_TX_POWER_MIN_VAL)
		return -EINVAL;

	ar = ath12k_ah_to_ar(ah, 0);

	mutex_lock(&ar->conf_mutex);

	ret = ath12k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
					WMI_PEER_USE_FIXED_PWR, txpwr);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set tx power for station ret: %d\n",
			    ret);
		goto out;
	}

out:
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static void ath12k_mac_op_sta_rc_update(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
					struct ieee80211_sta *sta,
					u32 changed)
{
	struct ath12k *ar;
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_peer *peer;
	u32 bw, smps;

	ar = ath12k_get_ar_by_vif(hw, vif);
	if (!ar) {
		WARN_ON_ONCE(1);
		return;
	}

	spin_lock_bh(&ar->ab->base_lock);

	peer = ath12k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
	if (!peer) {
		spin_unlock_bh(&ar->ab->base_lock);
		ath12k_warn(ar->ab, "mac sta rc update failed to find peer %pM on vdev %i\n",
			    sta->addr, arvif->vdev_id);
		return;
	}

	spin_unlock_bh(&ar->ab->base_lock);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "mac sta rc update for %pM changed %08x bw %d nss %d smps %d\n",
		   sta->addr, changed, sta->deflink.bandwidth, sta->deflink.rx_nss,
		   sta->deflink.smps_mode);

	spin_lock_bh(&ar->data_lock);

	if (changed & IEEE80211_RC_BW_CHANGED) {
		bw = ath12k_mac_ieee80211_sta_bw_to_wmi(ar, sta);
		arsta->bw_prev = arsta->bw;
		arsta->bw = bw;
	}

	if (changed & IEEE80211_RC_NSS_CHANGED)
		arsta->nss = sta->deflink.rx_nss;

	if (changed & IEEE80211_RC_SMPS_CHANGED) {
		smps = WMI_PEER_SMPS_PS_NONE;

		switch (sta->deflink.smps_mode) {
		case IEEE80211_SMPS_AUTOMATIC:
		case IEEE80211_SMPS_OFF:
			smps = WMI_PEER_SMPS_PS_NONE;
			break;
		case IEEE80211_SMPS_STATIC:
			smps = WMI_PEER_SMPS_STATIC;
			break;
		case IEEE80211_SMPS_DYNAMIC:
			smps = WMI_PEER_SMPS_DYNAMIC;
			break;
		default:
			ath12k_warn(ar->ab, "Invalid smps %d in sta rc update for %pM\n",
				    sta->deflink.smps_mode, sta->addr);
			smps = WMI_PEER_SMPS_PS_NONE;
			break;
		}

		arsta->smps = smps;
	}

	arsta->changed |= changed;

	spin_unlock_bh(&ar->data_lock);

	ieee80211_queue_work(hw, &arsta->update_wk);
}

static int ath12k_conf_tx_uapsd(struct ath12k_vif *arvif,
				u16 ac, bool enable)
{
	struct ath12k *ar = arvif->ar;
	u32 value;
	int ret;

	if (arvif->vdev_type != WMI_VDEV_TYPE_STA)
		return 0;

	switch (ac) {
	case IEEE80211_AC_VO:
		value = WMI_STA_PS_UAPSD_AC3_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC3_TRIGGER_EN;
		break;
	case IEEE80211_AC_VI:
		value = WMI_STA_PS_UAPSD_AC2_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC2_TRIGGER_EN;
		break;
	case IEEE80211_AC_BE:
		value = WMI_STA_PS_UAPSD_AC1_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC1_TRIGGER_EN;
		break;
	case IEEE80211_AC_BK:
		value = WMI_STA_PS_UAPSD_AC0_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC0_TRIGGER_EN;
		break;
	}

	if (enable)
		arvif->u.sta.uapsd |= value;
	else
		arvif->u.sta.uapsd &= ~value;

	ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
					  WMI_STA_PS_PARAM_UAPSD,
					  arvif->u.sta.uapsd);
	if (ret) {
		ath12k_warn(ar->ab, "could not set uapsd params %d\n", ret);
		goto exit;
	}

	if (arvif->u.sta.uapsd)
		value = WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD;
	else
		value = WMI_STA_PS_RX_WAKE_POLICY_WAKE;

	ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
					  WMI_STA_PS_PARAM_RX_WAKE_POLICY,
					  value);
	if (ret)
		ath12k_warn(ar->ab, "could not set rx wake param %d\n", ret);

exit:
	return ret;
}

static int ath12k_mac_conf_tx(struct ath12k_vif *arvif,
			      unsigned int link_id, u16 ac,
			      const struct ieee80211_tx_queue_params *params)
{
	struct wmi_wmm_params_arg *p = NULL;
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	switch (ac) {
	case IEEE80211_AC_VO:
		p = &arvif->wmm_params.ac_vo;
		break;
	case IEEE80211_AC_VI:
		p = &arvif->wmm_params.ac_vi;
		break;
	case IEEE80211_AC_BE:
		p = &arvif->wmm_params.ac_be;
		break;
	case IEEE80211_AC_BK:
		p = &arvif->wmm_params.ac_bk;
		break;
	}

	if (WARN_ON(!p)) {
		ret = -EINVAL;
		goto exit;
	}

	p->cwmin = params->cw_min;
	p->cwmax = params->cw_max;
	p->aifs = params->aifs;
	p->txop = params->txop;

	ret = ath12k_wmi_send_wmm_update_cmd(ar, arvif->vdev_id,
					     &arvif->wmm_params);
	if (ret) {
		ath12k_warn(ab, "pdev idx %d failed to set wmm params: %d\n",
			    ar->pdev_idx, ret);
		goto exit;
	}

	ret = ath12k_conf_tx_uapsd(arvif, ac, params->uapsd);
	if (ret)
		ath12k_warn(ab, "pdev idx %d failed to set sta uapsd: %d\n",
			    ar->pdev_idx, ret);

exit:
	return ret;
}

static int ath12k_mac_op_conf_tx(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 unsigned int link_id, u16 ac,
				 const struct ieee80211_tx_queue_params *params)
{
	struct ath12k *ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_vif_cache *cache = arvif->cache;
	int ret;

	ar = ath12k_get_ar_by_vif(hw, vif);
	if (!ar) {
		/* cache the info and apply after vdev is created */
		cache = ath12k_arvif_get_cache(arvif);
		if (!cache)
			return -ENOSPC;
		cache->tx_conf.changed = true;
		cache->tx_conf.ac = ac;
		cache->tx_conf.tx_queue_params = *params;
		return 0;
	}

	mutex_lock(&ar->conf_mutex);
	ret = ath12k_mac_conf_tx(arvif, link_id, ac, params);
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static struct ieee80211_sta_ht_cap
ath12k_create_ht_cap(struct ath12k *ar, u32 ar_ht_cap, u32 rate_cap_rx_chainmask)
{
	int i;
	struct ieee80211_sta_ht_cap ht_cap = {0};
	u32 ar_vht_cap = ar->pdev->cap.vht_cap;

	if (!(ar_ht_cap & WMI_HT_CAP_ENABLED))
		return ht_cap;

	ht_cap.ht_supported = 1;
	ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
	ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_NONE;
	ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	ht_cap.cap |= IEEE80211_HT_CAP_DSSSCCK40;
	ht_cap.cap |= WLAN_HT_CAP_SM_PS_STATIC << IEEE80211_HT_CAP_SM_PS_SHIFT;

	if (ar_ht_cap & WMI_HT_CAP_HT20_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;

	if (ar_ht_cap & WMI_HT_CAP_HT40_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;

	if (ar_ht_cap & WMI_HT_CAP_DYNAMIC_SMPS) {
		u32 smps;

		smps   = WLAN_HT_CAP_SM_PS_DYNAMIC;
		smps <<= IEEE80211_HT_CAP_SM_PS_SHIFT;

		ht_cap.cap |= smps;
	}

	if (ar_ht_cap & WMI_HT_CAP_TX_STBC)
		ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

	if (ar_ht_cap & WMI_HT_CAP_RX_STBC) {
		u32 stbc;

		stbc   = ar_ht_cap;
		stbc  &= WMI_HT_CAP_RX_STBC;
		stbc >>= WMI_HT_CAP_RX_STBC_MASK_SHIFT;
		stbc <<= IEEE80211_HT_CAP_RX_STBC_SHIFT;
		stbc  &= IEEE80211_HT_CAP_RX_STBC;

		ht_cap.cap |= stbc;
	}

	if (ar_ht_cap & WMI_HT_CAP_RX_LDPC)
		ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;

	if (ar_ht_cap & WMI_HT_CAP_L_SIG_TXOP_PROT)
		ht_cap.cap |= IEEE80211_HT_CAP_LSIG_TXOP_PROT;

	if (ar_vht_cap & WMI_VHT_CAP_MAX_MPDU_LEN_MASK)
		ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;

	for (i = 0; i < ar->num_rx_chains; i++) {
		if (rate_cap_rx_chainmask & BIT(i))
			ht_cap.mcs.rx_mask[i] = 0xFF;
	}

	ht_cap.mcs.tx_params |= IEEE80211_HT_MCS_TX_DEFINED;

	return ht_cap;
}

static int ath12k_mac_set_txbf_conf(struct ath12k_vif *arvif)
{
	u32 value = 0;
	struct ath12k *ar = arvif->ar;
	int nsts;
	int sound_dim;
	u32 vht_cap = ar->pdev->cap.vht_cap;
	u32 vdev_param = WMI_VDEV_PARAM_TXBF;

	if (vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE)) {
		nsts = vht_cap & IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;
		nsts >>= IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
		value |= SM(nsts, WMI_TXBF_STS_CAP_OFFSET);
	}

	if (vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE)) {
		sound_dim = vht_cap &
			    IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
		sound_dim >>= IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
		if (sound_dim > (ar->num_tx_chains - 1))
			sound_dim = ar->num_tx_chains - 1;
		value |= SM(sound_dim, WMI_BF_SOUND_DIM_OFFSET);
	}

	if (!value)
		return 0;

	if (vht_cap & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE) {
		value |= WMI_VDEV_PARAM_TXBF_SU_TX_BFER;

		if ((vht_cap & IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE) &&
		    arvif->vdev_type == WMI_VDEV_TYPE_AP)
			value |= WMI_VDEV_PARAM_TXBF_MU_TX_BFER;
	}

	if (vht_cap & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE) {
		value |= WMI_VDEV_PARAM_TXBF_SU_TX_BFEE;

		if ((vht_cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE) &&
		    arvif->vdev_type == WMI_VDEV_TYPE_STA)
			value |= WMI_VDEV_PARAM_TXBF_MU_TX_BFEE;
	}

	return ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					     vdev_param, value);
}

static void ath12k_set_vht_txbf_cap(struct ath12k *ar, u32 *vht_cap)
{
	bool subfer, subfee;
	int sound_dim = 0;

	subfer = !!(*vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE));
	subfee = !!(*vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE));

	if (ar->num_tx_chains < 2) {
		*vht_cap &= ~(IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
		subfer = false;
	}

	/* If SU Beaformer is not set, then disable MU Beamformer Capability */
	if (!subfer)
		*vht_cap &= ~(IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE);

	/* If SU Beaformee is not set, then disable MU Beamformee Capability */
	if (!subfee)
		*vht_cap &= ~(IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	sound_dim = u32_get_bits(*vht_cap,
				 IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK);
	*vht_cap = u32_replace_bits(*vht_cap, 0,
				    IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK);

	/* TODO: Need to check invalid STS and Sound_dim values set by FW? */

	/* Enable Sounding Dimension Field only if SU BF is enabled */
	if (subfer) {
		if (sound_dim > (ar->num_tx_chains - 1))
			sound_dim = ar->num_tx_chains - 1;

		*vht_cap = u32_replace_bits(*vht_cap, sound_dim,
					    IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK);
	}

	/* Use the STS advertised by FW unless SU Beamformee is not supported*/
	if (!subfee)
		*vht_cap &= ~(IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK);
}

static struct ieee80211_sta_vht_cap
ath12k_create_vht_cap(struct ath12k *ar, u32 rate_cap_tx_chainmask,
		      u32 rate_cap_rx_chainmask)
{
	struct ieee80211_sta_vht_cap vht_cap = {0};
	u16 txmcs_map, rxmcs_map;
	int i;

	vht_cap.vht_supported = 1;
	vht_cap.cap = ar->pdev->cap.vht_cap;

	ath12k_set_vht_txbf_cap(ar, &vht_cap.cap);

	/* TODO: Enable back VHT160 mode once association issues are fixed */
	/* Disabling VHT160 and VHT80+80 modes */
	vht_cap.cap &= ~IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;
	vht_cap.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_160;

	rxmcs_map = 0;
	txmcs_map = 0;
	for (i = 0; i < 8; i++) {
		if (i < ar->num_tx_chains && rate_cap_tx_chainmask & BIT(i))
			txmcs_map |= IEEE80211_VHT_MCS_SUPPORT_0_9 << (i * 2);
		else
			txmcs_map |= IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * 2);

		if (i < ar->num_rx_chains && rate_cap_rx_chainmask & BIT(i))
			rxmcs_map |= IEEE80211_VHT_MCS_SUPPORT_0_9 << (i * 2);
		else
			rxmcs_map |= IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * 2);
	}

	if (rate_cap_tx_chainmask <= 1)
		vht_cap.cap &= ~IEEE80211_VHT_CAP_TXSTBC;

	vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(rxmcs_map);
	vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(txmcs_map);

	return vht_cap;
}

static void ath12k_mac_setup_ht_vht_cap(struct ath12k *ar,
					struct ath12k_pdev_cap *cap,
					u32 *ht_cap_info)
{
	struct ieee80211_supported_band *band;
	u32 rate_cap_tx_chainmask;
	u32 rate_cap_rx_chainmask;
	u32 ht_cap;

	rate_cap_tx_chainmask = ar->cfg_tx_chainmask >> cap->tx_chain_mask_shift;
	rate_cap_rx_chainmask = ar->cfg_rx_chainmask >> cap->rx_chain_mask_shift;

	if (cap->supported_bands & WMI_HOST_WLAN_2G_CAP) {
		band = &ar->mac.sbands[NL80211_BAND_2GHZ];
		ht_cap = cap->band[NL80211_BAND_2GHZ].ht_cap_info;
		if (ht_cap_info)
			*ht_cap_info = ht_cap;
		band->ht_cap = ath12k_create_ht_cap(ar, ht_cap,
						    rate_cap_rx_chainmask);
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP &&
	    (ar->ab->hw_params->single_pdev_only ||
	     !ar->supports_6ghz)) {
		band = &ar->mac.sbands[NL80211_BAND_5GHZ];
		ht_cap = cap->band[NL80211_BAND_5GHZ].ht_cap_info;
		if (ht_cap_info)
			*ht_cap_info = ht_cap;
		band->ht_cap = ath12k_create_ht_cap(ar, ht_cap,
						    rate_cap_rx_chainmask);
		band->vht_cap = ath12k_create_vht_cap(ar, rate_cap_tx_chainmask,
						      rate_cap_rx_chainmask);
	}
}

static int ath12k_check_chain_mask(struct ath12k *ar, u32 ant, bool is_tx_ant)
{
	/* TODO: Check the request chainmask against the supported
	 * chainmask table which is advertised in extented_service_ready event
	 */

	return 0;
}

static void ath12k_gen_ppe_thresh(struct ath12k_wmi_ppe_threshold_arg *fw_ppet,
				  u8 *he_ppet)
{
	int nss, ru;
	u8 bit = 7;

	he_ppet[0] = fw_ppet->numss_m1 & IEEE80211_PPE_THRES_NSS_MASK;
	he_ppet[0] |= (fw_ppet->ru_bit_mask <<
		       IEEE80211_PPE_THRES_RU_INDEX_BITMASK_POS) &
		      IEEE80211_PPE_THRES_RU_INDEX_BITMASK_MASK;
	for (nss = 0; nss <= fw_ppet->numss_m1; nss++) {
		for (ru = 0; ru < 4; ru++) {
			u8 val;
			int i;

			if ((fw_ppet->ru_bit_mask & BIT(ru)) == 0)
				continue;
			val = (fw_ppet->ppet16_ppet8_ru3_ru0[nss] >> (ru * 6)) &
			       0x3f;
			val = ((val >> 3) & 0x7) | ((val & 0x7) << 3);
			for (i = 5; i >= 0; i--) {
				he_ppet[bit / 8] |=
					((val >> i) & 0x1) << ((bit % 8));
				bit++;
			}
		}
	}
}

static void
ath12k_mac_filter_he_cap_mesh(struct ieee80211_he_cap_elem *he_cap_elem)
{
	u8 m;

	m = IEEE80211_HE_MAC_CAP0_TWT_RES |
	    IEEE80211_HE_MAC_CAP0_TWT_REQ;
	he_cap_elem->mac_cap_info[0] &= ~m;

	m = IEEE80211_HE_MAC_CAP2_TRS |
	    IEEE80211_HE_MAC_CAP2_BCAST_TWT |
	    IEEE80211_HE_MAC_CAP2_MU_CASCADING;
	he_cap_elem->mac_cap_info[2] &= ~m;

	m = IEEE80211_HE_MAC_CAP3_FLEX_TWT_SCHED |
	    IEEE80211_HE_MAC_CAP2_BCAST_TWT |
	    IEEE80211_HE_MAC_CAP2_MU_CASCADING;
	he_cap_elem->mac_cap_info[3] &= ~m;

	m = IEEE80211_HE_MAC_CAP4_BSRP_BQRP_A_MPDU_AGG |
	    IEEE80211_HE_MAC_CAP4_BQR;
	he_cap_elem->mac_cap_info[4] &= ~m;

	m = IEEE80211_HE_MAC_CAP5_SUBCHAN_SELECTIVE_TRANSMISSION |
	    IEEE80211_HE_MAC_CAP5_UL_2x996_TONE_RU |
	    IEEE80211_HE_MAC_CAP5_PUNCTURED_SOUNDING |
	    IEEE80211_HE_MAC_CAP5_HT_VHT_TRIG_FRAME_RX;
	he_cap_elem->mac_cap_info[5] &= ~m;

	m = IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
	    IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO;
	he_cap_elem->phy_cap_info[2] &= ~m;

	m = IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU |
	    IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_MASK |
	    IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_MASK;
	he_cap_elem->phy_cap_info[3] &= ~m;

	m = IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER;
	he_cap_elem->phy_cap_info[4] &= ~m;

	m = IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
	he_cap_elem->phy_cap_info[5] &= ~m;

	m = IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
	    IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
	    IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB |
	    IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO;
	he_cap_elem->phy_cap_info[6] &= ~m;

	m = IEEE80211_HE_PHY_CAP7_PSR_BASED_SR |
	    IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP |
	    IEEE80211_HE_PHY_CAP7_STBC_TX_ABOVE_80MHZ |
	    IEEE80211_HE_PHY_CAP7_STBC_RX_ABOVE_80MHZ;
	he_cap_elem->phy_cap_info[7] &= ~m;

	m = IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
	    IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G |
	    IEEE80211_HE_PHY_CAP8_20MHZ_IN_160MHZ_HE_PPDU |
	    IEEE80211_HE_PHY_CAP8_80MHZ_IN_160MHZ_HE_PPDU;
	he_cap_elem->phy_cap_info[8] &= ~m;

	m = IEEE80211_HE_PHY_CAP9_LONGER_THAN_16_SIGB_OFDM_SYM |
	    IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
	    IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU |
	    IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU |
	    IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
	    IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;
	he_cap_elem->phy_cap_info[9] &= ~m;
}

static __le16 ath12k_mac_setup_he_6ghz_cap(struct ath12k_pdev_cap *pcap,
					   struct ath12k_band_cap *bcap)
{
	u8 val;

	bcap->he_6ghz_capa = IEEE80211_HT_MPDU_DENSITY_NONE;
	if (bcap->ht_cap_info & WMI_HT_CAP_DYNAMIC_SMPS)
		bcap->he_6ghz_capa |=
			u32_encode_bits(WLAN_HT_CAP_SM_PS_DYNAMIC,
					IEEE80211_HE_6GHZ_CAP_SM_PS);
	else
		bcap->he_6ghz_capa |=
			u32_encode_bits(WLAN_HT_CAP_SM_PS_DISABLED,
					IEEE80211_HE_6GHZ_CAP_SM_PS);
	val = u32_get_bits(pcap->vht_cap,
			   IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK);
	bcap->he_6ghz_capa |=
		u32_encode_bits(val, IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP);
	val = u32_get_bits(pcap->vht_cap,
			   IEEE80211_VHT_CAP_MAX_MPDU_MASK);
	bcap->he_6ghz_capa |=
		u32_encode_bits(val, IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN);
	if (pcap->vht_cap & IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN)
		bcap->he_6ghz_capa |= IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS;
	if (pcap->vht_cap & IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN)
		bcap->he_6ghz_capa |= IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS;

	return cpu_to_le16(bcap->he_6ghz_capa);
}

static void ath12k_mac_copy_he_cap(struct ath12k_band_cap *band_cap,
				   int iftype, u8 num_tx_chains,
				   struct ieee80211_sta_he_cap *he_cap)
{
	struct ieee80211_he_cap_elem *he_cap_elem = &he_cap->he_cap_elem;
	struct ieee80211_he_mcs_nss_supp *mcs_nss = &he_cap->he_mcs_nss_supp;

	he_cap->has_he = true;
	memcpy(he_cap_elem->mac_cap_info, band_cap->he_cap_info,
	       sizeof(he_cap_elem->mac_cap_info));
	memcpy(he_cap_elem->phy_cap_info, band_cap->he_cap_phy_info,
	       sizeof(he_cap_elem->phy_cap_info));

	he_cap_elem->mac_cap_info[1] &=
		IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_MASK;

	he_cap_elem->phy_cap_info[5] &=
		~IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_MASK;
	he_cap_elem->phy_cap_info[5] &=
		~IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_MASK;
	he_cap_elem->phy_cap_info[5] |= num_tx_chains - 1;

	switch (iftype) {
	case NL80211_IFTYPE_AP:
		he_cap_elem->phy_cap_info[3] &=
			~IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_MASK;
		he_cap_elem->phy_cap_info[9] |=
			IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU;
		break;
	case NL80211_IFTYPE_STATION:
		he_cap_elem->mac_cap_info[0] &= ~IEEE80211_HE_MAC_CAP0_TWT_RES;
		he_cap_elem->mac_cap_info[0] |= IEEE80211_HE_MAC_CAP0_TWT_REQ;
		he_cap_elem->phy_cap_info[9] |=
			IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		ath12k_mac_filter_he_cap_mesh(he_cap_elem);
		break;
	}

	mcs_nss->rx_mcs_80 = cpu_to_le16(band_cap->he_mcs & 0xffff);
	mcs_nss->tx_mcs_80 = cpu_to_le16(band_cap->he_mcs & 0xffff);
	mcs_nss->rx_mcs_160 = cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
	mcs_nss->tx_mcs_160 = cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
	mcs_nss->rx_mcs_80p80 = cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
	mcs_nss->tx_mcs_80p80 = cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);

	memset(he_cap->ppe_thres, 0, sizeof(he_cap->ppe_thres));
	if (he_cap_elem->phy_cap_info[6] &
	    IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT)
		ath12k_gen_ppe_thresh(&band_cap->he_ppet, he_cap->ppe_thres);
}

static void
ath12k_mac_copy_eht_mcs_nss(struct ath12k_band_cap *band_cap,
			    struct ieee80211_eht_mcs_nss_supp *mcs_nss,
			    const struct ieee80211_he_cap_elem *he_cap,
			    const struct ieee80211_eht_cap_elem_fixed *eht_cap)
{
	if ((he_cap->phy_cap_info[0] &
	     (IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G |
	      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G |
	      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G |
	      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)) == 0)
		memcpy(&mcs_nss->only_20mhz, &band_cap->eht_mcs_20_only,
		       sizeof(struct ieee80211_eht_mcs_nss_supp_20mhz_only));

	if (he_cap->phy_cap_info[0] &
	    (IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G |
	     IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G))
		memcpy(&mcs_nss->bw._80, &band_cap->eht_mcs_80,
		       sizeof(struct ieee80211_eht_mcs_nss_supp_bw));

	if (he_cap->phy_cap_info[0] &
	    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G)
		memcpy(&mcs_nss->bw._160, &band_cap->eht_mcs_160,
		       sizeof(struct ieee80211_eht_mcs_nss_supp_bw));

	if (eht_cap->phy_cap_info[0] & IEEE80211_EHT_PHY_CAP0_320MHZ_IN_6GHZ)
		memcpy(&mcs_nss->bw._320, &band_cap->eht_mcs_320,
		       sizeof(struct ieee80211_eht_mcs_nss_supp_bw));
}

static void ath12k_mac_copy_eht_ppe_thresh(struct ath12k_wmi_ppe_threshold_arg *fw_ppet,
					   struct ieee80211_sta_eht_cap *cap)
{
	u16 bit = IEEE80211_EHT_PPE_THRES_INFO_HEADER_SIZE;
	u8 i, nss, ru, ppet_bit_len_per_ru = IEEE80211_EHT_PPE_THRES_INFO_PPET_SIZE * 2;

	u8p_replace_bits(&cap->eht_ppe_thres[0], fw_ppet->numss_m1,
			 IEEE80211_EHT_PPE_THRES_NSS_MASK);

	u16p_replace_bits((u16 *)&cap->eht_ppe_thres[0], fw_ppet->ru_bit_mask,
			  IEEE80211_EHT_PPE_THRES_RU_INDEX_BITMASK_MASK);

	for (nss = 0; nss <= fw_ppet->numss_m1; nss++) {
		for (ru = 0;
		     ru < hweight16(IEEE80211_EHT_PPE_THRES_RU_INDEX_BITMASK_MASK);
		     ru++) {
			u32 val = 0;

			if ((fw_ppet->ru_bit_mask & BIT(ru)) == 0)
				continue;

			u32p_replace_bits(&val, fw_ppet->ppet16_ppet8_ru3_ru0[nss] >>
						(ru * ppet_bit_len_per_ru),
					  GENMASK(ppet_bit_len_per_ru - 1, 0));

			for (i = 0; i < ppet_bit_len_per_ru; i++) {
				cap->eht_ppe_thres[bit / 8] |=
					(((val >> i) & 0x1) << ((bit % 8)));
				bit++;
			}
		}
	}
}

static void
ath12k_mac_filter_eht_cap_mesh(struct ieee80211_eht_cap_elem_fixed
			       *eht_cap_elem)
{
	u8 m;

	m = IEEE80211_EHT_MAC_CAP0_EPCS_PRIO_ACCESS;
	eht_cap_elem->mac_cap_info[0] &= ~m;

	m = IEEE80211_EHT_PHY_CAP0_PARTIAL_BW_UL_MU_MIMO;
	eht_cap_elem->phy_cap_info[0] &= ~m;

	m = IEEE80211_EHT_PHY_CAP3_NG_16_MU_FEEDBACK |
	    IEEE80211_EHT_PHY_CAP3_CODEBOOK_7_5_MU_FDBK |
	    IEEE80211_EHT_PHY_CAP3_TRIG_MU_BF_PART_BW_FDBK |
	    IEEE80211_EHT_PHY_CAP3_TRIG_CQI_FDBK;
	eht_cap_elem->phy_cap_info[3] &= ~m;

	m = IEEE80211_EHT_PHY_CAP4_PART_BW_DL_MU_MIMO |
	    IEEE80211_EHT_PHY_CAP4_PSR_SR_SUPP |
	    IEEE80211_EHT_PHY_CAP4_POWER_BOOST_FACT_SUPP |
	    IEEE80211_EHT_PHY_CAP4_EHT_MU_PPDU_4_EHT_LTF_08_GI;
	eht_cap_elem->phy_cap_info[4] &= ~m;

	m = IEEE80211_EHT_PHY_CAP5_NON_TRIG_CQI_FEEDBACK |
	    IEEE80211_EHT_PHY_CAP5_TX_LESS_242_TONE_RU_SUPP |
	    IEEE80211_EHT_PHY_CAP5_RX_LESS_242_TONE_RU_SUPP |
	    IEEE80211_EHT_PHY_CAP5_MAX_NUM_SUPP_EHT_LTF_MASK;
	eht_cap_elem->phy_cap_info[5] &= ~m;

	m = IEEE80211_EHT_PHY_CAP6_MAX_NUM_SUPP_EHT_LTF_MASK;
	eht_cap_elem->phy_cap_info[6] &= ~m;

	m = IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_80MHZ |
	    IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_160MHZ |
	    IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_320MHZ |
	    IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_80MHZ |
	    IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_160MHZ |
	    IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_320MHZ;
	eht_cap_elem->phy_cap_info[7] &= ~m;
}

static void ath12k_mac_copy_eht_cap(struct ath12k *ar,
				    struct ath12k_band_cap *band_cap,
				    struct ieee80211_he_cap_elem *he_cap_elem,
				    int iftype,
				    struct ieee80211_sta_eht_cap *eht_cap)
{
	struct ieee80211_eht_cap_elem_fixed *eht_cap_elem = &eht_cap->eht_cap_elem;

	memset(eht_cap, 0, sizeof(struct ieee80211_sta_eht_cap));

	if (!(test_bit(WMI_TLV_SERVICE_11BE, ar->ab->wmi_ab.svc_map)))
		return;

	eht_cap->has_eht = true;
	memcpy(eht_cap_elem->mac_cap_info, band_cap->eht_cap_mac_info,
	       sizeof(eht_cap_elem->mac_cap_info));
	memcpy(eht_cap_elem->phy_cap_info, band_cap->eht_cap_phy_info,
	       sizeof(eht_cap_elem->phy_cap_info));

	switch (iftype) {
	case NL80211_IFTYPE_AP:
		eht_cap_elem->phy_cap_info[0] &=
			~IEEE80211_EHT_PHY_CAP0_242_TONE_RU_GT20MHZ;
		eht_cap_elem->phy_cap_info[4] &=
			~IEEE80211_EHT_PHY_CAP4_PART_BW_DL_MU_MIMO;
		eht_cap_elem->phy_cap_info[5] &=
			~IEEE80211_EHT_PHY_CAP5_TX_LESS_242_TONE_RU_SUPP;
		break;
	case NL80211_IFTYPE_STATION:
		eht_cap_elem->phy_cap_info[7] &=
			~(IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_80MHZ |
			  IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_160MHZ |
			  IEEE80211_EHT_PHY_CAP7_NON_OFDMA_UL_MU_MIMO_320MHZ);
		eht_cap_elem->phy_cap_info[7] &=
			~(IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_80MHZ |
			  IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_160MHZ |
			  IEEE80211_EHT_PHY_CAP7_MU_BEAMFORMER_320MHZ);
		break;
	case NL80211_IFTYPE_MESH_POINT:
		ath12k_mac_filter_eht_cap_mesh(eht_cap_elem);
		break;
	default:
		break;
	}

	ath12k_mac_copy_eht_mcs_nss(band_cap, &eht_cap->eht_mcs_nss_supp,
				    he_cap_elem, eht_cap_elem);

	if (eht_cap_elem->phy_cap_info[5] &
	    IEEE80211_EHT_PHY_CAP5_PPE_THRESHOLD_PRESENT)
		ath12k_mac_copy_eht_ppe_thresh(&band_cap->eht_ppet, eht_cap);
}

static int ath12k_mac_copy_sband_iftype_data(struct ath12k *ar,
					     struct ath12k_pdev_cap *cap,
					     struct ieee80211_sband_iftype_data *data,
					     int band)
{
	struct ath12k_band_cap *band_cap = &cap->band[band];
	int i, idx = 0;

	for (i = 0; i < NUM_NL80211_IFTYPES; i++) {
		struct ieee80211_sta_he_cap *he_cap = &data[idx].he_cap;

		switch (i) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_MESH_POINT:
			break;

		default:
			continue;
		}

		data[idx].types_mask = BIT(i);

		ath12k_mac_copy_he_cap(band_cap, i, ar->num_tx_chains, he_cap);
		if (band == NL80211_BAND_6GHZ) {
			data[idx].he_6ghz_capa.capa =
				ath12k_mac_setup_he_6ghz_cap(cap, band_cap);
		}
		ath12k_mac_copy_eht_cap(ar, band_cap, &he_cap->he_cap_elem, i,
					&data[idx].eht_cap);
		idx++;
	}

	return idx;
}

static void ath12k_mac_setup_sband_iftype_data(struct ath12k *ar,
					       struct ath12k_pdev_cap *cap)
{
	struct ieee80211_supported_band *sband;
	enum nl80211_band band;
	int count;

	if (cap->supported_bands & WMI_HOST_WLAN_2G_CAP) {
		band = NL80211_BAND_2GHZ;
		count = ath12k_mac_copy_sband_iftype_data(ar, cap,
							  ar->mac.iftype[band],
							  band);
		sband = &ar->mac.sbands[band];
		_ieee80211_set_sband_iftype_data(sband, ar->mac.iftype[band],
						 count);
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP) {
		band = NL80211_BAND_5GHZ;
		count = ath12k_mac_copy_sband_iftype_data(ar, cap,
							  ar->mac.iftype[band],
							  band);
		sband = &ar->mac.sbands[band];
		_ieee80211_set_sband_iftype_data(sband, ar->mac.iftype[band],
						 count);
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP &&
	    ar->supports_6ghz) {
		band = NL80211_BAND_6GHZ;
		count = ath12k_mac_copy_sband_iftype_data(ar, cap,
							  ar->mac.iftype[band],
							  band);
		sband = &ar->mac.sbands[band];
		_ieee80211_set_sband_iftype_data(sband, ar->mac.iftype[band],
						 count);
	}
}

static int __ath12k_set_antenna(struct ath12k *ar, u32 tx_ant, u32 rx_ant)
{
	struct ath12k_hw *ah = ath12k_ar_to_ah(ar);
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (ath12k_check_chain_mask(ar, tx_ant, true))
		return -EINVAL;

	if (ath12k_check_chain_mask(ar, rx_ant, false))
		return -EINVAL;

	/* Since we advertised the max cap of all radios combined during wiphy
	 * registration, ensure we don't set the antenna config higher than the
	 * limits
	 */
	tx_ant = min_t(u32, tx_ant, ar->pdev->cap.tx_chain_mask);
	rx_ant = min_t(u32, rx_ant, ar->pdev->cap.rx_chain_mask);

	ar->cfg_tx_chainmask = tx_ant;
	ar->cfg_rx_chainmask = rx_ant;

	if (ah->state != ATH12K_HW_STATE_ON &&
	    ah->state != ATH12K_HW_STATE_RESTARTED)
		return 0;

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_TX_CHAIN_MASK,
					tx_ant, ar->pdev->pdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set tx-chainmask: %d, req 0x%x\n",
			    ret, tx_ant);
		return ret;
	}

	ar->num_tx_chains = hweight32(tx_ant);

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_RX_CHAIN_MASK,
					rx_ant, ar->pdev->pdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set rx-chainmask: %d, req 0x%x\n",
			    ret, rx_ant);
		return ret;
	}

	ar->num_rx_chains = hweight32(rx_ant);

	/* Reload HT/VHT/HE capability */
	ath12k_mac_setup_ht_vht_cap(ar, &ar->pdev->cap, NULL);
	ath12k_mac_setup_sband_iftype_data(ar, &ar->pdev->cap);

	return 0;
}

static void ath12k_mgmt_over_wmi_tx_drop(struct ath12k *ar, struct sk_buff *skb)
{
	int num_mgmt;

	ieee80211_free_txskb(ath12k_ar_to_hw(ar), skb);

	num_mgmt = atomic_dec_if_positive(&ar->num_pending_mgmt_tx);

	if (num_mgmt < 0)
		WARN_ON_ONCE(1);

	if (!num_mgmt)
		wake_up(&ar->txmgmt_empty_waitq);
}

int ath12k_mac_tx_mgmt_pending_free(int buf_id, void *skb, void *ctx)
{
	struct sk_buff *msdu = skb;
	struct ieee80211_tx_info *info;
	struct ath12k *ar = ctx;
	struct ath12k_base *ab = ar->ab;

	spin_lock_bh(&ar->txmgmt_idr_lock);
	idr_remove(&ar->txmgmt_idr, buf_id);
	spin_unlock_bh(&ar->txmgmt_idr_lock);
	dma_unmap_single(ab->dev, ATH12K_SKB_CB(msdu)->paddr, msdu->len,
			 DMA_TO_DEVICE);

	info = IEEE80211_SKB_CB(msdu);
	memset(&info->status, 0, sizeof(info->status));

	ath12k_mgmt_over_wmi_tx_drop(ar, skb);

	return 0;
}

static int ath12k_mac_vif_txmgmt_idr_remove(int buf_id, void *skb, void *ctx)
{
	struct ieee80211_vif *vif = ctx;
	struct ath12k_skb_cb *skb_cb = ATH12K_SKB_CB(skb);
	struct sk_buff *msdu = skb;
	struct ath12k *ar = skb_cb->ar;
	struct ath12k_base *ab = ar->ab;

	if (skb_cb->vif == vif) {
		spin_lock_bh(&ar->txmgmt_idr_lock);
		idr_remove(&ar->txmgmt_idr, buf_id);
		spin_unlock_bh(&ar->txmgmt_idr_lock);
		dma_unmap_single(ab->dev, skb_cb->paddr, msdu->len,
				 DMA_TO_DEVICE);
	}

	return 0;
}

static int ath12k_mac_mgmt_tx_wmi(struct ath12k *ar, struct ath12k_vif *arvif,
				  struct sk_buff *skb)
{
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *info;
	dma_addr_t paddr;
	int buf_id;
	int ret;

	ATH12K_SKB_CB(skb)->ar = ar;
	spin_lock_bh(&ar->txmgmt_idr_lock);
	buf_id = idr_alloc(&ar->txmgmt_idr, skb, 0,
			   ATH12K_TX_MGMT_NUM_PENDING_MAX, GFP_ATOMIC);
	spin_unlock_bh(&ar->txmgmt_idr_lock);
	if (buf_id < 0)
		return -ENOSPC;

	info = IEEE80211_SKB_CB(skb);
	if (!(info->flags & IEEE80211_TX_CTL_HW_80211_ENCAP)) {
		if ((ieee80211_is_action(hdr->frame_control) ||
		     ieee80211_is_deauth(hdr->frame_control) ||
		     ieee80211_is_disassoc(hdr->frame_control)) &&
		     ieee80211_has_protected(hdr->frame_control)) {
			skb_put(skb, IEEE80211_CCMP_MIC_LEN);
		}
	}

	paddr = dma_map_single(ab->dev, skb->data, skb->len, DMA_TO_DEVICE);
	if (dma_mapping_error(ab->dev, paddr)) {
		ath12k_warn(ab, "failed to DMA map mgmt Tx buffer\n");
		ret = -EIO;
		goto err_free_idr;
	}

	ATH12K_SKB_CB(skb)->paddr = paddr;

	ret = ath12k_wmi_mgmt_send(ar, arvif->vdev_id, buf_id, skb);
	if (ret) {
		ath12k_warn(ar->ab, "failed to send mgmt frame: %d\n", ret);
		goto err_unmap_buf;
	}

	return 0;

err_unmap_buf:
	dma_unmap_single(ab->dev, ATH12K_SKB_CB(skb)->paddr,
			 skb->len, DMA_TO_DEVICE);
err_free_idr:
	spin_lock_bh(&ar->txmgmt_idr_lock);
	idr_remove(&ar->txmgmt_idr, buf_id);
	spin_unlock_bh(&ar->txmgmt_idr_lock);

	return ret;
}

static void ath12k_mgmt_over_wmi_tx_purge(struct ath12k *ar)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&ar->wmi_mgmt_tx_queue)) != NULL)
		ath12k_mgmt_over_wmi_tx_drop(ar, skb);
}

static void ath12k_mgmt_over_wmi_tx_work(struct work_struct *work)
{
	struct ath12k *ar = container_of(work, struct ath12k, wmi_mgmt_tx_work);
	struct ath12k_skb_cb *skb_cb;
	struct ath12k_vif *arvif;
	struct sk_buff *skb;
	int ret;

	while ((skb = skb_dequeue(&ar->wmi_mgmt_tx_queue)) != NULL) {
		skb_cb = ATH12K_SKB_CB(skb);
		if (!skb_cb->vif) {
			ath12k_warn(ar->ab, "no vif found for mgmt frame\n");
			ath12k_mgmt_over_wmi_tx_drop(ar, skb);
			continue;
		}

		arvif = ath12k_vif_to_arvif(skb_cb->vif);

		if (ar->allocated_vdev_map & (1LL << arvif->vdev_id)) {
			ret = ath12k_mac_mgmt_tx_wmi(ar, arvif, skb);
			if (ret) {
				ath12k_warn(ar->ab, "failed to tx mgmt frame, vdev_id %d :%d\n",
					    arvif->vdev_id, ret);
				ath12k_mgmt_over_wmi_tx_drop(ar, skb);
			}
		} else {
			ath12k_warn(ar->ab,
				    "dropping mgmt frame for vdev %d, is_started %d\n",
				    arvif->vdev_id,
				    arvif->is_started);
			ath12k_mgmt_over_wmi_tx_drop(ar, skb);
		}
	}
}

static int ath12k_mac_mgmt_tx(struct ath12k *ar, struct sk_buff *skb,
			      bool is_prb_rsp)
{
	struct sk_buff_head *q = &ar->wmi_mgmt_tx_queue;

	if (test_bit(ATH12K_FLAG_CRASH_FLUSH, &ar->ab->dev_flags))
		return -ESHUTDOWN;

	/* Drop probe response packets when the pending management tx
	 * count has reached a certain threshold, so as to prioritize
	 * other mgmt packets like auth and assoc to be sent on time
	 * for establishing successful connections.
	 */
	if (is_prb_rsp &&
	    atomic_read(&ar->num_pending_mgmt_tx) > ATH12K_PRB_RSP_DROP_THRESHOLD) {
		ath12k_warn(ar->ab,
			    "dropping probe response as pending queue is almost full\n");
		return -ENOSPC;
	}

	if (skb_queue_len_lockless(q) >= ATH12K_TX_MGMT_NUM_PENDING_MAX) {
		ath12k_warn(ar->ab, "mgmt tx queue is full\n");
		return -ENOSPC;
	}

	skb_queue_tail(q, skb);
	atomic_inc(&ar->num_pending_mgmt_tx);
	ieee80211_queue_work(ath12k_ar_to_hw(ar), &ar->wmi_mgmt_tx_work);

	return 0;
}

static void ath12k_mac_add_p2p_noa_ie(struct ath12k *ar,
				      struct ieee80211_vif *vif,
				      struct sk_buff *skb,
				      bool is_prb_rsp)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);

	if (likely(!is_prb_rsp))
		return;

	spin_lock_bh(&ar->data_lock);

	if (arvif->u.ap.noa_data &&
	    !pskb_expand_head(skb, 0, arvif->u.ap.noa_len,
			      GFP_ATOMIC))
		skb_put_data(skb, arvif->u.ap.noa_data,
			     arvif->u.ap.noa_len);

	spin_unlock_bh(&ar->data_lock);
}

static void ath12k_mac_op_tx(struct ieee80211_hw *hw,
			     struct ieee80211_tx_control *control,
			     struct sk_buff *skb)
{
	struct ath12k_skb_cb *skb_cb = ATH12K_SKB_CB(skb);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_vif *vif = info->control.vif;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k *ar = arvif->ar;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_key_conf *key = info->control.hw_key;
	u32 info_flags = info->flags;
	bool is_prb_rsp;
	int ret;

	memset(skb_cb, 0, sizeof(*skb_cb));
	skb_cb->vif = vif;

	if (key) {
		skb_cb->cipher = key->cipher;
		skb_cb->flags |= ATH12K_SKB_CIPHER_SET;
	}

	is_prb_rsp = ieee80211_is_probe_resp(hdr->frame_control);

	if (info_flags & IEEE80211_TX_CTL_HW_80211_ENCAP) {
		skb_cb->flags |= ATH12K_SKB_HW_80211_ENCAP;
	} else if (ieee80211_is_mgmt(hdr->frame_control)) {
		ret = ath12k_mac_mgmt_tx(ar, skb, is_prb_rsp);
		if (ret) {
			ath12k_warn(ar->ab, "failed to queue management frame %d\n",
				    ret);
			ieee80211_free_txskb(hw, skb);
		}
		return;
	}

	/* This is case only for P2P_GO */
	if (vif->type == NL80211_IFTYPE_AP && vif->p2p)
		ath12k_mac_add_p2p_noa_ie(ar, vif, skb, is_prb_rsp);

	ret = ath12k_dp_tx(ar, arvif, skb);
	if (ret) {
		ath12k_warn(ar->ab, "failed to transmit frame %d\n", ret);
		ieee80211_free_txskb(hw, skb);
	}
}

void ath12k_mac_drain_tx(struct ath12k *ar)
{
	/* make sure rcu-protected mac80211 tx path itself is drained */
	synchronize_net();

	cancel_work_sync(&ar->wmi_mgmt_tx_work);
	ath12k_mgmt_over_wmi_tx_purge(ar);
}

static int ath12k_mac_config_mon_status_default(struct ath12k *ar, bool enable)
{
	return -EOPNOTSUPP;
	/* TODO: Need to support new monitor mode */
}

static int ath12k_mac_start(struct ath12k *ar)
{
	struct ath12k_hw *ah = ar->ah;
	struct ath12k_base *ab = ar->ab;
	struct ath12k_pdev *pdev = ar->pdev;
	int ret;

	lockdep_assert_held(&ah->hw_mutex);

	mutex_lock(&ar->conf_mutex);

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_PMF_QOS,
					1, pdev->pdev_id);

	if (ret) {
		ath12k_err(ab, "failed to enable PMF QOS: (%d\n", ret);
		goto err;
	}

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_DYNAMIC_BW, 1,
					pdev->pdev_id);
	if (ret) {
		ath12k_err(ab, "failed to enable dynamic bw: %d\n", ret);
		goto err;
	}

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_ARP_AC_OVERRIDE,
					0, pdev->pdev_id);
	if (ret) {
		ath12k_err(ab, "failed to set ac override for ARP: %d\n",
			   ret);
		goto err;
	}

	ret = ath12k_wmi_send_dfs_phyerr_offload_enable_cmd(ar, pdev->pdev_id);
	if (ret) {
		ath12k_err(ab, "failed to offload radar detection: %d\n",
			   ret);
		goto err;
	}

	ret = ath12k_dp_tx_htt_h2t_ppdu_stats_req(ar,
						  HTT_PPDU_STATS_TAG_DEFAULT);
	if (ret) {
		ath12k_err(ab, "failed to req ppdu stats: %d\n", ret);
		goto err;
	}

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_MESH_MCAST_ENABLE,
					1, pdev->pdev_id);

	if (ret) {
		ath12k_err(ab, "failed to enable MESH MCAST ENABLE: (%d\n", ret);
		goto err;
	}

	__ath12k_set_antenna(ar, ar->cfg_tx_chainmask, ar->cfg_rx_chainmask);

	/* TODO: Do we need to enable ANI? */

	ath12k_reg_update_chan_list(ar);

	ar->num_started_vdevs = 0;
	ar->num_created_vdevs = 0;
	ar->num_peers = 0;
	ar->allocated_vdev_map = 0;

	/* Configure monitor status ring with default rx_filter to get rx status
	 * such as rssi, rx_duration.
	 */
	ret = ath12k_mac_config_mon_status_default(ar, true);
	if (ret && (ret != -EOPNOTSUPP)) {
		ath12k_err(ab, "failed to configure monitor status ring with default rx_filter: (%d)\n",
			   ret);
		goto err;
	}

	if (ret == -EOPNOTSUPP)
		ath12k_dbg(ab, ATH12K_DBG_MAC,
			   "monitor status config is not yet supported");

	/* Configure the hash seed for hash based reo dest ring selection */
	ath12k_wmi_pdev_lro_cfg(ar, ar->pdev->pdev_id);

	/* allow device to enter IMPS */
	if (ab->hw_params->idle_ps) {
		ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_IDLE_PS_CONFIG,
						1, pdev->pdev_id);
		if (ret) {
			ath12k_err(ab, "failed to enable idle ps: %d\n", ret);
			goto err;
		}
	}

	mutex_unlock(&ar->conf_mutex);

	rcu_assign_pointer(ab->pdevs_active[ar->pdev_idx],
			   &ab->pdevs[ar->pdev_idx]);

	return 0;
err:
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void ath12k_drain_tx(struct ath12k_hw *ah)
{
	struct ath12k *ar;
	int i;

	for_each_ar(ah, ar, i)
		ath12k_mac_drain_tx(ar);
}

static int ath12k_mac_op_start(struct ieee80211_hw *hw)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int ret, i;

	ath12k_drain_tx(ah);

	guard(mutex)(&ah->hw_mutex);

	switch (ah->state) {
	case ATH12K_HW_STATE_OFF:
		ah->state = ATH12K_HW_STATE_ON;
		break;
	case ATH12K_HW_STATE_RESTARTING:
		ah->state = ATH12K_HW_STATE_RESTARTED;
		break;
	case ATH12K_HW_STATE_RESTARTED:
	case ATH12K_HW_STATE_WEDGED:
	case ATH12K_HW_STATE_ON:
		ah->state = ATH12K_HW_STATE_OFF;

		WARN_ON(1);
		return -EINVAL;
	}

	for_each_ar(ah, ar, i) {
		ret = ath12k_mac_start(ar);
		if (ret) {
			ah->state = ATH12K_HW_STATE_OFF;

			ath12k_err(ar->ab, "fail to start mac operations in pdev idx %d ret %d\n",
				   ar->pdev_idx, ret);
			goto fail_start;
		}
	}

	return 0;

fail_start:
	for (; i > 0; i--) {
		ar = ath12k_ah_to_ar(ah, i - 1);
		ath12k_mac_stop(ar);
	}

	return ret;
}

int ath12k_mac_rfkill_config(struct ath12k *ar)
{
	struct ath12k_base *ab = ar->ab;
	u32 param;
	int ret;

	if (ab->hw_params->rfkill_pin == 0)
		return -EOPNOTSUPP;

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac rfkill_pin %d rfkill_cfg %d rfkill_on_level %d",
		   ab->hw_params->rfkill_pin, ab->hw_params->rfkill_cfg,
		   ab->hw_params->rfkill_on_level);

	param = u32_encode_bits(ab->hw_params->rfkill_on_level,
				WMI_RFKILL_CFG_RADIO_LEVEL) |
		u32_encode_bits(ab->hw_params->rfkill_pin,
				WMI_RFKILL_CFG_GPIO_PIN_NUM) |
		u32_encode_bits(ab->hw_params->rfkill_cfg,
				WMI_RFKILL_CFG_PIN_AS_GPIO);

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_HW_RFKILL_CONFIG,
					param, ar->pdev->pdev_id);
	if (ret) {
		ath12k_warn(ab,
			    "failed to set rfkill config 0x%x: %d\n",
			    param, ret);
		return ret;
	}

	return 0;
}

int ath12k_mac_rfkill_enable_radio(struct ath12k *ar, bool enable)
{
	enum wmi_rfkill_enable_radio param;
	int ret;

	if (enable)
		param = WMI_RFKILL_ENABLE_RADIO_ON;
	else
		param = WMI_RFKILL_ENABLE_RADIO_OFF;

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac %d rfkill enable %d",
		   ar->pdev_idx, param);

	ret = ath12k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_RFKILL_ENABLE,
					param, ar->pdev->pdev_id);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set rfkill enable param %d: %d\n",
			    param, ret);
		return ret;
	}

	return 0;
}

static void ath12k_mac_stop(struct ath12k *ar)
{
	struct ath12k_hw *ah = ar->ah;
	struct htt_ppdu_stats_info *ppdu_stats, *tmp;
	int ret;

	lockdep_assert_held(&ah->hw_mutex);

	mutex_lock(&ar->conf_mutex);
	ret = ath12k_mac_config_mon_status_default(ar, false);
	if (ret && (ret != -EOPNOTSUPP))
		ath12k_err(ar->ab, "failed to clear rx_filter for monitor status ring: (%d)\n",
			   ret);

	clear_bit(ATH12K_CAC_RUNNING, &ar->dev_flags);
	mutex_unlock(&ar->conf_mutex);

	cancel_delayed_work_sync(&ar->scan.timeout);
	cancel_work_sync(&ar->regd_update_work);
	cancel_work_sync(&ar->ab->rfkill_work);

	spin_lock_bh(&ar->data_lock);
	list_for_each_entry_safe(ppdu_stats, tmp, &ar->ppdu_stats_info, list) {
		list_del(&ppdu_stats->list);
		kfree(ppdu_stats);
	}
	spin_unlock_bh(&ar->data_lock);

	rcu_assign_pointer(ar->ab->pdevs_active[ar->pdev_idx], NULL);

	synchronize_rcu();

	atomic_set(&ar->num_pending_mgmt_tx, 0);
}

static void ath12k_mac_op_stop(struct ieee80211_hw *hw, bool suspend)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int i;

	ath12k_drain_tx(ah);

	mutex_lock(&ah->hw_mutex);

	ah->state = ATH12K_HW_STATE_OFF;

	for_each_ar(ah, ar, i)
		ath12k_mac_stop(ar);

	mutex_unlock(&ah->hw_mutex);
}

static u8
ath12k_mac_get_vdev_stats_id(struct ath12k_vif *arvif)
{
	struct ath12k_base *ab = arvif->ar->ab;
	u8 vdev_stats_id = 0;

	do {
		if (ab->free_vdev_stats_id_map & (1LL << vdev_stats_id)) {
			vdev_stats_id++;
			if (vdev_stats_id >= ATH12K_MAX_VDEV_STATS_ID) {
				vdev_stats_id = ATH12K_INVAL_VDEV_STATS_ID;
				break;
			}
		} else {
			ab->free_vdev_stats_id_map |= (1LL << vdev_stats_id);
			break;
		}
	} while (vdev_stats_id);

	arvif->vdev_stats_id = vdev_stats_id;
	return vdev_stats_id;
}

static int ath12k_mac_setup_vdev_params_mbssid(struct ath12k_vif *arvif,
					       u32 *flags, u32 *tx_vdev_id)
{
	struct ieee80211_vif *tx_vif = arvif->vif->mbssid_tx_vif;
	struct ath12k *ar = arvif->ar;
	struct ath12k_vif *tx_arvif;

	if (!tx_vif)
		return 0;

	tx_arvif = ath12k_vif_to_arvif(tx_vif);

	if (arvif->vif->bss_conf.nontransmitted) {
		if (ar->ah->hw->wiphy != ieee80211_vif_to_wdev(tx_vif)->wiphy)
			return -EINVAL;

		*flags = WMI_VDEV_MBSSID_FLAGS_NON_TRANSMIT_AP;
		*tx_vdev_id = tx_arvif->vdev_id;
	} else if (tx_arvif == arvif) {
		*flags = WMI_VDEV_MBSSID_FLAGS_TRANSMIT_AP;
	} else {
		return -EINVAL;
	}

	if (arvif->vif->bss_conf.ema_ap)
		*flags |= WMI_VDEV_MBSSID_FLAGS_EMA_MODE;

	return 0;
}

static int ath12k_mac_setup_vdev_create_arg(struct ath12k_vif *arvif,
					    struct ath12k_wmi_vdev_create_arg *arg)
{
	struct ath12k *ar = arvif->ar;
	struct ath12k_pdev *pdev = ar->pdev;
	int ret;

	arg->if_id = arvif->vdev_id;
	arg->type = arvif->vdev_type;
	arg->subtype = arvif->vdev_subtype;
	arg->pdev_id = pdev->pdev_id;

	arg->mbssid_flags = WMI_VDEV_MBSSID_FLAGS_NON_MBSSID_AP;
	arg->mbssid_tx_vdev_id = 0;
	if (!test_bit(WMI_TLV_SERVICE_MBSS_PARAM_IN_VDEV_START_SUPPORT,
		      ar->ab->wmi_ab.svc_map)) {
		ret = ath12k_mac_setup_vdev_params_mbssid(arvif,
							  &arg->mbssid_flags,
							  &arg->mbssid_tx_vdev_id);
		if (ret)
			return ret;
	}

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP) {
		arg->chains[NL80211_BAND_2GHZ].tx = ar->num_tx_chains;
		arg->chains[NL80211_BAND_2GHZ].rx = ar->num_rx_chains;
	}
	if (pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP) {
		arg->chains[NL80211_BAND_5GHZ].tx = ar->num_tx_chains;
		arg->chains[NL80211_BAND_5GHZ].rx = ar->num_rx_chains;
	}
	if (pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP &&
	    ar->supports_6ghz) {
		arg->chains[NL80211_BAND_6GHZ].tx = ar->num_tx_chains;
		arg->chains[NL80211_BAND_6GHZ].rx = ar->num_rx_chains;
	}

	arg->if_stats_id = ath12k_mac_get_vdev_stats_id(arvif);
	return 0;
}

static u32
ath12k_mac_prepare_he_mode(struct ath12k_pdev *pdev, u32 viftype)
{
	struct ath12k_pdev_cap *pdev_cap = &pdev->cap;
	struct ath12k_band_cap *cap_band = NULL;
	u32 *hecap_phy_ptr = NULL;
	u32 hemode;

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP)
		cap_band = &pdev_cap->band[NL80211_BAND_2GHZ];
	else
		cap_band = &pdev_cap->band[NL80211_BAND_5GHZ];

	hecap_phy_ptr = &cap_band->he_cap_phy_info[0];

	hemode = u32_encode_bits(HE_SU_BFEE_ENABLE, HE_MODE_SU_TX_BFEE) |
		 u32_encode_bits(HECAP_PHY_SUBFMR_GET(hecap_phy_ptr),
				 HE_MODE_SU_TX_BFER) |
		 u32_encode_bits(HECAP_PHY_ULMUMIMO_GET(hecap_phy_ptr),
				 HE_MODE_UL_MUMIMO);

	/* TODO: WDS and other modes */
	if (viftype == NL80211_IFTYPE_AP) {
		hemode |= u32_encode_bits(HECAP_PHY_MUBFMR_GET(hecap_phy_ptr),
					  HE_MODE_MU_TX_BFER) |
			  u32_encode_bits(HE_DL_MUOFDMA_ENABLE, HE_MODE_DL_OFDMA) |
			  u32_encode_bits(HE_UL_MUOFDMA_ENABLE, HE_MODE_UL_OFDMA);
	} else {
		hemode |= u32_encode_bits(HE_MU_BFEE_ENABLE, HE_MODE_MU_TX_BFEE);
	}

	return hemode;
}

static int ath12k_set_he_mu_sounding_mode(struct ath12k *ar,
					  struct ath12k_vif *arvif)
{
	u32 param_id, param_value;
	struct ath12k_base *ab = ar->ab;
	int ret;

	param_id = WMI_VDEV_PARAM_SET_HEMU_MODE;
	param_value = ath12k_mac_prepare_he_mode(ar->pdev, arvif->vif->type);
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath12k_warn(ab, "failed to set vdev %d HE MU mode: %d param_value %x\n",
			    arvif->vdev_id, ret, param_value);
		return ret;
	}
	param_id = WMI_VDEV_PARAM_SET_HE_SOUNDING_MODE;
	param_value =
		u32_encode_bits(HE_VHT_SOUNDING_MODE_ENABLE, HE_VHT_SOUNDING_MODE) |
		u32_encode_bits(HE_TRIG_NONTRIG_SOUNDING_MODE_ENABLE,
				HE_TRIG_NONTRIG_SOUNDING_MODE);
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath12k_warn(ab, "failed to set vdev %d HE MU mode: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}
	return ret;
}

static void ath12k_mac_update_vif_offload(struct ath12k_vif *arvif)
{
	struct ieee80211_vif *vif = arvif->vif;
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	u32 param_id, param_value;
	int ret;

	param_id = WMI_VDEV_PARAM_TX_ENCAP_TYPE;
	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		vif->offload_flags &= ~(IEEE80211_OFFLOAD_ENCAP_ENABLED |
					IEEE80211_OFFLOAD_DECAP_ENABLED);

	if (vif->offload_flags & IEEE80211_OFFLOAD_ENCAP_ENABLED)
		arvif->tx_encap_type = ATH12K_HW_TXRX_ETHERNET;
	else if (test_bit(ATH12K_FLAG_RAW_MODE, &ab->dev_flags))
		arvif->tx_encap_type = ATH12K_HW_TXRX_RAW;
	else
		arvif->tx_encap_type = ATH12K_HW_TXRX_NATIVE_WIFI;

	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, arvif->tx_encap_type);
	if (ret) {
		ath12k_warn(ab, "failed to set vdev %d tx encap mode: %d\n",
			    arvif->vdev_id, ret);
		vif->offload_flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;
	}

	param_id = WMI_VDEV_PARAM_RX_DECAP_TYPE;
	if (vif->offload_flags & IEEE80211_OFFLOAD_DECAP_ENABLED)
		param_value = ATH12K_HW_TXRX_ETHERNET;
	else if (test_bit(ATH12K_FLAG_RAW_MODE, &ab->dev_flags))
		param_value = ATH12K_HW_TXRX_RAW;
	else
		param_value = ATH12K_HW_TXRX_NATIVE_WIFI;

	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath12k_warn(ab, "failed to set vdev %d rx decap mode: %d\n",
			    arvif->vdev_id, ret);
		vif->offload_flags &= ~IEEE80211_OFFLOAD_DECAP_ENABLED;
	}
}

static void ath12k_mac_op_update_vif_offload(struct ieee80211_hw *hw,
					     struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);

	ath12k_mac_update_vif_offload(arvif);
}

static int ath12k_mac_vdev_create(struct ath12k *ar, struct ieee80211_vif *vif)
{
	struct ath12k_hw *ah = ar->ah;
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_hw *hw = ah->hw;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_wmi_vdev_create_arg vdev_arg = {0};
	struct ath12k_wmi_peer_create_arg peer_param;
	u32 param_id, param_value;
	u16 nss;
	int i;
	int ret, vdev_id;

	lockdep_assert_held(&ar->conf_mutex);

	arvif->ar = ar;
	vdev_id = __ffs64(ab->free_vdev_map);
	arvif->vdev_id = vdev_id;
	arvif->vdev_subtype = WMI_VDEV_SUBTYPE_NONE;

	switch (vif->type) {
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_STATION:
		arvif->vdev_type = WMI_VDEV_TYPE_STA;

		if (vif->p2p)
			arvif->vdev_subtype = WMI_VDEV_SUBTYPE_P2P_CLIENT;

		break;
	case NL80211_IFTYPE_MESH_POINT:
		arvif->vdev_subtype = WMI_VDEV_SUBTYPE_MESH_11S;
		fallthrough;
	case NL80211_IFTYPE_AP:
		arvif->vdev_type = WMI_VDEV_TYPE_AP;

		if (vif->p2p)
			arvif->vdev_subtype = WMI_VDEV_SUBTYPE_P2P_GO;

		break;
	case NL80211_IFTYPE_MONITOR:
		arvif->vdev_type = WMI_VDEV_TYPE_MONITOR;
		ar->monitor_vdev_id = vdev_id;
		break;
	case NL80211_IFTYPE_P2P_DEVICE:
		arvif->vdev_type = WMI_VDEV_TYPE_STA;
		arvif->vdev_subtype = WMI_VDEV_SUBTYPE_P2P_DEVICE;
		break;
	default:
		WARN_ON(1);
		break;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac vdev create id %d type %d subtype %d map %llx\n",
		   arvif->vdev_id, arvif->vdev_type, arvif->vdev_subtype,
		   ab->free_vdev_map);

	vif->cab_queue = arvif->vdev_id % (ATH12K_HW_MAX_QUEUES - 1);
	for (i = 0; i < ARRAY_SIZE(vif->hw_queue); i++)
		vif->hw_queue[i] = i % (ATH12K_HW_MAX_QUEUES - 1);

	ret = ath12k_mac_setup_vdev_create_arg(arvif, &vdev_arg);
	if (ret) {
		ath12k_warn(ab, "failed to create vdev parameters %d: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	ret = ath12k_wmi_vdev_create(ar, vif->addr, &vdev_arg);
	if (ret) {
		ath12k_warn(ab, "failed to create WMI vdev %d: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	ar->num_created_vdevs++;
	arvif->is_created = true;
	ath12k_dbg(ab, ATH12K_DBG_MAC, "vdev %pM created, vdev_id %d\n",
		   vif->addr, arvif->vdev_id);
	ar->allocated_vdev_map |= 1LL << arvif->vdev_id;
	ab->free_vdev_map &= ~(1LL << arvif->vdev_id);

	spin_lock_bh(&ar->data_lock);
	list_add(&arvif->list, &ar->arvifs);
	spin_unlock_bh(&ar->data_lock);

	ath12k_mac_update_vif_offload(arvif);

	nss = hweight32(ar->cfg_tx_chainmask) ? : 1;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    WMI_VDEV_PARAM_NSS, nss);
	if (ret) {
		ath12k_warn(ab, "failed to set vdev %d chainmask 0x%x, nss %d :%d\n",
			    arvif->vdev_id, ar->cfg_tx_chainmask, nss, ret);
		goto err_vdev_del;
	}

	switch (arvif->vdev_type) {
	case WMI_VDEV_TYPE_AP:
		peer_param.vdev_id = arvif->vdev_id;
		peer_param.peer_addr = vif->addr;
		peer_param.peer_type = WMI_PEER_TYPE_DEFAULT;
		ret = ath12k_peer_create(ar, arvif, NULL, &peer_param);
		if (ret) {
			ath12k_warn(ab, "failed to vdev %d create peer for AP: %d\n",
				    arvif->vdev_id, ret);
			goto err_vdev_del;
		}

		ret = ath12k_mac_set_kickout(arvif);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set vdev %i kickout parameters: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}
		break;
	case WMI_VDEV_TYPE_STA:
		param_id = WMI_STA_PS_PARAM_RX_WAKE_POLICY;
		param_value = WMI_STA_PS_RX_WAKE_POLICY_WAKE;
		ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set vdev %d RX wake policy: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		param_id = WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD;
		param_value = WMI_STA_PS_TX_WAKE_THRESHOLD_ALWAYS;
		ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set vdev %d TX wake threshold: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		param_id = WMI_STA_PS_PARAM_PSPOLL_COUNT;
		param_value = WMI_STA_PS_PSPOLL_COUNT_NO_MAX;
		ret = ath12k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set vdev %d pspoll count: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		ret = ath12k_wmi_pdev_set_ps_mode(ar, arvif->vdev_id, false);
		if (ret) {
			ath12k_warn(ar->ab, "failed to disable vdev %d ps mode: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}
		break;
	default:
		break;
	}

	arvif->txpower = vif->bss_conf.txpower;
	ret = ath12k_mac_txpower_recalc(ar);
	if (ret)
		goto err_peer_del;

	param_id = WMI_VDEV_PARAM_RTS_THRESHOLD;
	param_value = hw->wiphy->rts_threshold;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set rts threshold for vdev %d: %d\n",
			    arvif->vdev_id, ret);
	}

	ath12k_dp_vdev_tx_attach(ar, arvif);

	if (vif->type != NL80211_IFTYPE_MONITOR && ar->monitor_conf_enabled)
		ath12k_mac_monitor_vdev_create(ar);

	arvif->ar = ar;
	return ret;

err_peer_del:
	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		reinit_completion(&ar->peer_delete_done);

		ret = ath12k_wmi_send_peer_delete_cmd(ar, vif->addr,
						      arvif->vdev_id);
		if (ret) {
			ath12k_warn(ar->ab, "failed to delete peer vdev_id %d addr %pM\n",
				    arvif->vdev_id, vif->addr);
			goto err;
		}

		ret = ath12k_wait_for_peer_delete_done(ar, arvif->vdev_id,
						       vif->addr);
		if (ret)
			goto err;

		ar->num_peers--;
	}

err_vdev_del:
	ath12k_wmi_vdev_delete(ar, arvif->vdev_id);
	ar->num_created_vdevs--;
	arvif->is_created = false;
	arvif->ar = NULL;
	ar->allocated_vdev_map &= ~(1LL << arvif->vdev_id);
	ab->free_vdev_map |= 1LL << arvif->vdev_id;
	ab->free_vdev_stats_id_map &= ~(1LL << arvif->vdev_stats_id);
	spin_lock_bh(&ar->data_lock);
	list_del(&arvif->list);
	spin_unlock_bh(&ar->data_lock);

err:
	arvif->ar = NULL;
	return ret;
}

static void ath12k_mac_vif_cache_flush(struct ath12k *ar,  struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_vif_cache *cache = arvif->cache;
	struct ath12k_base *ab = ar->ab;

	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (!cache)
		return;

	if (cache->tx_conf.changed) {
		ret = ath12k_mac_conf_tx(arvif, 0, cache->tx_conf.ac,
					 &cache->tx_conf.tx_queue_params);
		if (ret)
			ath12k_warn(ab,
				    "unable to apply tx config parameters to vdev %d\n",
				    ret);
	}

	if (cache->bss_conf_changed) {
		ath12k_mac_bss_info_changed(ar, arvif, &vif->bss_conf,
					    cache->bss_conf_changed);
	}

	if (cache->key_conf.changed) {
		ret = ath12k_mac_set_key(ar, cache->key_conf.cmd, vif, NULL,
					 cache->key_conf.key);
		if (ret)
			ath12k_warn(ab, "unable to apply set key param to vdev %d ret %d\n",
				    arvif->vdev_id, ret);
	}
	ath12k_arvif_put_cache(arvif);
}

static struct ath12k *ath12k_mac_assign_vif_to_vdev(struct ieee80211_hw *hw,
						    struct ieee80211_vif *vif,
						    struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_hw *ah = hw->priv;
	struct ath12k *ar, *prev_ar;
	struct ath12k_base *ab;
	int ret;

	if (ah->num_radio == 1)
		ar = ah->radio;
	else if (ctx)
		ar = ath12k_get_ar_by_ctx(hw, ctx);
	else
		return NULL;

	if (!ar)
		return NULL;

	if (arvif->ar) {
		/* This is not expected really */
		if (WARN_ON(!arvif->is_created)) {
			arvif->ar = NULL;
			return NULL;
		}

		if (ah->num_radio == 1)
			return arvif->ar;

		/* This can happen as scan vdev gets created during multiple scans
		 * across different radios before a vdev is brought up in
		 * a certain radio.
		 */
		if (ar != arvif->ar) {
			if (WARN_ON(arvif->is_started))
				return NULL;

			/* backup the previously used ar ptr since arvif->ar would
			 * be set to NULL after vdev delete is done
			 */
			prev_ar = arvif->ar;
			mutex_lock(&prev_ar->conf_mutex);
			ret = ath12k_mac_vdev_delete(prev_ar, vif);

			if (ret)
				ath12k_warn(prev_ar->ab, "unable to delete vdev %d\n",
					    ret);
			mutex_unlock(&prev_ar->conf_mutex);
		}
	}

	ab = ar->ab;

	mutex_lock(&ar->conf_mutex);

	if (arvif->is_created)
		goto flush;

	if (vif->type == NL80211_IFTYPE_AP &&
	    ar->num_peers > (ar->max_num_peers - 1)) {
		ath12k_warn(ab, "failed to create vdev due to insufficient peer entry resource in firmware\n");
		goto unlock;
	}

	if (ar->num_created_vdevs > (TARGET_NUM_VDEVS - 1)) {
		ath12k_warn(ab, "failed to create vdev, reached max vdev limit %d\n",
			    TARGET_NUM_VDEVS);
		goto unlock;
	}

	ret = ath12k_mac_vdev_create(ar, vif);
	if (ret) {
		ath12k_warn(ab, "failed to create vdev %pM ret %d", vif->addr, ret);
		goto unlock;
	}

flush:
	/* If the vdev is created during channel assign and not during
	 * add_interface(), Apply any parameters for the vdev which were received
	 * after add_interface, corresponding to this vif.
	 */
	ath12k_mac_vif_cache_flush(ar, vif);
unlock:
	mutex_unlock(&ar->conf_mutex);
	return arvif->ar;
}

static int ath12k_mac_op_add_interface(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int i;

	memset(arvif, 0, sizeof(*arvif));

	arvif->vif = vif;

	INIT_LIST_HEAD(&arvif->list);
	INIT_DELAYED_WORK(&arvif->connection_loss_work,
			  ath12k_mac_vif_sta_connection_loss_work);

	for (i = 0; i < ARRAY_SIZE(arvif->bitrate_mask.control); i++) {
		arvif->bitrate_mask.control[i].legacy = 0xffffffff;
		memset(arvif->bitrate_mask.control[i].ht_mcs, 0xff,
		       sizeof(arvif->bitrate_mask.control[i].ht_mcs));
		memset(arvif->bitrate_mask.control[i].vht_mcs, 0xff,
		       sizeof(arvif->bitrate_mask.control[i].vht_mcs));
	}

	/* Allocate Default Queue now and reassign during actual vdev create */
	vif->cab_queue = ATH12K_HW_DEFAULT_QUEUE;
	for (i = 0; i < ARRAY_SIZE(vif->hw_queue); i++)
		vif->hw_queue[i] = ATH12K_HW_DEFAULT_QUEUE;

	vif->driver_flags |= IEEE80211_VIF_SUPPORTS_UAPSD;

	/* For single radio wiphy(i.e ah->num_radio is 1), create the vdev
	 * during add_interface itself, for multi radio wiphy, defer the vdev
	 * creation until channel_assign to determine the radio on which the
	 * vdev needs to be created
	 */
	ath12k_mac_assign_vif_to_vdev(hw, vif, NULL);
	return 0;
}

static void ath12k_mac_vif_unref(struct ath12k_dp *dp, struct ieee80211_vif *vif)
{
	struct ath12k_tx_desc_info *tx_desc_info;
	struct ath12k_skb_cb *skb_cb;
	struct sk_buff *skb;
	int i;

	for (i = 0; i < ATH12K_HW_MAX_QUEUES; i++) {
		spin_lock_bh(&dp->tx_desc_lock[i]);

		list_for_each_entry(tx_desc_info, &dp->tx_desc_used_list[i],
				    list) {
			skb = tx_desc_info->skb;
			if (!skb)
				continue;

			skb_cb = ATH12K_SKB_CB(skb);
			if (skb_cb->vif == vif)
				skb_cb->vif = NULL;
		}

		spin_unlock_bh(&dp->tx_desc_lock[i]);
	}
}

static int ath12k_mac_vdev_delete(struct ath12k *ar, struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_base *ab = ar->ab;
	unsigned long time_left;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);
	reinit_completion(&ar->vdev_delete_done);

	ret = ath12k_wmi_vdev_delete(ar, arvif->vdev_id);
	if (ret) {
		ath12k_warn(ab, "failed to delete WMI vdev %d: %d\n",
			    arvif->vdev_id, ret);
		goto err_vdev_del;
	}

	time_left = wait_for_completion_timeout(&ar->vdev_delete_done,
						ATH12K_VDEV_DELETE_TIMEOUT_HZ);
	if (time_left == 0) {
		ath12k_warn(ab, "Timeout in receiving vdev delete response\n");
		goto err_vdev_del;
	}

	ab->free_vdev_map |= 1LL << arvif->vdev_id;
	ar->allocated_vdev_map &= ~(1LL << arvif->vdev_id);
	ar->num_created_vdevs--;

	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ar->monitor_vdev_id = -1;
		ar->monitor_vdev_created = false;
	} else if (ar->monitor_vdev_created && !ar->monitor_started) {
		ret = ath12k_mac_monitor_vdev_delete(ar);
	}

	ath12k_dbg(ab, ATH12K_DBG_MAC, "vdev %pM deleted, vdev_id %d\n",
		   vif->addr, arvif->vdev_id);

err_vdev_del:
	spin_lock_bh(&ar->data_lock);
	list_del(&arvif->list);
	spin_unlock_bh(&ar->data_lock);

	ath12k_peer_cleanup(ar, arvif->vdev_id);
	ath12k_arvif_put_cache(arvif);

	idr_for_each(&ar->txmgmt_idr,
		     ath12k_mac_vif_txmgmt_idr_remove, vif);

	ath12k_mac_vif_unref(&ab->dp, vif);
	ath12k_dp_tx_put_bank_profile(&ab->dp, arvif->bank_id);

	/* Recalc txpower for remaining vdev */
	ath12k_mac_txpower_recalc(ar);

	/* TODO: recal traffic pause state based on the available vdevs */
	arvif->is_created = false;
	arvif->ar = NULL;

	return ret;
}

static void ath12k_mac_op_remove_interface(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_base *ab;
	struct ath12k *ar;
	int ret;

	if (!arvif->is_created) {
		/* if we cached some config but never received assign chanctx,
		 * free the allocated cache.
		 */
		ath12k_arvif_put_cache(arvif);
		return;
	}

	ar = arvif->ar;
	ab = ar->ab;

	cancel_delayed_work_sync(&arvif->connection_loss_work);

	mutex_lock(&ar->conf_mutex);

	ath12k_dbg(ab, ATH12K_DBG_MAC, "mac remove interface (vdev %d)\n",
		   arvif->vdev_id);

	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		ret = ath12k_peer_delete(ar, arvif->vdev_id, vif->addr);
		if (ret)
			ath12k_warn(ab, "failed to submit AP self-peer removal on vdev %d: %d\n",
				    arvif->vdev_id, ret);
	}

	ath12k_mac_vdev_delete(ar, vif);

	mutex_unlock(&ar->conf_mutex);
}

/* FIXME: Has to be verified. */
#define SUPPORTED_FILTERS			\
	(FIF_ALLMULTI |				\
	FIF_CONTROL |				\
	FIF_PSPOLL |				\
	FIF_OTHER_BSS |				\
	FIF_BCN_PRBRESP_PROMISC |		\
	FIF_PROBE_REQ |				\
	FIF_FCSFAIL)

static void ath12k_mac_configure_filter(struct ath12k *ar,
					unsigned int total_flags)
{
	bool reset_flag;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ar->filter_flags = total_flags;

	/* For monitor mode */
	reset_flag = !(ar->filter_flags & FIF_BCN_PRBRESP_PROMISC);

	ret = ath12k_dp_tx_htt_monitor_mode_ring_config(ar, reset_flag);
	if (ret)
		ath12k_warn(ar->ab,
			    "fail to set monitor filter: %d\n", ret);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "total_flags:0x%x, reset_flag:%d\n",
		   total_flags, reset_flag);
}

static void ath12k_mac_op_configure_filter(struct ieee80211_hw *hw,
					   unsigned int changed_flags,
					   unsigned int *total_flags,
					   u64 multicast)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;

	ar = ath12k_ah_to_ar(ah, 0);

	mutex_lock(&ar->conf_mutex);

	*total_flags &= SUPPORTED_FILTERS;
	ath12k_mac_configure_filter(ar, *total_flags);

	mutex_unlock(&ar->conf_mutex);
}

static int ath12k_mac_op_get_antenna(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	int antennas_rx = 0, antennas_tx = 0;
	struct ath12k *ar;
	int i;

	for_each_ar(ah, ar, i) {
		mutex_lock(&ar->conf_mutex);
		antennas_rx = max_t(u32, antennas_rx, ar->cfg_rx_chainmask);
		antennas_tx = max_t(u32, antennas_tx, ar->cfg_tx_chainmask);
		mutex_unlock(&ar->conf_mutex);
	}

	*tx_ant = antennas_tx;
	*rx_ant = antennas_rx;

	return 0;
}

static int ath12k_mac_op_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int ret = 0;
	int i;

	for_each_ar(ah, ar, i) {
		mutex_lock(&ar->conf_mutex);
		ret = __ath12k_set_antenna(ar, tx_ant, rx_ant);
		mutex_unlock(&ar->conf_mutex);
		if (ret)
			break;
	}

	return ret;
}

static int ath12k_mac_ampdu_action(struct ath12k_vif *arvif,
				   struct ieee80211_ampdu_params *params)
{
	struct ath12k *ar = arvif->ar;
	int ret = -EINVAL;

	lockdep_assert_held(&ar->conf_mutex);

	switch (params->action) {
	case IEEE80211_AMPDU_RX_START:
		ret = ath12k_dp_rx_ampdu_start(ar, params);
		break;
	case IEEE80211_AMPDU_RX_STOP:
		ret = ath12k_dp_rx_ampdu_stop(ar, params);
		break;
	case IEEE80211_AMPDU_TX_START:
	case IEEE80211_AMPDU_TX_STOP_CONT:
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		/* Tx A-MPDU aggregation offloaded to hw/fw so deny mac80211
		 * Tx aggregation requests.
		 */
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

static int ath12k_mac_op_ampdu_action(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_ampdu_params *params)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int ret = -EINVAL;

	ar = ath12k_get_ar_by_vif(hw, vif);
	if (!ar)
		return -EINVAL;

	ar = ath12k_ah_to_ar(ah, 0);

	mutex_lock(&ar->conf_mutex);
	ret = ath12k_mac_ampdu_action(arvif, params);
	mutex_unlock(&ar->conf_mutex);

	if (ret)
		ath12k_warn(ar->ab, "pdev idx %d unable to perform ampdu action %d ret %d\n",
			    ar->pdev_idx, params->action, ret);

	return ret;
}

static int ath12k_mac_op_add_chanctx(struct ieee80211_hw *hw,
				     struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k *ar;
	struct ath12k_base *ab;

	ar = ath12k_get_ar_by_ctx(hw, ctx);
	if (!ar)
		return -EINVAL;

	ab = ar->ab;

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac chanctx add freq %u width %d ptr %p\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx);

	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	/* TODO: In case of multiple channel context, populate rx_channel from
	 * Rx PPDU desc information.
	 */
	ar->rx_channel = ctx->def.chan;
	spin_unlock_bh(&ar->data_lock);

	mutex_unlock(&ar->conf_mutex);

	return 0;
}

static void ath12k_mac_op_remove_chanctx(struct ieee80211_hw *hw,
					 struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k *ar;
	struct ath12k_base *ab;

	ar = ath12k_get_ar_by_ctx(hw, ctx);
	if (!ar)
		return;

	ab = ar->ab;

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac chanctx remove freq %u width %d ptr %p\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx);

	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	/* TODO: In case of there is one more channel context left, populate
	 * rx_channel with the channel of that remaining channel context.
	 */
	ar->rx_channel = NULL;
	spin_unlock_bh(&ar->data_lock);

	mutex_unlock(&ar->conf_mutex);
}

static enum wmi_phy_mode
ath12k_mac_check_down_grade_phy_mode(struct ath12k *ar,
				     enum wmi_phy_mode mode,
				     enum nl80211_band band,
				     enum nl80211_iftype type)
{
	struct ieee80211_sta_eht_cap *eht_cap = NULL;
	enum wmi_phy_mode down_mode;
	int n = ar->mac.sbands[band].n_iftype_data;
	int i;
	struct ieee80211_sband_iftype_data *data;

	if (mode < MODE_11BE_EHT20)
		return mode;

	data = ar->mac.iftype[band];
	for (i = 0; i < n; i++) {
		if (data[i].types_mask & BIT(type)) {
			eht_cap = &data[i].eht_cap;
			break;
		}
	}

	if (eht_cap && eht_cap->has_eht)
		return mode;

	switch (mode) {
	case MODE_11BE_EHT20:
		down_mode = MODE_11AX_HE20;
		break;
	case MODE_11BE_EHT40:
		down_mode = MODE_11AX_HE40;
		break;
	case MODE_11BE_EHT80:
		down_mode = MODE_11AX_HE80;
		break;
	case MODE_11BE_EHT80_80:
		down_mode = MODE_11AX_HE80_80;
		break;
	case MODE_11BE_EHT160:
	case MODE_11BE_EHT160_160:
	case MODE_11BE_EHT320:
		down_mode = MODE_11AX_HE160;
		break;
	case MODE_11BE_EHT20_2G:
		down_mode = MODE_11AX_HE20_2G;
		break;
	case MODE_11BE_EHT40_2G:
		down_mode = MODE_11AX_HE40_2G;
		break;
	default:
		down_mode = mode;
		break;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "mac vdev start phymode %s downgrade to %s\n",
		   ath12k_mac_phymode_str(mode),
		   ath12k_mac_phymode_str(down_mode));

	return down_mode;
}

static int
ath12k_mac_vdev_start_restart(struct ath12k_vif *arvif,
			      struct ieee80211_chanctx_conf *ctx,
			      bool restart)
{
	struct ath12k *ar = arvif->ar;
	struct ath12k_base *ab = ar->ab;
	struct wmi_vdev_start_req_arg arg = {};
	const struct cfg80211_chan_def *chandef = &ctx->def;
	int he_support = arvif->vif->bss_conf.he_support;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	reinit_completion(&ar->vdev_setup_done);

	arg.vdev_id = arvif->vdev_id;
	arg.dtim_period = arvif->dtim_period;
	arg.bcn_intval = arvif->beacon_interval;
	arg.punct_bitmap = ~arvif->punct_bitmap;

	arg.freq = chandef->chan->center_freq;
	arg.band_center_freq1 = chandef->center_freq1;
	arg.band_center_freq2 = chandef->center_freq2;
	arg.mode = ath12k_phymodes[chandef->chan->band][chandef->width];

	arg.mode = ath12k_mac_check_down_grade_phy_mode(ar, arg.mode,
							chandef->chan->band,
							arvif->vif->type);
	arg.min_power = 0;
	arg.max_power = chandef->chan->max_power * 2;
	arg.max_reg_power = chandef->chan->max_reg_power * 2;
	arg.max_antenna_gain = chandef->chan->max_antenna_gain * 2;

	arg.pref_tx_streams = ar->num_tx_chains;
	arg.pref_rx_streams = ar->num_rx_chains;

	arg.mbssid_flags = WMI_VDEV_MBSSID_FLAGS_NON_MBSSID_AP;
	arg.mbssid_tx_vdev_id = 0;
	if (test_bit(WMI_TLV_SERVICE_MBSS_PARAM_IN_VDEV_START_SUPPORT,
		     ar->ab->wmi_ab.svc_map)) {
		ret = ath12k_mac_setup_vdev_params_mbssid(arvif,
							  &arg.mbssid_flags,
							  &arg.mbssid_tx_vdev_id);
		if (ret)
			return ret;
	}

	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		arg.ssid = arvif->u.ap.ssid;
		arg.ssid_len = arvif->u.ap.ssid_len;
		arg.hidden_ssid = arvif->u.ap.hidden_ssid;

		/* For now allow DFS for AP mode */
		arg.chan_radar = !!(chandef->chan->flags & IEEE80211_CHAN_RADAR);

		arg.freq2_radar = ctx->radar_enabled;

		arg.passive = arg.chan_radar;

		spin_lock_bh(&ab->base_lock);
		arg.regdomain = ar->ab->dfs_region;
		spin_unlock_bh(&ab->base_lock);

		/* TODO: Notify if secondary 80Mhz also needs radar detection */
		if (he_support) {
			ret = ath12k_set_he_mu_sounding_mode(ar, arvif);
			if (ret) {
				ath12k_warn(ar->ab, "failed to set he mode vdev %i\n",
					    arg.vdev_id);
				return ret;
			}
		}
	}

	arg.passive |= !!(chandef->chan->flags & IEEE80211_CHAN_NO_IR);

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac vdev %d start center_freq %d phymode %s punct_bitmap 0x%x\n",
		   arg.vdev_id, arg.freq,
		   ath12k_mac_phymode_str(arg.mode), arg.punct_bitmap);

	ret = ath12k_wmi_vdev_start(ar, &arg, restart);
	if (ret) {
		ath12k_warn(ar->ab, "failed to %s WMI vdev %i\n",
			    restart ? "restart" : "start", arg.vdev_id);
		return ret;
	}

	ret = ath12k_mac_vdev_setup_sync(ar);
	if (ret) {
		ath12k_warn(ab, "failed to synchronize setup for vdev %i %s: %d\n",
			    arg.vdev_id, restart ? "restart" : "start", ret);
		return ret;
	}

	ar->num_started_vdevs++;
	ath12k_dbg(ab, ATH12K_DBG_MAC,  "vdev %pM started, vdev_id %d\n",
		   arvif->vif->addr, arvif->vdev_id);

	/* Enable CAC Flag in the driver by checking the channel DFS cac time,
	 * i.e dfs_cac_ms value which will be valid only for radar channels
	 * and state as NL80211_DFS_USABLE which indicates CAC needs to be
	 * done before channel usage. This flags is used to drop rx packets.
	 * during CAC.
	 */
	/* TODO: Set the flag for other interface types as required */
	if (arvif->vdev_type == WMI_VDEV_TYPE_AP &&
	    chandef->chan->dfs_cac_ms &&
	    chandef->chan->dfs_state == NL80211_DFS_USABLE) {
		set_bit(ATH12K_CAC_RUNNING, &ar->dev_flags);
		ath12k_dbg(ab, ATH12K_DBG_MAC,
			   "CAC Started in chan_freq %d for vdev %d\n",
			   arg.freq, arg.vdev_id);
	}

	ret = ath12k_mac_set_txbf_conf(arvif);
	if (ret)
		ath12k_warn(ab, "failed to set txbf conf for vdev %d: %d\n",
			    arvif->vdev_id, ret);

	return 0;
}

static int ath12k_mac_vdev_start(struct ath12k_vif *arvif,
				 struct ieee80211_chanctx_conf *ctx)
{
	return ath12k_mac_vdev_start_restart(arvif, ctx, false);
}

static int ath12k_mac_vdev_restart(struct ath12k_vif *arvif,
				   struct ieee80211_chanctx_conf *ctx)
{
	return ath12k_mac_vdev_start_restart(arvif, ctx, true);
}

struct ath12k_mac_change_chanctx_arg {
	struct ieee80211_chanctx_conf *ctx;
	struct ieee80211_vif_chanctx_switch *vifs;
	int n_vifs;
	int next_vif;
	struct ath12k *ar;
};

static void
ath12k_mac_change_chanctx_cnt_iter(void *data, u8 *mac,
				   struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_mac_change_chanctx_arg *arg = data;

	if (arvif->ar != arg->ar)
		return;

	if (rcu_access_pointer(vif->bss_conf.chanctx_conf) != arg->ctx)
		return;

	arg->n_vifs++;
}

static void
ath12k_mac_change_chanctx_fill_iter(void *data, u8 *mac,
				    struct ieee80211_vif *vif)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_mac_change_chanctx_arg *arg = data;
	struct ieee80211_chanctx_conf *ctx;

	if (arvif->ar != arg->ar)
		return;

	ctx = rcu_access_pointer(vif->bss_conf.chanctx_conf);
	if (ctx != arg->ctx)
		return;

	if (WARN_ON(arg->next_vif == arg->n_vifs))
		return;

	arg->vifs[arg->next_vif].vif = vif;
	arg->vifs[arg->next_vif].old_ctx = ctx;
	arg->vifs[arg->next_vif].new_ctx = ctx;
	arg->next_vif++;
}

static u32 ath12k_mac_nlwidth_to_wmiwidth(enum nl80211_chan_width width)
{
	switch (width) {
	case NL80211_CHAN_WIDTH_20:
		return WMI_CHAN_WIDTH_20;
	case NL80211_CHAN_WIDTH_40:
		return WMI_CHAN_WIDTH_40;
	case NL80211_CHAN_WIDTH_80:
		return WMI_CHAN_WIDTH_80;
	case NL80211_CHAN_WIDTH_160:
		return WMI_CHAN_WIDTH_160;
	case NL80211_CHAN_WIDTH_80P80:
		return WMI_CHAN_WIDTH_80P80;
	case NL80211_CHAN_WIDTH_5:
		return WMI_CHAN_WIDTH_5;
	case NL80211_CHAN_WIDTH_10:
		return WMI_CHAN_WIDTH_10;
	case NL80211_CHAN_WIDTH_320:
		return WMI_CHAN_WIDTH_320;
	default:
		WARN_ON(1);
		return WMI_CHAN_WIDTH_20;
	}
}

static int ath12k_mac_update_peer_puncturing_width(struct ath12k *ar,
						   struct ath12k_vif *arvif,
						   struct cfg80211_chan_def def)
{
	u32 param_id, param_value;
	int ret;

	if (arvif->vdev_type != WMI_VDEV_TYPE_STA)
		return 0;

	param_id = WMI_PEER_CHWIDTH_PUNCTURE_20MHZ_BITMAP;
	param_value = ath12k_mac_nlwidth_to_wmiwidth(def.width) |
		u32_encode_bits((~def.punctured),
				WMI_PEER_PUNCTURE_BITMAP);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "punctured bitmap %02x width %d vdev %d\n",
		   def.punctured, def.width, arvif->vdev_id);

	ret = ath12k_wmi_set_peer_param(ar, arvif->bssid,
					arvif->vdev_id, param_id,
					param_value);

	return ret;
}

static void
ath12k_mac_update_vif_chan(struct ath12k *ar,
			   struct ieee80211_vif_chanctx_switch *vifs,
			   int n_vifs)
{
	struct ath12k_wmi_vdev_up_params params = {};
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_vif *vif;
	struct ath12k_vif *arvif;
	int ret;
	int i;
	bool monitor_vif = false;

	lockdep_assert_held(&ar->conf_mutex);

	for (i = 0; i < n_vifs; i++) {
		vif = vifs[i].vif;
		arvif = ath12k_vif_to_arvif(vif);

		if (vif->type == NL80211_IFTYPE_MONITOR)
			monitor_vif = true;

		ath12k_dbg(ab, ATH12K_DBG_MAC,
			   "mac chanctx switch vdev_id %i freq %u->%u width %d->%d\n",
			   arvif->vdev_id,
			   vifs[i].old_ctx->def.chan->center_freq,
			   vifs[i].new_ctx->def.chan->center_freq,
			   vifs[i].old_ctx->def.width,
			   vifs[i].new_ctx->def.width);

		if (WARN_ON(!arvif->is_started))
			continue;

		arvif->punct_bitmap = vifs[i].new_ctx->def.punctured;

		/* Firmware expect vdev_restart only if vdev is up.
		 * If vdev is down then it expect vdev_stop->vdev_start.
		 */
		if (arvif->is_up) {
			ret = ath12k_mac_vdev_restart(arvif, vifs[i].new_ctx);
			if (ret) {
				ath12k_warn(ab, "failed to restart vdev %d: %d\n",
					    arvif->vdev_id, ret);
				continue;
			}
		} else {
			ret = ath12k_mac_vdev_stop(arvif);
			if (ret) {
				ath12k_warn(ab, "failed to stop vdev %d: %d\n",
					    arvif->vdev_id, ret);
				continue;
			}

			ret = ath12k_mac_vdev_start(arvif, vifs[i].new_ctx);
			if (ret)
				ath12k_warn(ab, "failed to start vdev %d: %d\n",
					    arvif->vdev_id, ret);
			continue;
		}

		ret = ath12k_mac_setup_bcn_tmpl(arvif);
		if (ret)
			ath12k_warn(ab, "failed to update bcn tmpl during csa: %d\n",
				    ret);

		memset(&params, 0, sizeof(params));
		params.vdev_id = arvif->vdev_id;
		params.aid = arvif->aid;
		params.bssid = arvif->bssid;
		if (vif->mbssid_tx_vif) {
			params.tx_bssid = ath12k_vif_to_arvif(vif->mbssid_tx_vif)->bssid;
			params.nontx_profile_idx = vif->bss_conf.bssid_index;
			params.nontx_profile_cnt = 1 << vif->bss_conf.bssid_indicator;
		}
		ret = ath12k_wmi_vdev_up(arvif->ar, &params);
		if (ret) {
			ath12k_warn(ab, "failed to bring vdev up %d: %d\n",
				    arvif->vdev_id, ret);
			continue;
		}

		ret = ath12k_mac_update_peer_puncturing_width(arvif->ar, arvif,
							      vifs[i].new_ctx->def);
		if (ret) {
			ath12k_warn(ar->ab,
				    "failed to update puncturing bitmap %02x and width %d: %d\n",
				    vifs[i].new_ctx->def.punctured,
				    vifs[i].new_ctx->def.width, ret);
			continue;
		}
	}

	/* Restart the internal monitor vdev on new channel */
	if (!monitor_vif && ar->monitor_vdev_created) {
		if (!ath12k_mac_monitor_stop(ar))
			ath12k_mac_monitor_start(ar);
	}
}

static void
ath12k_mac_update_active_vif_chan(struct ath12k *ar,
				  struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k_mac_change_chanctx_arg arg = { .ctx = ctx, .ar = ar };
	struct ieee80211_hw *hw = ath12k_ar_to_hw(ar);

	lockdep_assert_held(&ar->conf_mutex);

	ieee80211_iterate_active_interfaces_atomic(hw,
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath12k_mac_change_chanctx_cnt_iter,
						   &arg);
	if (arg.n_vifs == 0)
		return;

	arg.vifs = kcalloc(arg.n_vifs, sizeof(arg.vifs[0]), GFP_KERNEL);
	if (!arg.vifs)
		return;

	ieee80211_iterate_active_interfaces_atomic(hw,
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath12k_mac_change_chanctx_fill_iter,
						   &arg);

	ath12k_mac_update_vif_chan(ar, arg.vifs, arg.n_vifs);

	kfree(arg.vifs);
}

static void ath12k_mac_op_change_chanctx(struct ieee80211_hw *hw,
					 struct ieee80211_chanctx_conf *ctx,
					 u32 changed)
{
	struct ath12k *ar;
	struct ath12k_base *ab;

	ar = ath12k_get_ar_by_ctx(hw, ctx);
	if (!ar)
		return;

	ab = ar->ab;

	mutex_lock(&ar->conf_mutex);

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac chanctx change freq %u width %d ptr %p changed %x\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx, changed);

	/* This shouldn't really happen because channel switching should use
	 * switch_vif_chanctx().
	 */
	if (WARN_ON(changed & IEEE80211_CHANCTX_CHANGE_CHANNEL))
		goto unlock;

	if (changed & IEEE80211_CHANCTX_CHANGE_WIDTH ||
	    changed & IEEE80211_CHANCTX_CHANGE_RADAR ||
	    changed & IEEE80211_CHANCTX_CHANGE_PUNCTURING)
		ath12k_mac_update_active_vif_chan(ar, ctx);

	/* TODO: Recalc radar detection */

unlock:
	mutex_unlock(&ar->conf_mutex);
}

static int ath12k_start_vdev_delay(struct ath12k *ar,
				   struct ath12k_vif *arvif)
{
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_vif *vif = arvif->vif;
	int ret;

	if (WARN_ON(arvif->is_started))
		return -EBUSY;

	ret = ath12k_mac_vdev_start(arvif, &arvif->chanctx);
	if (ret) {
		ath12k_warn(ab, "failed to start vdev %i addr %pM on freq %d: %d\n",
			    arvif->vdev_id, vif->addr,
			    arvif->chanctx.def.chan->center_freq, ret);
		return ret;
	}

	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ret = ath12k_monitor_vdev_up(ar, arvif->vdev_id);
		if (ret) {
			ath12k_warn(ab, "failed put monitor up: %d\n", ret);
			return ret;
		}
	}

	arvif->is_started = true;

	/* TODO: Setup ps and cts/rts protection */
	return 0;
}

static int
ath12k_mac_op_assign_vif_chanctx(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_bss_conf *link_conf,
				 struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k *ar;
	struct ath12k_base *ab;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int ret;

	/* For multi radio wiphy, the vdev was not created during add_interface
	 * create now since we have a channel ctx now to assign to a specific ar/fw
	 */
	ar = ath12k_mac_assign_vif_to_vdev(hw, vif, ctx);
	if (!ar) {
		WARN_ON(1);
		return -EINVAL;
	}

	ab = ar->ab;

	mutex_lock(&ar->conf_mutex);

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac chanctx assign ptr %p vdev_id %i\n",
		   ctx, arvif->vdev_id);

	arvif->punct_bitmap = ctx->def.punctured;

	/* for some targets bss peer must be created before vdev_start */
	if (ab->hw_params->vdev_start_delay &&
	    arvif->vdev_type != WMI_VDEV_TYPE_AP &&
	    arvif->vdev_type != WMI_VDEV_TYPE_MONITOR &&
	    !ath12k_peer_exist_by_vdev_id(ab, arvif->vdev_id)) {
		memcpy(&arvif->chanctx, ctx, sizeof(*ctx));
		ret = 0;
		goto out;
	}

	if (WARN_ON(arvif->is_started)) {
		ret = -EBUSY;
		goto out;
	}

	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ret = ath12k_mac_monitor_start(ar);
		if (ret)
			goto out;
		arvif->is_started = true;
		goto out;
	}

	ret = ath12k_mac_vdev_start(arvif, ctx);
	if (ret) {
		ath12k_warn(ab, "failed to start vdev %i addr %pM on freq %d: %d\n",
			    arvif->vdev_id, vif->addr,
			    ctx->def.chan->center_freq, ret);
		goto out;
	}

	if (arvif->vdev_type != WMI_VDEV_TYPE_MONITOR && ar->monitor_vdev_created)
		ath12k_mac_monitor_start(ar);

	arvif->is_started = true;

	/* TODO: Setup ps and cts/rts protection */

out:
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void
ath12k_mac_op_unassign_vif_chanctx(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_bss_conf *link_conf,
				   struct ieee80211_chanctx_conf *ctx)
{
	struct ath12k *ar;
	struct ath12k_base *ab;
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	int ret;

	/* The vif is expected to be attached to an ar's VDEV.
	 * We leave the vif/vdev in this function as is
	 * and not delete the vdev symmetric to assign_vif_chanctx()
	 * the VDEV will be deleted and unassigned either during
	 * remove_interface() or when there is a change in channel
	 * that moves the vif to a new ar
	 */
	if (!arvif->is_created)
		return;

	ar = arvif->ar;
	ab = ar->ab;

	mutex_lock(&ar->conf_mutex);

	ath12k_dbg(ab, ATH12K_DBG_MAC,
		   "mac chanctx unassign ptr %p vdev_id %i\n",
		   ctx, arvif->vdev_id);

	WARN_ON(!arvif->is_started);

	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ret = ath12k_mac_monitor_stop(ar);
		if (ret) {
			mutex_unlock(&ar->conf_mutex);
			return;
		}

		arvif->is_started = false;
	}

	if (arvif->vdev_type != WMI_VDEV_TYPE_STA &&
	    arvif->vdev_type != WMI_VDEV_TYPE_MONITOR) {
		ath12k_bss_disassoc(ar, arvif);
		ret = ath12k_mac_vdev_stop(arvif);
		if (ret)
			ath12k_warn(ab, "failed to stop vdev %i: %d\n",
				    arvif->vdev_id, ret);
	}
	arvif->is_started = false;

	if (arvif->vdev_type != WMI_VDEV_TYPE_MONITOR &&
	    ar->num_started_vdevs == 1 && ar->monitor_vdev_created)
		ath12k_mac_monitor_stop(ar);

	mutex_unlock(&ar->conf_mutex);
}

static int
ath12k_mac_op_switch_vif_chanctx(struct ieee80211_hw *hw,
				 struct ieee80211_vif_chanctx_switch *vifs,
				 int n_vifs,
				 enum ieee80211_chanctx_switch_mode mode)
{
	struct ath12k *ar;

	ar = ath12k_get_ar_by_ctx(hw, vifs->old_ctx);
	if (!ar)
		return -EINVAL;

	mutex_lock(&ar->conf_mutex);

	/* Switching channels across radio is not allowed */
	if (ar != ath12k_get_ar_by_ctx(hw, vifs->new_ctx)) {
		mutex_unlock(&ar->conf_mutex);
		return -EINVAL;
	}

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
		   "mac chanctx switch n_vifs %d mode %d\n",
		   n_vifs, mode);
	ath12k_mac_update_vif_chan(ar, vifs, n_vifs);

	mutex_unlock(&ar->conf_mutex);

	return 0;
}

static int
ath12k_set_vdev_param_to_all_vifs(struct ath12k *ar, int param, u32 value)
{
	struct ath12k_vif *arvif;
	int ret = 0;

	mutex_lock(&ar->conf_mutex);
	list_for_each_entry(arvif, &ar->arvifs, list) {
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "setting mac vdev %d param %d value %d\n",
			   param, arvif->vdev_id, value);

		ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param, value);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set param %d for vdev %d: %d\n",
				    param, arvif->vdev_id, ret);
			break;
		}
	}
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

/* mac80211 stores device specific RTS/Fragmentation threshold value,
 * this is set interface specific to firmware from ath12k driver
 */
static int ath12k_mac_op_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int param_id = WMI_VDEV_PARAM_RTS_THRESHOLD, ret = 0, i;

	/* Currently we set the rts threshold value to all the vifs across
	 * all radios of the single wiphy.
	 * TODO Once support for vif specific RTS threshold in mac80211 is
	 * available, ath12k can make use of it.
	 */
	for_each_ar(ah, ar, i) {
		ret = ath12k_set_vdev_param_to_all_vifs(ar, param_id, value);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set RTS config for all vdevs of pdev %d",
				    ar->pdev->pdev_id);
			break;
		}
	}

	return ret;
}

static int ath12k_mac_op_set_frag_threshold(struct ieee80211_hw *hw, u32 value)
{
	/* Even though there's a WMI vdev param for fragmentation threshold no
	 * known firmware actually implements it. Moreover it is not possible to
	 * rely frame fragmentation to mac80211 because firmware clears the
	 * "more fragments" bit in frame control making it impossible for remote
	 * devices to reassemble frames.
	 *
	 * Hence implement a dummy callback just to say fragmentation isn't
	 * supported. This effectively prevents mac80211 from doing frame
	 * fragmentation in software.
	 */
	return -EOPNOTSUPP;
}

static int ath12k_mac_flush(struct ath12k *ar)
{
	long time_left;
	int ret = 0;

	time_left = wait_event_timeout(ar->dp.tx_empty_waitq,
				       (atomic_read(&ar->dp.num_tx_pending) == 0),
				       ATH12K_FLUSH_TIMEOUT);
	if (time_left == 0) {
		ath12k_warn(ar->ab,
			    "failed to flush transmit queue, data pkts pending %d\n",
			    atomic_read(&ar->dp.num_tx_pending));
		ret = -ETIMEDOUT;
	}

	time_left = wait_event_timeout(ar->txmgmt_empty_waitq,
				       (atomic_read(&ar->num_pending_mgmt_tx) == 0),
				       ATH12K_FLUSH_TIMEOUT);
	if (time_left == 0) {
		ath12k_warn(ar->ab,
			    "failed to flush mgmt transmit queue, mgmt pkts pending %d\n",
			    atomic_read(&ar->num_pending_mgmt_tx));
		ret = -ETIMEDOUT;
	}

	return ret;
}

int ath12k_mac_wait_tx_complete(struct ath12k *ar)
{
	ath12k_mac_drain_tx(ar);
	return ath12k_mac_flush(ar);
}

static void ath12k_mac_op_flush(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
				u32 queues, bool drop)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	int i;

	if (drop)
		return;

	/* vif can be NULL when flush() is considered for hw */
	if (!vif) {
		for_each_ar(ah, ar, i)
			ath12k_mac_flush(ar);
		return;
	}

	ar = ath12k_get_ar_by_vif(hw, vif);

	if (!ar)
		return;

	ath12k_mac_flush(ar);
}

static int
ath12k_mac_bitrate_mask_num_ht_rates(struct ath12k *ar,
				     enum nl80211_band band,
				     const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].ht_mcs); i++)
		num_rates += hweight16(mask->control[band].ht_mcs[i]);

	return num_rates;
}

static bool
ath12k_mac_has_single_legacy_rate(struct ath12k *ar,
				  enum nl80211_band band,
				  const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;

	num_rates = hweight32(mask->control[band].legacy);

	if (ath12k_mac_bitrate_mask_num_ht_rates(ar, band, mask))
		return false;

	if (ath12k_mac_bitrate_mask_num_vht_rates(ar, band, mask))
		return false;

	return num_rates == 1;
}

static bool
ath12k_mac_bitrate_mask_get_single_nss(struct ath12k *ar,
				       enum nl80211_band band,
				       const struct cfg80211_bitrate_mask *mask,
				       int *nss)
{
	struct ieee80211_supported_band *sband = &ar->mac.sbands[band];
	u16 vht_mcs_map = le16_to_cpu(sband->vht_cap.vht_mcs.tx_mcs_map);
	u8 ht_nss_mask = 0;
	u8 vht_nss_mask = 0;
	int i;

	/* No need to consider legacy here. Basic rates are always present
	 * in bitrate mask
	 */

	for (i = 0; i < ARRAY_SIZE(mask->control[band].ht_mcs); i++) {
		if (mask->control[band].ht_mcs[i] == 0)
			continue;
		else if (mask->control[band].ht_mcs[i] ==
			 sband->ht_cap.mcs.rx_mask[i])
			ht_nss_mask |= BIT(i);
		else
			return false;
	}

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++) {
		if (mask->control[band].vht_mcs[i] == 0)
			continue;
		else if (mask->control[band].vht_mcs[i] ==
			 ath12k_mac_get_max_vht_mcs_map(vht_mcs_map, i))
			vht_nss_mask |= BIT(i);
		else
			return false;
	}

	if (ht_nss_mask != vht_nss_mask)
		return false;

	if (ht_nss_mask == 0)
		return false;

	if (BIT(fls(ht_nss_mask)) - 1 != ht_nss_mask)
		return false;

	*nss = fls(ht_nss_mask);

	return true;
}

static int
ath12k_mac_get_single_legacy_rate(struct ath12k *ar,
				  enum nl80211_band band,
				  const struct cfg80211_bitrate_mask *mask,
				  u32 *rate, u8 *nss)
{
	int rate_idx;
	u16 bitrate;
	u8 preamble;
	u8 hw_rate;

	if (hweight32(mask->control[band].legacy) != 1)
		return -EINVAL;

	rate_idx = ffs(mask->control[band].legacy) - 1;

	if (band == NL80211_BAND_5GHZ || band == NL80211_BAND_6GHZ)
		rate_idx += ATH12K_MAC_FIRST_OFDM_RATE_IDX;

	hw_rate = ath12k_legacy_rates[rate_idx].hw_value;
	bitrate = ath12k_legacy_rates[rate_idx].bitrate;

	if (ath12k_mac_bitrate_is_cck(bitrate))
		preamble = WMI_RATE_PREAMBLE_CCK;
	else
		preamble = WMI_RATE_PREAMBLE_OFDM;

	*nss = 1;
	*rate = ATH12K_HW_RATE_CODE(hw_rate, 0, preamble);

	return 0;
}

static int ath12k_mac_set_fixed_rate_params(struct ath12k_vif *arvif,
					    u32 rate, u8 nss, u8 sgi, u8 ldpc)
{
	struct ath12k *ar = arvif->ar;
	u32 vdev_param;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac set fixed rate params vdev %i rate 0x%02x nss %u sgi %u\n",
		   arvif->vdev_id, rate, nss, sgi);

	vdev_param = WMI_VDEV_PARAM_FIXED_RATE;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, rate);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set fixed rate param 0x%02x: %d\n",
			    rate, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_NSS;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, nss);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set nss param %d: %d\n",
			    nss, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_SGI;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, sgi);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set sgi param %d: %d\n",
			    sgi, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_LDPC;
	ret = ath12k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, ldpc);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set ldpc param %d: %d\n",
			    ldpc, ret);
		return ret;
	}

	return 0;
}

static bool
ath12k_mac_vht_mcs_range_present(struct ath12k *ar,
				 enum nl80211_band band,
				 const struct cfg80211_bitrate_mask *mask)
{
	int i;
	u16 vht_mcs;

	for (i = 0; i < NL80211_VHT_NSS_MAX; i++) {
		vht_mcs = mask->control[band].vht_mcs[i];

		switch (vht_mcs) {
		case 0:
		case BIT(8) - 1:
		case BIT(9) - 1:
		case BIT(10) - 1:
			break;
		default:
			return false;
		}
	}

	return true;
}

static void ath12k_mac_set_bitrate_mask_iter(void *data,
					     struct ieee80211_sta *sta)
{
	struct ath12k_vif *arvif = data;
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);
	struct ath12k *ar = arvif->ar;

	if (arsta->arvif != arvif)
		return;

	spin_lock_bh(&ar->data_lock);
	arsta->changed |= IEEE80211_RC_SUPP_RATES_CHANGED;
	spin_unlock_bh(&ar->data_lock);

	ieee80211_queue_work(ath12k_ar_to_hw(ar), &arsta->update_wk);
}

static void ath12k_mac_disable_peer_fixed_rate(void *data,
					       struct ieee80211_sta *sta)
{
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);
	struct ath12k_vif *arvif = data;
	struct ath12k *ar = arvif->ar;
	int ret;

	if (arsta->arvif != arvif)
		return;

	ret = ath12k_wmi_set_peer_param(ar, sta->addr,
					arvif->vdev_id,
					WMI_PEER_PARAM_FIXED_RATE,
					WMI_FIXED_RATE_NONE);
	if (ret)
		ath12k_warn(ar->ab,
			    "failed to disable peer fixed rate for STA %pM ret %d\n",
			    sta->addr, ret);
}

static int
ath12k_mac_op_set_bitrate_mask(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       const struct cfg80211_bitrate_mask *mask)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct cfg80211_chan_def def;
	struct ath12k *ar = arvif->ar;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	const u16 *vht_mcs_mask;
	u32 rate;
	u8 nss;
	u8 sgi;
	u8 ldpc;
	int single_nss;
	int ret;
	int num_rates;

	if (ath12k_mac_vif_chan(vif, &def))
		return -EPERM;

	band = def.chan->band;
	ht_mcs_mask = mask->control[band].ht_mcs;
	vht_mcs_mask = mask->control[band].vht_mcs;
	ldpc = !!(ar->ht_cap_info & WMI_HT_CAP_LDPC);

	sgi = mask->control[band].gi;
	if (sgi == NL80211_TXRATE_FORCE_LGI) {
		ret = -EINVAL;
		goto out;
	}

	/* mac80211 doesn't support sending a fixed HT/VHT MCS alone, rather it
	 * requires passing at least one of used basic rates along with them.
	 * Fixed rate setting across different preambles(legacy, HT, VHT) is
	 * not supported by the FW. Hence use of FIXED_RATE vdev param is not
	 * suitable for setting single HT/VHT rates.
	 * But, there could be a single basic rate passed from userspace which
	 * can be done through the FIXED_RATE param.
	 */
	if (ath12k_mac_has_single_legacy_rate(ar, band, mask)) {
		ret = ath12k_mac_get_single_legacy_rate(ar, band, mask, &rate,
							&nss);
		if (ret) {
			ath12k_warn(ar->ab, "failed to get single legacy rate for vdev %i: %d\n",
				    arvif->vdev_id, ret);
			goto out;
		}
		ieee80211_iterate_stations_mtx(hw,
					       ath12k_mac_disable_peer_fixed_rate,
					       arvif);
	} else if (ath12k_mac_bitrate_mask_get_single_nss(ar, band, mask,
							  &single_nss)) {
		rate = WMI_FIXED_RATE_NONE;
		nss = single_nss;
	} else {
		rate = WMI_FIXED_RATE_NONE;
		nss = min_t(u32, ar->num_tx_chains,
			    max(ath12k_mac_max_ht_nss(ht_mcs_mask),
				ath12k_mac_max_vht_nss(vht_mcs_mask)));

		/* If multiple rates across different preambles are given
		 * we can reconfigure this info with all peers using PEER_ASSOC
		 * command with the below exception cases.
		 * - Single VHT Rate : peer_assoc command accommodates only MCS
		 * range values i.e 0-7, 0-8, 0-9 for VHT. Though mac80211
		 * mandates passing basic rates along with HT/VHT rates, FW
		 * doesn't allow switching from VHT to Legacy. Hence instead of
		 * setting legacy and VHT rates using RATEMASK_CMD vdev cmd,
		 * we could set this VHT rate as peer fixed rate param, which
		 * will override FIXED rate and FW rate control algorithm.
		 * If single VHT rate is passed along with HT rates, we select
		 * the VHT rate as fixed rate for vht peers.
		 * - Multiple VHT Rates : When Multiple VHT rates are given,this
		 * can be set using RATEMASK CMD which uses FW rate-ctl alg.
		 * TODO: Setting multiple VHT MCS and replacing peer_assoc with
		 * RATEMASK_CMDID can cover all use cases of setting rates
		 * across multiple preambles and rates within same type.
		 * But requires more validation of the command at this point.
		 */

		num_rates = ath12k_mac_bitrate_mask_num_vht_rates(ar, band,
								  mask);

		if (!ath12k_mac_vht_mcs_range_present(ar, band, mask) &&
		    num_rates > 1) {
			/* TODO: Handle multiple VHT MCS values setting using
			 * RATEMASK CMD
			 */
			ath12k_warn(ar->ab,
				    "Setting more than one MCS Value in bitrate mask not supported\n");
			ret = -EINVAL;
			goto out;
		}

		ieee80211_iterate_stations_mtx(hw,
					       ath12k_mac_disable_peer_fixed_rate,
					       arvif);

		mutex_lock(&ar->conf_mutex);

		arvif->bitrate_mask = *mask;
		ieee80211_iterate_stations_mtx(hw,
					       ath12k_mac_set_bitrate_mask_iter,
					       arvif);

		mutex_unlock(&ar->conf_mutex);
	}

	mutex_lock(&ar->conf_mutex);

	ret = ath12k_mac_set_fixed_rate_params(arvif, rate, nss, sgi, ldpc);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set fixed rate params on vdev %i: %d\n",
			    arvif->vdev_id, ret);
	}

	mutex_unlock(&ar->conf_mutex);

out:
	return ret;
}

static void
ath12k_mac_op_reconfig_complete(struct ieee80211_hw *hw,
				enum ieee80211_reconfig_type reconfig_type)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;
	struct ath12k_base *ab;
	struct ath12k_vif *arvif;
	int recovery_count, i;

	if (reconfig_type != IEEE80211_RECONFIG_TYPE_RESTART)
		return;

	guard(mutex)(&ah->hw_mutex);

	if (ah->state != ATH12K_HW_STATE_RESTARTED)
		return;

	ah->state = ATH12K_HW_STATE_ON;
	ieee80211_wake_queues(hw);

	for_each_ar(ah, ar, i) {
		mutex_lock(&ar->conf_mutex);

		ab = ar->ab;

		ath12k_warn(ar->ab, "pdev %d successfully recovered\n",
			    ar->pdev->pdev_id);

		if (ab->is_reset) {
			recovery_count = atomic_inc_return(&ab->recovery_count);

			ath12k_dbg(ab, ATH12K_DBG_BOOT, "recovery count %d\n",
				   recovery_count);

			/* When there are multiple radios in an SOC,
			 * the recovery has to be done for each radio
			 */
			if (recovery_count == ab->num_radios) {
				atomic_dec(&ab->reset_count);
				complete(&ab->reset_complete);
				ab->is_reset = false;
				atomic_set(&ab->fail_cont_count, 0);
				ath12k_dbg(ab, ATH12K_DBG_BOOT, "reset success\n");
			}
		}

		list_for_each_entry(arvif, &ar->arvifs, list) {
			ath12k_dbg(ab, ATH12K_DBG_BOOT,
				   "reconfig cipher %d up %d vdev type %d\n",
				   arvif->key_cipher,
				   arvif->is_up,
				   arvif->vdev_type);

			/* After trigger disconnect, then upper layer will
			 * trigger connect again, then the PN number of
			 * upper layer will be reset to keep up with AP
			 * side, hence PN number mismatch will not happen.
			 */
			if (arvif->is_up &&
			    arvif->vdev_type == WMI_VDEV_TYPE_STA &&
			    arvif->vdev_subtype == WMI_VDEV_SUBTYPE_NONE) {
				ieee80211_hw_restart_disconnect(arvif->vif);

				ath12k_dbg(ab, ATH12K_DBG_BOOT,
					   "restart disconnect\n");
			}
		}

		mutex_unlock(&ar->conf_mutex);
	}
}

static void
ath12k_mac_update_bss_chan_survey(struct ath12k *ar,
				  struct ieee80211_channel *channel)
{
	int ret;
	enum wmi_bss_chan_info_req_type type = WMI_BSS_SURVEY_REQ_TYPE_READ;

	lockdep_assert_held(&ar->conf_mutex);

	if (!test_bit(WMI_TLV_SERVICE_BSS_CHANNEL_INFO_64, ar->ab->wmi_ab.svc_map) ||
	    ar->rx_channel != channel)
		return;

	if (ar->scan.state != ATH12K_SCAN_IDLE) {
		ath12k_dbg(ar->ab, ATH12K_DBG_MAC,
			   "ignoring bss chan info req while scanning..\n");
		return;
	}

	reinit_completion(&ar->bss_survey_done);

	ret = ath12k_wmi_pdev_bss_chan_info_request(ar, type);
	if (ret) {
		ath12k_warn(ar->ab, "failed to send pdev bss chan info request\n");
		return;
	}

	ret = wait_for_completion_timeout(&ar->bss_survey_done, 3 * HZ);
	if (ret == 0)
		ath12k_warn(ar->ab, "bss channel survey timed out\n");
}

static int ath12k_mac_op_get_survey(struct ieee80211_hw *hw, int idx,
				    struct survey_info *survey)
{
	struct ath12k *ar;
	struct ieee80211_supported_band *sband;
	struct survey_info *ar_survey;

	if (idx >= ATH12K_NUM_CHANS)
		return -ENOENT;

	sband = hw->wiphy->bands[NL80211_BAND_2GHZ];
	if (sband && idx >= sband->n_channels) {
		idx -= sband->n_channels;
		sband = NULL;
	}

	if (!sband)
		sband = hw->wiphy->bands[NL80211_BAND_5GHZ];
	if (sband && idx >= sband->n_channels) {
		idx -= sband->n_channels;
		sband = NULL;
	}

	if (!sband)
		sband = hw->wiphy->bands[NL80211_BAND_6GHZ];

	if (!sband || idx >= sband->n_channels)
		return -ENOENT;

	ar = ath12k_mac_get_ar_by_chan(hw, &sband->channels[idx]);
	if (!ar) {
		if (sband->channels[idx].flags & IEEE80211_CHAN_DISABLED) {
			memset(survey, 0, sizeof(*survey));
			return 0;
		}
		return -ENOENT;
	}

	ar_survey = &ar->survey[idx];

	mutex_lock(&ar->conf_mutex);

	ath12k_mac_update_bss_chan_survey(ar, &sband->channels[idx]);

	spin_lock_bh(&ar->data_lock);
	memcpy(survey, ar_survey, sizeof(*survey));
	spin_unlock_bh(&ar->data_lock);

	survey->channel = &sband->channels[idx];

	if (ar->rx_channel == survey->channel)
		survey->filled |= SURVEY_INFO_IN_USE;

	mutex_unlock(&ar->conf_mutex);
	return 0;
}

static void ath12k_mac_op_sta_statistics(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 struct ieee80211_sta *sta,
					 struct station_info *sinfo)
{
	struct ath12k_sta *arsta = ath12k_sta_to_arsta(sta);

	sinfo->rx_duration = arsta->rx_duration;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_DURATION);

	sinfo->tx_duration = arsta->tx_duration;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_DURATION);

	if (!arsta->txrate.legacy && !arsta->txrate.nss)
		return;

	if (arsta->txrate.legacy) {
		sinfo->txrate.legacy = arsta->txrate.legacy;
	} else {
		sinfo->txrate.mcs = arsta->txrate.mcs;
		sinfo->txrate.nss = arsta->txrate.nss;
		sinfo->txrate.bw = arsta->txrate.bw;
		sinfo->txrate.he_gi = arsta->txrate.he_gi;
		sinfo->txrate.he_dcm = arsta->txrate.he_dcm;
		sinfo->txrate.he_ru_alloc = arsta->txrate.he_ru_alloc;
	}
	sinfo->txrate.flags = arsta->txrate.flags;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BITRATE);

	/* TODO: Use real NF instead of default one. */
	sinfo->signal = arsta->rssi_comb + ATH12K_DEFAULT_NOISE_FLOOR;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_SIGNAL);
}

static int ath12k_mac_op_cancel_remain_on_channel(struct ieee80211_hw *hw,
						  struct ieee80211_vif *vif)
{
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar;

	ar = ath12k_ah_to_ar(ah, 0);

	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	ar->scan.roc_notify = false;
	spin_unlock_bh(&ar->data_lock);

	ath12k_scan_abort(ar);

	mutex_unlock(&ar->conf_mutex);

	cancel_delayed_work_sync(&ar->scan.timeout);

	return 0;
}

static int ath12k_mac_op_remain_on_channel(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif,
					   struct ieee80211_channel *chan,
					   int duration,
					   enum ieee80211_roc_type type)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k_wmi_scan_req_arg arg;
	struct ath12k *ar, *prev_ar;
	u32 scan_time_msec;
	bool create = true;
	int ret;

	if (ah->num_radio == 1) {
		WARN_ON(!arvif->is_created);
		ar = ath12k_ah_to_ar(ah, 0);
		goto scan;
	}

	ar = ath12k_mac_select_scan_device(hw, vif, chan->center_freq);
	if (!ar)
		return -EINVAL;

	/* If the vif is already assigned to a specific vdev of an ar,
	 * check whether its already started, vdev which is started
	 * are not allowed to switch to a new radio.
	 * If the vdev is not started, but was earlier created on a
	 * different ar, delete that vdev and create a new one. We don't
	 * delete at the scan stop as an optimization to avoid redundant
	 * delete-create vdev's for the same ar, in case the request is
	 * always on the same band for the vif
	 */
	if (arvif->is_created) {
		if (WARN_ON(!arvif->ar))
			return -EINVAL;

		if (ar != arvif->ar && arvif->is_started)
			return -EBUSY;

		if (ar != arvif->ar) {
			/* backup the previously used ar ptr, since the vdev delete
			 * would assign the arvif->ar to NULL after the call
			 */
			prev_ar = arvif->ar;
			mutex_lock(&prev_ar->conf_mutex);
			ret = ath12k_mac_vdev_delete(prev_ar, vif);
			mutex_unlock(&prev_ar->conf_mutex);
			if (ret) {
				ath12k_warn(prev_ar->ab,
					    "unable to delete scan vdev for roc: %d\n",
					    ret);
				return ret;
			}
		} else {
			create = false;
		}
	}

	if (create) {
		mutex_lock(&ar->conf_mutex);
		ret = ath12k_mac_vdev_create(ar, vif);
		mutex_unlock(&ar->conf_mutex);
		if (ret) {
			ath12k_warn(ar->ab, "unable to create scan vdev for roc: %d\n",
				    ret);
			return -EINVAL;
		}
	}

scan:
	mutex_lock(&ar->conf_mutex);
	spin_lock_bh(&ar->data_lock);

	switch (ar->scan.state) {
	case ATH12K_SCAN_IDLE:
		reinit_completion(&ar->scan.started);
		reinit_completion(&ar->scan.completed);
		reinit_completion(&ar->scan.on_channel);
		ar->scan.state = ATH12K_SCAN_STARTING;
		ar->scan.is_roc = true;
		ar->scan.vdev_id = arvif->vdev_id;
		ar->scan.roc_freq = chan->center_freq;
		ar->scan.roc_notify = true;
		ret = 0;
		break;
	case ATH12K_SCAN_STARTING:
	case ATH12K_SCAN_RUNNING:
	case ATH12K_SCAN_ABORTING:
		ret = -EBUSY;
		break;
	}

	spin_unlock_bh(&ar->data_lock);

	if (ret)
		goto exit;

	scan_time_msec = hw->wiphy->max_remain_on_channel_duration * 2;

	memset(&arg, 0, sizeof(arg));
	ath12k_wmi_start_scan_init(ar, &arg);
	arg.num_chan = 1;
	arg.chan_list = kcalloc(arg.num_chan, sizeof(*arg.chan_list),
				GFP_KERNEL);
	if (!arg.chan_list) {
		ret = -ENOMEM;
		goto exit;
	}

	arg.vdev_id = arvif->vdev_id;
	arg.scan_id = ATH12K_SCAN_ID;
	arg.chan_list[0] = chan->center_freq;
	arg.dwell_time_active = scan_time_msec;
	arg.dwell_time_passive = scan_time_msec;
	arg.max_scan_time = scan_time_msec;
	arg.scan_f_passive = 1;
	arg.burst_duration = duration;

	ret = ath12k_start_scan(ar, &arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to start roc scan: %d\n", ret);

		spin_lock_bh(&ar->data_lock);
		ar->scan.state = ATH12K_SCAN_IDLE;
		spin_unlock_bh(&ar->data_lock);
		goto free_chan_list;
	}

	ret = wait_for_completion_timeout(&ar->scan.on_channel, 3 * HZ);
	if (ret == 0) {
		ath12k_warn(ar->ab, "failed to switch to channel for roc scan\n");
		ret = ath12k_scan_stop(ar);
		if (ret)
			ath12k_warn(ar->ab, "failed to stop scan: %d\n", ret);
		ret = -ETIMEDOUT;
		goto free_chan_list;
	}

	ieee80211_queue_delayed_work(hw, &ar->scan.timeout,
				     msecs_to_jiffies(duration));

	ret = 0;

free_chan_list:
	kfree(arg.chan_list);
exit:
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void ath12k_mac_op_set_rekey_data(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 struct cfg80211_gtk_rekey_data *data)
{
	struct ath12k_vif *arvif = ath12k_vif_to_arvif(vif);
	struct ath12k_rekey_data *rekey_data = &arvif->rekey_data;
	struct ath12k_hw *ah = ath12k_hw_to_ah(hw);
	struct ath12k *ar = ath12k_ah_to_ar(ah, 0);

	ath12k_dbg(ar->ab, ATH12K_DBG_MAC, "mac set rekey data vdev %d\n",
		   arvif->vdev_id);

	mutex_lock(&ar->conf_mutex);

	memcpy(rekey_data->kck, data->kck, NL80211_KCK_LEN);
	memcpy(rekey_data->kek, data->kek, NL80211_KEK_LEN);

	/* The supplicant works on big-endian, the firmware expects it on
	 * little endian.
	 */
	rekey_data->replay_ctr = get_unaligned_be64(data->replay_ctr);

	arvif->rekey_data.enable_offload = true;

	ath12k_dbg_dump(ar->ab, ATH12K_DBG_MAC, "kck", NULL,
			rekey_data->kck, NL80211_KCK_LEN);
	ath12k_dbg_dump(ar->ab, ATH12K_DBG_MAC, "kek", NULL,
			rekey_data->kck, NL80211_KEK_LEN);
	ath12k_dbg_dump(ar->ab, ATH12K_DBG_MAC, "replay ctr", NULL,
			&rekey_data->replay_ctr, sizeof(rekey_data->replay_ctr));

	mutex_unlock(&ar->conf_mutex);
}

static const struct ieee80211_ops ath12k_ops = {
	.tx				= ath12k_mac_op_tx,
	.wake_tx_queue			= ieee80211_handle_wake_tx_queue,
	.start                          = ath12k_mac_op_start,
	.stop                           = ath12k_mac_op_stop,
	.reconfig_complete              = ath12k_mac_op_reconfig_complete,
	.add_interface                  = ath12k_mac_op_add_interface,
	.remove_interface		= ath12k_mac_op_remove_interface,
	.update_vif_offload		= ath12k_mac_op_update_vif_offload,
	.config                         = ath12k_mac_op_config,
	.bss_info_changed               = ath12k_mac_op_bss_info_changed,
	.configure_filter		= ath12k_mac_op_configure_filter,
	.hw_scan                        = ath12k_mac_op_hw_scan,
	.cancel_hw_scan                 = ath12k_mac_op_cancel_hw_scan,
	.set_key                        = ath12k_mac_op_set_key,
	.set_rekey_data	                = ath12k_mac_op_set_rekey_data,
	.sta_state                      = ath12k_mac_op_sta_state,
	.sta_set_txpwr			= ath12k_mac_op_sta_set_txpwr,
	.sta_rc_update			= ath12k_mac_op_sta_rc_update,
	.conf_tx                        = ath12k_mac_op_conf_tx,
	.set_antenna			= ath12k_mac_op_set_antenna,
	.get_antenna			= ath12k_mac_op_get_antenna,
	.ampdu_action			= ath12k_mac_op_ampdu_action,
	.add_chanctx			= ath12k_mac_op_add_chanctx,
	.remove_chanctx			= ath12k_mac_op_remove_chanctx,
	.change_chanctx			= ath12k_mac_op_change_chanctx,
	.assign_vif_chanctx		= ath12k_mac_op_assign_vif_chanctx,
	.unassign_vif_chanctx		= ath12k_mac_op_unassign_vif_chanctx,
	.switch_vif_chanctx		= ath12k_mac_op_switch_vif_chanctx,
	.set_rts_threshold		= ath12k_mac_op_set_rts_threshold,
	.set_frag_threshold		= ath12k_mac_op_set_frag_threshold,
	.set_bitrate_mask		= ath12k_mac_op_set_bitrate_mask,
	.get_survey			= ath12k_mac_op_get_survey,
	.flush				= ath12k_mac_op_flush,
	.sta_statistics			= ath12k_mac_op_sta_statistics,
	.remain_on_channel              = ath12k_mac_op_remain_on_channel,
	.cancel_remain_on_channel       = ath12k_mac_op_cancel_remain_on_channel,

#ifdef CONFIG_PM
	.suspend			= ath12k_wow_op_suspend,
	.resume				= ath12k_wow_op_resume,
	.set_wakeup			= ath12k_wow_op_set_wakeup,
#endif
};

static void ath12k_mac_update_ch_list(struct ath12k *ar,
				      struct ieee80211_supported_band *band,
				      u32 freq_low, u32 freq_high)
{
	int i;

	if (!(freq_low && freq_high))
		return;

	for (i = 0; i < band->n_channels; i++) {
		if (band->channels[i].center_freq < freq_low ||
		    band->channels[i].center_freq > freq_high)
			band->channels[i].flags |= IEEE80211_CHAN_DISABLED;
	}

	ar->freq_low = freq_low;
	ar->freq_high = freq_high;
}

static u32 ath12k_get_phy_id(struct ath12k *ar, u32 band)
{
	struct ath12k_pdev *pdev = ar->pdev;
	struct ath12k_pdev_cap *pdev_cap = &pdev->cap;

	if (band == WMI_HOST_WLAN_2G_CAP)
		return pdev_cap->band[NL80211_BAND_2GHZ].phy_id;

	if (band == WMI_HOST_WLAN_5G_CAP)
		return pdev_cap->band[NL80211_BAND_5GHZ].phy_id;

	ath12k_warn(ar->ab, "unsupported phy cap:%d\n", band);

	return 0;
}

static int ath12k_mac_setup_channels_rates(struct ath12k *ar,
					   u32 supported_bands,
					   struct ieee80211_supported_band *bands[])
{
	struct ieee80211_supported_band *band;
	struct ath12k_wmi_hal_reg_capabilities_ext_arg *reg_cap;
	struct ath12k_hw *ah = ar->ah;
	void *channels;
	u32 phy_id;

	BUILD_BUG_ON((ARRAY_SIZE(ath12k_2ghz_channels) +
		      ARRAY_SIZE(ath12k_5ghz_channels) +
		      ARRAY_SIZE(ath12k_6ghz_channels)) !=
		     ATH12K_NUM_CHANS);

	reg_cap = &ar->ab->hal_reg_cap[ar->pdev_idx];

	if (supported_bands & WMI_HOST_WLAN_2G_CAP) {
		channels = kmemdup(ath12k_2ghz_channels,
				   sizeof(ath12k_2ghz_channels),
				   GFP_KERNEL);
		if (!channels)
			return -ENOMEM;

		band = &ar->mac.sbands[NL80211_BAND_2GHZ];
		band->band = NL80211_BAND_2GHZ;
		band->n_channels = ARRAY_SIZE(ath12k_2ghz_channels);
		band->channels = channels;
		band->n_bitrates = ath12k_g_rates_size;
		band->bitrates = ath12k_g_rates;
		bands[NL80211_BAND_2GHZ] = band;

		if (ar->ab->hw_params->single_pdev_only) {
			phy_id = ath12k_get_phy_id(ar, WMI_HOST_WLAN_2G_CAP);
			reg_cap = &ar->ab->hal_reg_cap[phy_id];
		}
		ath12k_mac_update_ch_list(ar, band,
					  reg_cap->low_2ghz_chan,
					  reg_cap->high_2ghz_chan);
	}

	if (supported_bands & WMI_HOST_WLAN_5G_CAP) {
		if (reg_cap->high_5ghz_chan >= ATH12K_MIN_6G_FREQ) {
			channels = kmemdup(ath12k_6ghz_channels,
					   sizeof(ath12k_6ghz_channels), GFP_KERNEL);
			if (!channels) {
				kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
				return -ENOMEM;
			}

			ar->supports_6ghz = true;
			band = &ar->mac.sbands[NL80211_BAND_6GHZ];
			band->band = NL80211_BAND_6GHZ;
			band->n_channels = ARRAY_SIZE(ath12k_6ghz_channels);
			band->channels = channels;
			band->n_bitrates = ath12k_a_rates_size;
			band->bitrates = ath12k_a_rates;
			bands[NL80211_BAND_6GHZ] = band;
			ath12k_mac_update_ch_list(ar, band,
						  reg_cap->low_5ghz_chan,
						  reg_cap->high_5ghz_chan);
			ah->use_6ghz_regd = true;
		}

		if (reg_cap->low_5ghz_chan < ATH12K_MIN_6G_FREQ) {
			channels = kmemdup(ath12k_5ghz_channels,
					   sizeof(ath12k_5ghz_channels),
					   GFP_KERNEL);
			if (!channels) {
				kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
				kfree(ar->mac.sbands[NL80211_BAND_6GHZ].channels);
				return -ENOMEM;
			}

			band = &ar->mac.sbands[NL80211_BAND_5GHZ];
			band->band = NL80211_BAND_5GHZ;
			band->n_channels = ARRAY_SIZE(ath12k_5ghz_channels);
			band->channels = channels;
			band->n_bitrates = ath12k_a_rates_size;
			band->bitrates = ath12k_a_rates;
			bands[NL80211_BAND_5GHZ] = band;

			if (ar->ab->hw_params->single_pdev_only) {
				phy_id = ath12k_get_phy_id(ar, WMI_HOST_WLAN_5G_CAP);
				reg_cap = &ar->ab->hal_reg_cap[phy_id];
			}

			ath12k_mac_update_ch_list(ar, band,
						  reg_cap->low_5ghz_chan,
						  reg_cap->high_5ghz_chan);
		}
	}

	return 0;
}

static u16 ath12k_mac_get_ifmodes(struct ath12k_hw *ah)
{
	struct ath12k *ar;
	int i;
	u16 interface_modes = U16_MAX;

	for_each_ar(ah, ar, i)
		interface_modes &= ar->ab->hw_params->interface_modes;

	return interface_modes == U16_MAX ? 0 : interface_modes;
}

static bool ath12k_mac_is_iface_mode_enable(struct ath12k_hw *ah,
					    enum nl80211_iftype type)
{
	struct ath12k *ar;
	int i;
	u16 interface_modes, mode;
	bool is_enable = true;

	mode = BIT(type);
	for_each_ar(ah, ar, i) {
		interface_modes = ar->ab->hw_params->interface_modes;
		if (!(interface_modes & mode)) {
			is_enable = false;
			break;
		}
	}

	return is_enable;
}

static int ath12k_mac_setup_iface_combinations(struct ath12k_hw *ah)
{
	struct wiphy *wiphy = ah->hw->wiphy;
	struct ieee80211_iface_combination *combinations;
	struct ieee80211_iface_limit *limits;
	int n_limits, max_interfaces;
	bool ap, mesh, p2p;

	ap = ath12k_mac_is_iface_mode_enable(ah, NL80211_IFTYPE_AP);
	p2p = ath12k_mac_is_iface_mode_enable(ah, NL80211_IFTYPE_P2P_DEVICE);

	mesh = IS_ENABLED(CPTCFG_MAC80211_MESH) &&
		ath12k_mac_is_iface_mode_enable(ah, NL80211_IFTYPE_MESH_POINT);

	combinations = kzalloc(sizeof(*combinations), GFP_KERNEL);
	if (!combinations)
		return -ENOMEM;

	if ((ap || mesh) && !p2p) {
		n_limits = 2;
		max_interfaces = 16;
	} else if (p2p) {
		n_limits = 3;
		if (ap || mesh)
			max_interfaces = 16;
		else
			max_interfaces = 3;
	} else {
		n_limits = 1;
		max_interfaces = 1;
	}

	limits = kcalloc(n_limits, sizeof(*limits), GFP_KERNEL);
	if (!limits) {
		kfree(combinations);
		return -ENOMEM;
	}

	limits[0].max = 1;
	limits[0].types |= BIT(NL80211_IFTYPE_STATION);

	if (ap || mesh || p2p)
		limits[1].max = max_interfaces;

	if (ap)
		limits[1].types |= BIT(NL80211_IFTYPE_AP);

	if (mesh)
		limits[1].types |= BIT(NL80211_IFTYPE_MESH_POINT);

	if (p2p) {
		limits[1].types |= BIT(NL80211_IFTYPE_P2P_CLIENT) |
				   BIT(NL80211_IFTYPE_P2P_GO);
		limits[2].max = 1;
		limits[2].types |= BIT(NL80211_IFTYPE_P2P_DEVICE);
	}

	combinations[0].limits = limits;
	combinations[0].n_limits = n_limits;
	combinations[0].max_interfaces = max_interfaces;
	combinations[0].num_different_channels = 1;
	combinations[0].beacon_int_infra_match = true;
	combinations[0].beacon_int_min_gcd = 100;
	combinations[0].radar_detect_widths = BIT(NL80211_CHAN_WIDTH_20_NOHT) |
						BIT(NL80211_CHAN_WIDTH_20) |
						BIT(NL80211_CHAN_WIDTH_40) |
						BIT(NL80211_CHAN_WIDTH_80);

	wiphy->iface_combinations = combinations;
	wiphy->n_iface_combinations = 1;

	return 0;
}

static const u8 ath12k_if_types_ext_capa[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[2] = WLAN_EXT_CAPA3_MULTI_BSSID_SUPPORT,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
};

static const u8 ath12k_if_types_ext_capa_sta[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[2] = WLAN_EXT_CAPA3_MULTI_BSSID_SUPPORT,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
	[9] = WLAN_EXT_CAPA10_TWT_REQUESTER_SUPPORT,
};

static const u8 ath12k_if_types_ext_capa_ap[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[2] = WLAN_EXT_CAPA3_MULTI_BSSID_SUPPORT,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
	[9] = WLAN_EXT_CAPA10_TWT_RESPONDER_SUPPORT,
	[10] = WLAN_EXT_CAPA11_EMA_SUPPORT,
};

static const struct wiphy_iftype_ext_capab ath12k_iftypes_ext_capa[] = {
	{
		.extended_capabilities = ath12k_if_types_ext_capa,
		.extended_capabilities_mask = ath12k_if_types_ext_capa,
		.extended_capabilities_len = sizeof(ath12k_if_types_ext_capa),
	}, {
		.iftype = NL80211_IFTYPE_STATION,
		.extended_capabilities = ath12k_if_types_ext_capa_sta,
		.extended_capabilities_mask = ath12k_if_types_ext_capa_sta,
		.extended_capabilities_len =
				sizeof(ath12k_if_types_ext_capa_sta),
	}, {
		.iftype = NL80211_IFTYPE_AP,
		.extended_capabilities = ath12k_if_types_ext_capa_ap,
		.extended_capabilities_mask = ath12k_if_types_ext_capa_ap,
		.extended_capabilities_len =
				sizeof(ath12k_if_types_ext_capa_ap),
	},
};

static void ath12k_mac_cleanup_unregister(struct ath12k *ar)
{
	idr_for_each(&ar->txmgmt_idr, ath12k_mac_tx_mgmt_pending_free, ar);
	idr_destroy(&ar->txmgmt_idr);

	kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_5GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_6GHZ].channels);
}

static void ath12k_mac_hw_unregister(struct ath12k_hw *ah)
{
	struct ieee80211_hw *hw = ah->hw;
	struct wiphy *wiphy = hw->wiphy;
	struct ath12k *ar;
	int i;

	for_each_ar(ah, ar, i) {
		cancel_work_sync(&ar->regd_update_work);
		ath12k_debugfs_unregister(ar);
	}

	ieee80211_unregister_hw(hw);

	for_each_ar(ah, ar, i)
		ath12k_mac_cleanup_unregister(ar);

	kfree(wiphy->iface_combinations[0].limits);
	kfree(wiphy->iface_combinations);

	SET_IEEE80211_DEV(hw, NULL);
}

static int ath12k_mac_setup_register(struct ath12k *ar,
				     u32 *ht_cap,
				     struct ieee80211_supported_band *bands[])
{
	struct ath12k_pdev_cap *cap = &ar->pdev->cap;
	int ret;

	init_waitqueue_head(&ar->txmgmt_empty_waitq);
	idr_init(&ar->txmgmt_idr);
	spin_lock_init(&ar->txmgmt_idr_lock);

	ath12k_pdev_caps_update(ar);

	ret = ath12k_mac_setup_channels_rates(ar,
					      cap->supported_bands,
					      bands);
	if (ret)
		return ret;

	ath12k_mac_setup_ht_vht_cap(ar, cap, ht_cap);
	ath12k_mac_setup_sband_iftype_data(ar, cap);

	ar->max_num_stations = ath12k_core_get_max_station_per_radio(ar->ab);
	ar->max_num_peers = ath12k_core_get_max_peers_per_radio(ar->ab);

	return 0;
}

static int ath12k_mac_hw_register(struct ath12k_hw *ah)
{
	struct ieee80211_hw *hw = ah->hw;
	struct wiphy *wiphy = hw->wiphy;
	struct ath12k *ar = ath12k_ah_to_ar(ah, 0);
	struct ath12k_base *ab = ar->ab;
	struct ath12k_pdev *pdev;
	struct ath12k_pdev_cap *cap;
	static const u32 cipher_suites[] = {
		WLAN_CIPHER_SUITE_TKIP,
		WLAN_CIPHER_SUITE_CCMP,
		WLAN_CIPHER_SUITE_AES_CMAC,
		WLAN_CIPHER_SUITE_BIP_CMAC_256,
		WLAN_CIPHER_SUITE_BIP_GMAC_128,
		WLAN_CIPHER_SUITE_BIP_GMAC_256,
		WLAN_CIPHER_SUITE_GCMP,
		WLAN_CIPHER_SUITE_GCMP_256,
		WLAN_CIPHER_SUITE_CCMP_256,
	};
	int ret, i, j;
	u32 ht_cap = U32_MAX, antennas_rx = 0, antennas_tx = 0;
	bool is_6ghz = false, is_raw_mode = false, is_monitor_disable = false;
	u8 *mac_addr = NULL;
	u8 mbssid_max_interfaces = 0;

	wiphy->max_ap_assoc_sta = 0;

	for_each_ar(ah, ar, i) {
		u32 ht_cap_info = 0;

		pdev = ar->pdev;
		if (ar->ab->pdevs_macaddr_valid) {
			ether_addr_copy(ar->mac_addr, pdev->mac_addr);
		} else {
			ether_addr_copy(ar->mac_addr, ar->ab->mac_addr);
			ar->mac_addr[4] += ar->pdev_idx;
		}

		ret = ath12k_mac_setup_register(ar, &ht_cap_info, hw->wiphy->bands);
		if (ret)
			goto err_cleanup_unregister;

		ht_cap &= ht_cap_info;
		wiphy->max_ap_assoc_sta += ar->max_num_stations;

		/* Advertise the max antenna support of all radios, driver can handle
		 * per pdev specific antenna setting based on pdev cap when antenna
		 * changes are made
		 */
		cap = &pdev->cap;

		antennas_rx = max_t(u32, antennas_rx, cap->rx_chain_mask);
		antennas_tx = max_t(u32, antennas_tx, cap->tx_chain_mask);

		if (ar->supports_6ghz)
			is_6ghz = true;

		if (test_bit(ATH12K_FLAG_RAW_MODE, &ar->ab->dev_flags))
			is_raw_mode = true;

		if (!ar->ab->hw_params->supports_monitor)
			is_monitor_disable = true;

		if (i == 0)
			mac_addr = ar->mac_addr;
		else
			mac_addr = ab->mac_addr;

		mbssid_max_interfaces += TARGET_NUM_VDEVS;
	}

	wiphy->available_antennas_rx = antennas_rx;
	wiphy->available_antennas_tx = antennas_tx;

	SET_IEEE80211_PERM_ADDR(hw, mac_addr);
	SET_IEEE80211_DEV(hw, ab->dev);

	ret = ath12k_mac_setup_iface_combinations(ah);
	if (ret) {
		ath12k_err(ab, "failed to setup interface combinations: %d\n", ret);
		goto err_complete_cleanup_unregister;
	}

	wiphy->interface_modes = ath12k_mac_get_ifmodes(ah);

	if (ah->num_radio == 1 &&
	    wiphy->bands[NL80211_BAND_2GHZ] &&
	    wiphy->bands[NL80211_BAND_5GHZ] &&
	    wiphy->bands[NL80211_BAND_6GHZ])
		ieee80211_hw_set(hw, SINGLE_SCAN_ON_ALL_BANDS);

	ieee80211_hw_set(hw, SIGNAL_DBM);
	ieee80211_hw_set(hw, SUPPORTS_PS);
	ieee80211_hw_set(hw, SUPPORTS_DYNAMIC_PS);
	ieee80211_hw_set(hw, MFP_CAPABLE);
	ieee80211_hw_set(hw, REPORTS_TX_ACK_STATUS);
	ieee80211_hw_set(hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(hw, AP_LINK_PS);
	ieee80211_hw_set(hw, SPECTRUM_MGMT);
	ieee80211_hw_set(hw, CONNECTION_MONITOR);
	ieee80211_hw_set(hw, SUPPORTS_PER_STA_GTK);
	ieee80211_hw_set(hw, CHANCTX_STA_CSA);
	ieee80211_hw_set(hw, QUEUE_CONTROL);
	ieee80211_hw_set(hw, SUPPORTS_TX_FRAG);
	ieee80211_hw_set(hw, REPORTS_LOW_ACK);

	if ((ht_cap & WMI_HT_CAP_ENABLED) || ar->supports_6ghz) {
		ieee80211_hw_set(hw, AMPDU_AGGREGATION);
		ieee80211_hw_set(hw, TX_AMPDU_SETUP_IN_HW);
		ieee80211_hw_set(hw, SUPPORTS_REORDERING_BUFFER);
		ieee80211_hw_set(hw, SUPPORTS_AMSDU_IN_AMPDU);
		ieee80211_hw_set(hw, USES_RSS);
	}

	wiphy->features |= NL80211_FEATURE_STATIC_SMPS;
	wiphy->flags |= WIPHY_FLAG_IBSS_RSN;

	/* TODO: Check if HT capability advertised from firmware is different
	 * for each band for a dual band capable radio. It will be tricky to
	 * handle it when the ht capability different for each band.
	 */
	if (ht_cap & WMI_HT_CAP_DYNAMIC_SMPS ||
	    (ar->supports_6ghz && ab->hw_params->supports_dynamic_smps_6ghz))
		wiphy->features |= NL80211_FEATURE_DYNAMIC_SMPS;

	wiphy->max_scan_ssids = WLAN_SCAN_PARAMS_MAX_SSID;
	wiphy->max_scan_ie_len = WLAN_SCAN_PARAMS_MAX_IE_LEN;

	hw->max_listen_interval = ATH12K_MAX_HW_LISTEN_INTERVAL;

	wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	wiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
	wiphy->max_remain_on_channel_duration = 5000;

	wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE |
				   NL80211_FEATURE_AP_SCAN;

	/* MLO is not yet supported so disable Wireless Extensions for now
	 * to make sure ath12k users don't use it. This flag can be removed
	 * once WIPHY_FLAG_SUPPORTS_MLO is enabled.
	 */
	wiphy->flags |= WIPHY_FLAG_DISABLE_WEXT;

	hw->queues = ATH12K_HW_MAX_QUEUES;
	wiphy->tx_queue_len = ATH12K_QUEUE_LEN;
	hw->offchannel_tx_hw_queue = ATH12K_HW_MAX_QUEUES - 1;
	hw->max_rx_aggregation_subframes = IEEE80211_MAX_AMPDU_BUF_EHT;

	hw->vif_data_size = sizeof(struct ath12k_vif);
	hw->sta_data_size = sizeof(struct ath12k_sta);
	hw->extra_tx_headroom = ab->hw_params->iova_mask;

	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_CQM_RSSI_LIST);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_STA_TX_PWR);

	wiphy->cipher_suites = cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);

	wiphy->iftype_ext_capab = ath12k_iftypes_ext_capa;
	wiphy->num_iftype_ext_capab = ARRAY_SIZE(ath12k_iftypes_ext_capa);

	wiphy->mbssid_max_interfaces = mbssid_max_interfaces;
	wiphy->ema_max_profile_periodicity = TARGET_EMA_MAX_PROFILE_PERIOD;

	if (is_6ghz) {
		wiphy_ext_feature_set(wiphy,
				      NL80211_EXT_FEATURE_FILS_DISCOVERY);
		wiphy_ext_feature_set(wiphy,
				      NL80211_EXT_FEATURE_UNSOL_BCAST_PROBE_RESP);
	}

	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_PUNCT);

	ath12k_reg_init(hw);

	if (!is_raw_mode) {
		hw->netdev_features = NETIF_F_HW_CSUM;
		ieee80211_hw_set(hw, SW_CRYPTO_CONTROL);
		ieee80211_hw_set(hw, SUPPORT_FAST_XMIT);
	}

	if (test_bit(WMI_TLV_SERVICE_NLO, ar->wmi->wmi_ab->svc_map)) {
		wiphy->max_sched_scan_ssids = WMI_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_match_sets = WMI_PNO_MAX_SUPP_NETWORKS;
		wiphy->max_sched_scan_ie_len = WMI_PNO_MAX_IE_LENGTH;
		wiphy->max_sched_scan_plans = WMI_PNO_MAX_SCHED_SCAN_PLANS;
		wiphy->max_sched_scan_plan_interval =
					WMI_PNO_MAX_SCHED_SCAN_PLAN_INT;
		wiphy->max_sched_scan_plan_iterations =
					WMI_PNO_MAX_SCHED_SCAN_PLAN_ITRNS;
		wiphy->features |= NL80211_FEATURE_ND_RANDOM_MAC_ADDR;
	}

	ret = ath12k_wow_init(ar);
	if (ret) {
		ath12k_warn(ar->ab, "failed to init wow: %d\n", ret);
		goto err_free_if_combs;
	}

	ret = ieee80211_register_hw(hw);
	if (ret) {
		ath12k_err(ab, "ieee80211 registration failed: %d\n", ret);
		goto err_free_if_combs;
	}

	if (is_monitor_disable)
		/* There's a race between calling ieee80211_register_hw()
		 * and here where the monitor mode is enabled for a little
		 * while. But that time is so short and in practise it make
		 * a difference in real life.
		 */
		wiphy->interface_modes &= ~BIT(NL80211_IFTYPE_MONITOR);

	for_each_ar(ah, ar, i) {
		/* Apply the regd received during initialization */
		ret = ath12k_regd_update(ar, true);
		if (ret) {
			ath12k_err(ar->ab, "ath12k regd update failed: %d\n", ret);
			goto err_unregister_hw;
		}

		ath12k_debugfs_register(ar);
	}

	return 0;

err_unregister_hw:
	for_each_ar(ah, ar, i)
		ath12k_debugfs_unregister(ar);

	ieee80211_unregister_hw(hw);

err_free_if_combs:
	kfree(wiphy->iface_combinations[0].limits);
	kfree(wiphy->iface_combinations);

err_complete_cleanup_unregister:
	i = ah->num_radio;

err_cleanup_unregister:
	for (j = 0; j < i; j++) {
		ar = ath12k_ah_to_ar(ah, j);
		ath12k_mac_cleanup_unregister(ar);
	}

	SET_IEEE80211_DEV(hw, NULL);

	return ret;
}

static void ath12k_mac_setup(struct ath12k *ar)
{
	struct ath12k_base *ab = ar->ab;
	struct ath12k_pdev *pdev = ar->pdev;
	u8 pdev_idx = ar->pdev_idx;

	ar->lmac_id = ath12k_hw_get_mac_from_pdev_id(ab->hw_params, pdev_idx);

	ar->wmi = &ab->wmi_ab.wmi[pdev_idx];
	/* FIXME: wmi[0] is already initialized during attach,
	 * Should we do this again?
	 */
	ath12k_wmi_pdev_attach(ab, pdev_idx);

	ar->cfg_tx_chainmask = pdev->cap.tx_chain_mask;
	ar->cfg_rx_chainmask = pdev->cap.rx_chain_mask;
	ar->num_tx_chains = hweight32(pdev->cap.tx_chain_mask);
	ar->num_rx_chains = hweight32(pdev->cap.rx_chain_mask);

	spin_lock_init(&ar->data_lock);
	INIT_LIST_HEAD(&ar->arvifs);
	INIT_LIST_HEAD(&ar->ppdu_stats_info);
	mutex_init(&ar->conf_mutex);
	init_completion(&ar->vdev_setup_done);
	init_completion(&ar->vdev_delete_done);
	init_completion(&ar->peer_assoc_done);
	init_completion(&ar->peer_delete_done);
	init_completion(&ar->install_key_done);
	init_completion(&ar->bss_survey_done);
	init_completion(&ar->scan.started);
	init_completion(&ar->scan.completed);
	init_completion(&ar->scan.on_channel);

	INIT_DELAYED_WORK(&ar->scan.timeout, ath12k_scan_timeout_work);
	INIT_WORK(&ar->regd_update_work, ath12k_regd_update_work);

	INIT_WORK(&ar->wmi_mgmt_tx_work, ath12k_mgmt_over_wmi_tx_work);
	skb_queue_head_init(&ar->wmi_mgmt_tx_queue);
}

int ath12k_mac_register(struct ath12k_base *ab)
{
	struct ath12k_hw *ah;
	int i;
	int ret;

	if (test_bit(ATH12K_FLAG_REGISTERED, &ab->dev_flags))
		return 0;

	/* Initialize channel counters frequency value in hertz */
	ab->cc_freq_hz = 320000;
	ab->free_vdev_map = (1LL << (ab->num_radios * TARGET_NUM_VDEVS)) - 1;

	for (i = 0; i < ab->num_hw; i++) {
		ah = ab->ah[i];

		ret = ath12k_mac_hw_register(ah);
		if (ret)
			goto err;
	}

	return 0;

err:
	for (i = i - 1; i >= 0; i--) {
		ah = ab->ah[i];
		if (!ah)
			continue;

		ath12k_mac_hw_unregister(ah);
	}

	return ret;
}

void ath12k_mac_unregister(struct ath12k_base *ab)
{
	struct ath12k_hw *ah;
	int i;

	for (i = ab->num_hw - 1; i >= 0; i--) {
		ah = ab->ah[i];
		if (!ah)
			continue;

		ath12k_mac_hw_unregister(ah);
	}
}

static void ath12k_mac_hw_destroy(struct ath12k_hw *ah)
{
	ieee80211_free_hw(ah->hw);
}

static struct ath12k_hw *ath12k_mac_hw_allocate(struct ath12k_base *ab,
						struct ath12k_pdev_map *pdev_map,
						u8 num_pdev_map)
{
	struct ieee80211_hw *hw;
	struct ath12k *ar;
	struct ath12k_pdev *pdev;
	struct ath12k_hw *ah;
	int i;
	u8 pdev_idx;

	hw = ieee80211_alloc_hw(struct_size(ah, radio, num_pdev_map),
				&ath12k_ops);
	if (!hw)
		return NULL;

	ah = ath12k_hw_to_ah(hw);
	ah->hw = hw;
	ah->num_radio = num_pdev_map;

	mutex_init(&ah->hw_mutex);

	for (i = 0; i < num_pdev_map; i++) {
		ab = pdev_map[i].ab;
		pdev_idx = pdev_map[i].pdev_idx;
		pdev = &ab->pdevs[pdev_idx];

		ar = ath12k_ah_to_ar(ah, i);
		ar->ah = ah;
		ar->ab = ab;
		ar->hw_link_id = pdev->hw_link_id;
		ar->pdev = pdev;
		ar->pdev_idx = pdev_idx;
		pdev->ar = ar;

		ath12k_mac_setup(ar);
	}

	return ah;
}

void ath12k_mac_destroy(struct ath12k_base *ab)
{
	struct ath12k_pdev *pdev;
	int i;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = &ab->pdevs[i];
		if (!pdev->ar)
			continue;

		pdev->ar = NULL;
	}

	for (i = 0; i < ab->num_hw; i++) {
		if (!ab->ah[i])
			continue;

		ath12k_mac_hw_destroy(ab->ah[i]);
		ab->ah[i] = NULL;
	}
}

int ath12k_mac_allocate(struct ath12k_base *ab)
{
	struct ath12k_hw *ah;
	struct ath12k_pdev_map pdev_map[MAX_RADIOS];
	int ret, i, j;
	u8 radio_per_hw;

	if (test_bit(ATH12K_FLAG_REGISTERED, &ab->dev_flags))
		return 0;

	ab->num_hw = ab->num_radios;
	radio_per_hw = 1;

	for (i = 0; i < ab->num_hw; i++) {
		for (j = 0; j < radio_per_hw; j++) {
			pdev_map[j].ab = ab;
			pdev_map[j].pdev_idx = (i * radio_per_hw) + j;
		}

		ah = ath12k_mac_hw_allocate(ab, pdev_map, radio_per_hw);
		if (!ah) {
			ath12k_warn(ab, "failed to allocate mac80211 hw device for hw_idx %d\n",
				    i);
			ret = -ENOMEM;
			goto err;
		}

		ab->ah[i] = ah;
	}

	ath12k_dp_pdev_pre_alloc(ab);

	return 0;

err:
	for (i = i - 1; i >= 0; i--) {
		if (!ab->ah[i])
			continue;

		ath12k_mac_hw_destroy(ab->ah[i]);
		ab->ah[i] = NULL;
	}

	return ret;
}

int ath12k_mac_vif_set_keepalive(struct ath12k_vif *arvif,
				 enum wmi_sta_keepalive_method method,
				 u32 interval)
{
	struct wmi_sta_keepalive_arg arg = {};
	struct ath12k *ar = arvif->ar;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (arvif->vdev_type != WMI_VDEV_TYPE_STA)
		return 0;

	if (!test_bit(WMI_TLV_SERVICE_STA_KEEP_ALIVE, ar->ab->wmi_ab.svc_map))
		return 0;

	arg.vdev_id = arvif->vdev_id;
	arg.enabled = 1;
	arg.method = method;
	arg.interval = interval;

	ret = ath12k_wmi_sta_keepalive(ar, &arg);
	if (ret) {
		ath12k_warn(ar->ab, "failed to set keepalive on vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	return 0;
}
