// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "core.h"
#include "dp_tx.h"
#include "debug.h"
#include "debugfs.h"
#include "debugfs_htt_stats.h"

static ssize_t ath12k_write_simulate_radar(struct file *file,
					   const char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	struct ath12k *ar = file->private_data;
	int ret;

	wiphy_lock(ath12k_ar_to_hw(ar)->wiphy);
	ret = ath12k_wmi_simulate_radar(ar);
	if (ret)
		goto exit;

	ret = count;
exit:
	wiphy_unlock(ath12k_ar_to_hw(ar)->wiphy);
	return ret;
}

static const struct file_operations fops_simulate_radar = {
	.write = ath12k_write_simulate_radar,
	.open = simple_open
};

static ssize_t ath12k_write_tpc_stats_type(struct file *file,
					   const char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	struct ath12k *ar = file->private_data;
	u8 type;
	int ret;

	ret = kstrtou8_from_user(user_buf, count, 0, &type);
	if (ret)
		return ret;

	if (type >= WMI_HALPHY_PDEV_TX_STATS_MAX)
		return -EINVAL;

	spin_lock_bh(&ar->data_lock);
	ar->debug.tpc_stats_type = type;
	spin_unlock_bh(&ar->data_lock);

	return count;
}

static int ath12k_debug_tpc_stats_request(struct ath12k *ar)
{
	enum wmi_halphy_ctrl_path_stats_id tpc_stats_sub_id;
	struct ath12k_base *ab = ar->ab;
	int ret;

	lockdep_assert_wiphy(ath12k_ar_to_hw(ar)->wiphy);

	reinit_completion(&ar->debug.tpc_complete);

	spin_lock_bh(&ar->data_lock);
	ar->debug.tpc_request = true;
	tpc_stats_sub_id = ar->debug.tpc_stats_type;
	spin_unlock_bh(&ar->data_lock);

	ret = ath12k_wmi_send_tpc_stats_request(ar, tpc_stats_sub_id);
	if (ret) {
		ath12k_warn(ab, "failed to request pdev tpc stats: %d\n", ret);
		spin_lock_bh(&ar->data_lock);
		ar->debug.tpc_request = false;
		spin_unlock_bh(&ar->data_lock);
		return ret;
	}

	return 0;
}

static int ath12k_get_tpc_ctl_mode_idx(struct wmi_tpc_stats_arg *tpc_stats,
				       enum wmi_tpc_pream_bw pream_bw, int *mode_idx)
{
	u32 chan_freq = le32_to_cpu(tpc_stats->tpc_config.chan_freq);
	u8 band;

	band = ((chan_freq > ATH12K_MIN_6G_FREQ) ? NL80211_BAND_6GHZ :
		((chan_freq > ATH12K_MIN_5G_FREQ) ? NL80211_BAND_5GHZ :
		NL80211_BAND_2GHZ));

	if (band == NL80211_BAND_5GHZ || band == NL80211_BAND_6GHZ) {
		switch (pream_bw) {
		case WMI_TPC_PREAM_HT20:
		case WMI_TPC_PREAM_VHT20:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HT_VHT20_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_HE20:
		case WMI_TPC_PREAM_EHT20:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HE_EHT20_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_HT40:
		case WMI_TPC_PREAM_VHT40:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HT_VHT40_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_HE40:
		case WMI_TPC_PREAM_EHT40:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HE_EHT40_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_VHT80:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_VHT80_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_EHT60:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_EHT80_SU_PUNC20;
			break;
		case WMI_TPC_PREAM_HE80:
		case WMI_TPC_PREAM_EHT80:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HE_EHT80_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_VHT160:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_VHT160_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_EHT120:
		case WMI_TPC_PREAM_EHT140:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_EHT160_SU_PUNC20;
			break;
		case WMI_TPC_PREAM_HE160:
		case WMI_TPC_PREAM_EHT160:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HE_EHT160_5GHZ_6GHZ;
			break;
		case WMI_TPC_PREAM_EHT200:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_EHT320_SU_PUNC120;
			break;
		case WMI_TPC_PREAM_EHT240:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_EHT320_SU_PUNC80;
			break;
		case WMI_TPC_PREAM_EHT280:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_EHT320_SU_PUNC40;
			break;
		case WMI_TPC_PREAM_EHT320:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HE_EHT320_5GHZ_6GHZ;
			break;
		default:
			/* for 5GHZ and 6GHZ, default case will be for OFDM */
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_LEGACY_5GHZ_6GHZ;
			break;
		}
	} else {
		switch (pream_bw) {
		case WMI_TPC_PREAM_OFDM:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_LEGACY_2GHZ;
			break;
		case WMI_TPC_PREAM_HT20:
		case WMI_TPC_PREAM_VHT20:
		case WMI_TPC_PREAM_HE20:
		case WMI_TPC_PREAM_EHT20:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HT20_2GHZ;
			break;
		case WMI_TPC_PREAM_HT40:
		case WMI_TPC_PREAM_VHT40:
		case WMI_TPC_PREAM_HE40:
		case WMI_TPC_PREAM_EHT40:
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_HT40_2GHZ;
			break;
		default:
			/* for 2GHZ, default case will be CCK */
			*mode_idx = ATH12K_TPC_STATS_CTL_MODE_CCK_2GHZ;
			break;
		}
	}

	return 0;
}

static s16 ath12k_tpc_get_rate(struct ath12k *ar,
			       struct wmi_tpc_stats_arg *tpc_stats,
			       u32 rate_idx, u32 num_chains, u32 rate_code,
			       enum wmi_tpc_pream_bw pream_bw,
			       enum wmi_halphy_ctrl_path_stats_id type,
			       u32 eht_rate_idx)
{
	u32 tot_nss, tot_modes, txbf_on_off, index_offset1, index_offset2, index_offset3;
	u8 chain_idx, stm_idx, num_streams;
	bool is_mu, txbf_enabled = 0;
	s8 rates_ctl_min, tpc_ctl;
	s16 rates, tpc, reg_pwr;
	u16 rate1, rate2;
	int mode, ret;

	num_streams = 1 + ATH12K_HW_NSS(rate_code);
	chain_idx = num_chains - 1;
	stm_idx = num_streams - 1;
	mode = -1;

	ret = ath12k_get_tpc_ctl_mode_idx(tpc_stats, pream_bw, &mode);
	if (ret) {
		ath12k_warn(ar->ab, "Invalid mode index received\n");
		tpc = TPC_INVAL;
		goto out;
	}

	if (num_chains < num_streams) {
		tpc = TPC_INVAL;
		goto out;
	}

	if (le32_to_cpu(tpc_stats->tpc_config.num_tx_chain) <= 1) {
		tpc = TPC_INVAL;
		goto out;
	}

	if (type == WMI_HALPHY_PDEV_TX_SUTXBF_STATS ||
	    type == WMI_HALPHY_PDEV_TX_MUTXBF_STATS)
		txbf_enabled = 1;

	if (type == WMI_HALPHY_PDEV_TX_MU_STATS ||
	    type == WMI_HALPHY_PDEV_TX_MUTXBF_STATS) {
		is_mu = true;
	} else {
		is_mu = false;
	}

	/* Below is the min calculation of ctl array, rates array and
	 * regulator power table. tpc is minimum of all 3
	 */
	if (pream_bw >= WMI_TPC_PREAM_EHT20 && pream_bw <= WMI_TPC_PREAM_EHT320) {
		rate2 = tpc_stats->rates_array2.rate_array[eht_rate_idx];
		if (is_mu)
			rates = u32_get_bits(rate2, ATH12K_TPC_RATE_ARRAY_MU);
		else
			rates = u32_get_bits(rate2, ATH12K_TPC_RATE_ARRAY_SU);
	} else {
		rate1 = tpc_stats->rates_array1.rate_array[rate_idx];
		if (is_mu)
			rates = u32_get_bits(rate1, ATH12K_TPC_RATE_ARRAY_MU);
		else
			rates = u32_get_bits(rate1, ATH12K_TPC_RATE_ARRAY_SU);
	}

	if (tpc_stats->tlvs_rcvd & WMI_TPC_CTL_PWR_ARRAY) {
		tot_nss = le32_to_cpu(tpc_stats->ctl_array.tpc_ctl_pwr.d1);
		tot_modes = le32_to_cpu(tpc_stats->ctl_array.tpc_ctl_pwr.d2);
		txbf_on_off = le32_to_cpu(tpc_stats->ctl_array.tpc_ctl_pwr.d3);
		index_offset1 = txbf_on_off * tot_modes * tot_nss;
		index_offset2 = tot_modes * tot_nss;
		index_offset3 = tot_nss;

		tpc_ctl = *(tpc_stats->ctl_array.ctl_pwr_table +
			    chain_idx * index_offset1 + txbf_enabled * index_offset2
			    + mode * index_offset3 + stm_idx);
	} else {
		tpc_ctl = TPC_MAX;
		ath12k_warn(ar->ab,
			    "ctl array for tpc stats not received from fw\n");
	}

	rates_ctl_min = min_t(s16, rates, tpc_ctl);

	reg_pwr = tpc_stats->max_reg_allowed_power.reg_pwr_array[chain_idx];

	if (reg_pwr < 0)
		reg_pwr = TPC_INVAL;

	tpc = min_t(s16, rates_ctl_min, reg_pwr);

	/* MODULATION_LIMIT is the maximum power limit,tpc should not exceed
	 * modulation limit even if min tpc of all three array is greater
	 * modulation limit
	 */
	tpc = min_t(s16, tpc, MODULATION_LIMIT);

out:
	return tpc;
}

static u16 ath12k_get_ratecode(u16 pream_idx, u16 nss, u16 mcs_rate)
{
	u16 mode_type = ~0;

	/* Below assignments are just for printing purpose only */
	switch (pream_idx) {
	case WMI_TPC_PREAM_CCK:
		mode_type = WMI_RATE_PREAMBLE_CCK;
		break;
	case WMI_TPC_PREAM_OFDM:
		mode_type = WMI_RATE_PREAMBLE_OFDM;
		break;
	case WMI_TPC_PREAM_HT20:
	case WMI_TPC_PREAM_HT40:
		mode_type = WMI_RATE_PREAMBLE_HT;
		break;
	case WMI_TPC_PREAM_VHT20:
	case WMI_TPC_PREAM_VHT40:
	case WMI_TPC_PREAM_VHT80:
	case WMI_TPC_PREAM_VHT160:
		mode_type = WMI_RATE_PREAMBLE_VHT;
		break;
	case WMI_TPC_PREAM_HE20:
	case WMI_TPC_PREAM_HE40:
	case WMI_TPC_PREAM_HE80:
	case WMI_TPC_PREAM_HE160:
		mode_type = WMI_RATE_PREAMBLE_HE;
		break;
	case WMI_TPC_PREAM_EHT20:
	case WMI_TPC_PREAM_EHT40:
	case WMI_TPC_PREAM_EHT60:
	case WMI_TPC_PREAM_EHT80:
	case WMI_TPC_PREAM_EHT120:
	case WMI_TPC_PREAM_EHT140:
	case WMI_TPC_PREAM_EHT160:
	case WMI_TPC_PREAM_EHT200:
	case WMI_TPC_PREAM_EHT240:
	case WMI_TPC_PREAM_EHT280:
	case WMI_TPC_PREAM_EHT320:
		mode_type = WMI_RATE_PREAMBLE_EHT;
		if (mcs_rate == 0 || mcs_rate == 1)
			mcs_rate += 14;
		else
			mcs_rate -= 2;
		break;
	default:
		return mode_type;
	}
	return ((mode_type << 8) | ((nss & 0x7) << 5) | (mcs_rate & 0x1F));
}

static bool ath12k_he_supports_extra_mcs(struct ath12k *ar, int freq)
{
	struct ath12k_pdev_cap *cap = &ar->pdev->cap;
	struct ath12k_band_cap *cap_band;
	bool extra_mcs_supported;

	if (freq <= ATH12K_2GHZ_MAX_FREQUENCY)
		cap_band = &cap->band[NL80211_BAND_2GHZ];
	else if (freq <= ATH12K_5GHZ_MAX_FREQUENCY)
		cap_band = &cap->band[NL80211_BAND_5GHZ];
	else
		cap_band = &cap->band[NL80211_BAND_6GHZ];

	extra_mcs_supported = u32_get_bits(cap_band->he_cap_info[1],
					   HE_EXTRA_MCS_SUPPORT);
	return extra_mcs_supported;
}

static int ath12k_tpc_fill_pream(struct ath12k *ar, char *buf, int buf_len, int len,
				 enum wmi_tpc_pream_bw pream_bw, u32 max_rix,
				 int max_nss, int max_rates, int pream_type,
				 enum wmi_halphy_ctrl_path_stats_id tpc_type,
				 int rate_idx, int eht_rate_idx)
{
	struct wmi_tpc_stats_arg *tpc_stats = ar->debug.tpc_stats;
	int nss, rates, chains;
	u8 active_tx_chains;
	u16 rate_code;
	s16 tpc;

	static const char *const pream_str[] = {
		[WMI_TPC_PREAM_CCK]     = "CCK",
		[WMI_TPC_PREAM_OFDM]    = "OFDM",
		[WMI_TPC_PREAM_HT20]    = "HT20",
		[WMI_TPC_PREAM_HT40]    = "HT40",
		[WMI_TPC_PREAM_VHT20]   = "VHT20",
		[WMI_TPC_PREAM_VHT40]   = "VHT40",
		[WMI_TPC_PREAM_VHT80]   = "VHT80",
		[WMI_TPC_PREAM_VHT160]  = "VHT160",
		[WMI_TPC_PREAM_HE20]    = "HE20",
		[WMI_TPC_PREAM_HE40]    = "HE40",
		[WMI_TPC_PREAM_HE80]    = "HE80",
		[WMI_TPC_PREAM_HE160]   = "HE160",
		[WMI_TPC_PREAM_EHT20]   = "EHT20",
		[WMI_TPC_PREAM_EHT40]   = "EHT40",
		[WMI_TPC_PREAM_EHT60]   = "EHT60",
		[WMI_TPC_PREAM_EHT80]   = "EHT80",
		[WMI_TPC_PREAM_EHT120]   = "EHT120",
		[WMI_TPC_PREAM_EHT140]   = "EHT140",
		[WMI_TPC_PREAM_EHT160]   = "EHT160",
		[WMI_TPC_PREAM_EHT200]   = "EHT200",
		[WMI_TPC_PREAM_EHT240]   = "EHT240",
		[WMI_TPC_PREAM_EHT280]   = "EHT280",
		[WMI_TPC_PREAM_EHT320]   = "EHT320"};

	active_tx_chains = ar->num_tx_chains;

	for (nss = 0; nss < max_nss; nss++) {
		for (rates = 0; rates < max_rates; rates++, rate_idx++, max_rix++) {
			/* FW send extra MCS(10&11) for VHT and HE rates,
			 *  this is not used. Hence skipping it here
			 */
			if (pream_type == WMI_RATE_PREAMBLE_VHT &&
			    rates > ATH12K_VHT_MCS_MAX)
				continue;

			if (pream_type == WMI_RATE_PREAMBLE_HE &&
			    rates > ATH12K_HE_MCS_MAX)
				continue;

			if (pream_type == WMI_RATE_PREAMBLE_EHT &&
			    rates > ATH12K_EHT_MCS_MAX)
				continue;

			rate_code = ath12k_get_ratecode(pream_bw, nss, rates);
			len += scnprintf(buf + len, buf_len - len,
					 "%d\t %s\t 0x%03x\t", max_rix,
					 pream_str[pream_bw], rate_code);

			for (chains = 0; chains < active_tx_chains; chains++) {
				if (nss > chains) {
					len += scnprintf(buf + len,
							 buf_len - len,
							 "\t%s", "NA");
				} else {
					tpc = ath12k_tpc_get_rate(ar, tpc_stats,
								  rate_idx, chains + 1,
								  rate_code, pream_bw,
								  tpc_type,
								  eht_rate_idx);

					if (tpc == TPC_INVAL) {
						len += scnprintf(buf + len,
								 buf_len - len, "\tNA");
					} else {
						len += scnprintf(buf + len,
								 buf_len - len, "\t%d",
								 tpc);
					}
				}
			}
			len += scnprintf(buf + len, buf_len - len, "\n");

			if (pream_type == WMI_RATE_PREAMBLE_EHT)
				/*For fetching the next eht rates pwr from rates array2*/
				++eht_rate_idx;
		}
	}

	return len;
}

static int ath12k_tpc_stats_print(struct ath12k *ar,
				  struct wmi_tpc_stats_arg *tpc_stats,
				  char *buf, size_t len,
				  enum wmi_halphy_ctrl_path_stats_id type)
{
	u32 eht_idx = 0, pream_idx = 0, rate_pream_idx = 0, total_rates = 0, max_rix = 0;
	u32 chan_freq, num_tx_chain, caps, i, j = 1;
	size_t buf_len = ATH12K_TPC_STATS_BUF_SIZE;
	u8 nss, active_tx_chains;
	bool he_ext_mcs;
	static const char *const type_str[WMI_HALPHY_PDEV_TX_STATS_MAX] = {
		[WMI_HALPHY_PDEV_TX_SU_STATS]		= "SU",
		[WMI_HALPHY_PDEV_TX_SUTXBF_STATS]	= "SU WITH TXBF",
		[WMI_HALPHY_PDEV_TX_MU_STATS]		= "MU",
		[WMI_HALPHY_PDEV_TX_MUTXBF_STATS]	= "MU WITH TXBF"};

	u8 max_rates[WMI_TPC_PREAM_MAX] = {
		[WMI_TPC_PREAM_CCK]     = ATH12K_CCK_RATES,
		[WMI_TPC_PREAM_OFDM]    = ATH12K_OFDM_RATES,
		[WMI_TPC_PREAM_HT20]    = ATH12K_HT_RATES,
		[WMI_TPC_PREAM_HT40]    = ATH12K_HT_RATES,
		[WMI_TPC_PREAM_VHT20]   = ATH12K_VHT_RATES,
		[WMI_TPC_PREAM_VHT40]   = ATH12K_VHT_RATES,
		[WMI_TPC_PREAM_VHT80]   = ATH12K_VHT_RATES,
		[WMI_TPC_PREAM_VHT160]  = ATH12K_VHT_RATES,
		[WMI_TPC_PREAM_HE20]    = ATH12K_HE_RATES,
		[WMI_TPC_PREAM_HE40]    = ATH12K_HE_RATES,
		[WMI_TPC_PREAM_HE80]    = ATH12K_HE_RATES,
		[WMI_TPC_PREAM_HE160]   = ATH12K_HE_RATES,
		[WMI_TPC_PREAM_EHT20]   = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT40]   = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT60]   = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT80]   = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT120]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT140]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT160]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT200]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT240]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT280]  = ATH12K_EHT_RATES,
		[WMI_TPC_PREAM_EHT320]  = ATH12K_EHT_RATES};
	static const u8 max_nss[WMI_TPC_PREAM_MAX] = {
		[WMI_TPC_PREAM_CCK]     = ATH12K_NSS_1,
		[WMI_TPC_PREAM_OFDM]    = ATH12K_NSS_1,
		[WMI_TPC_PREAM_HT20]    = ATH12K_NSS_4,
		[WMI_TPC_PREAM_HT40]    = ATH12K_NSS_4,
		[WMI_TPC_PREAM_VHT20]   = ATH12K_NSS_8,
		[WMI_TPC_PREAM_VHT40]   = ATH12K_NSS_8,
		[WMI_TPC_PREAM_VHT80]   = ATH12K_NSS_8,
		[WMI_TPC_PREAM_VHT160]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_HE20]    = ATH12K_NSS_8,
		[WMI_TPC_PREAM_HE40]    = ATH12K_NSS_8,
		[WMI_TPC_PREAM_HE80]    = ATH12K_NSS_8,
		[WMI_TPC_PREAM_HE160]   = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT20]   = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT40]   = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT60]   = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT80]   = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT120]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT140]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT160]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT200]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT240]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT280]  = ATH12K_NSS_4,
		[WMI_TPC_PREAM_EHT320]  = ATH12K_NSS_4};

	u16 rate_idx[WMI_TPC_PREAM_MAX] = {}, eht_rate_idx[WMI_TPC_PREAM_MAX] = {};
	static const u8 pream_type[WMI_TPC_PREAM_MAX] = {
		[WMI_TPC_PREAM_CCK]     = WMI_RATE_PREAMBLE_CCK,
		[WMI_TPC_PREAM_OFDM]    = WMI_RATE_PREAMBLE_OFDM,
		[WMI_TPC_PREAM_HT20]    = WMI_RATE_PREAMBLE_HT,
		[WMI_TPC_PREAM_HT40]    = WMI_RATE_PREAMBLE_HT,
		[WMI_TPC_PREAM_VHT20]   = WMI_RATE_PREAMBLE_VHT,
		[WMI_TPC_PREAM_VHT40]   = WMI_RATE_PREAMBLE_VHT,
		[WMI_TPC_PREAM_VHT80]   = WMI_RATE_PREAMBLE_VHT,
		[WMI_TPC_PREAM_VHT160]  = WMI_RATE_PREAMBLE_VHT,
		[WMI_TPC_PREAM_HE20]    = WMI_RATE_PREAMBLE_HE,
		[WMI_TPC_PREAM_HE40]    = WMI_RATE_PREAMBLE_HE,
		[WMI_TPC_PREAM_HE80]    = WMI_RATE_PREAMBLE_HE,
		[WMI_TPC_PREAM_HE160]   = WMI_RATE_PREAMBLE_HE,
		[WMI_TPC_PREAM_EHT20]   = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT40]   = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT60]   = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT80]   = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT120]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT140]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT160]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT200]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT240]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT280]  = WMI_RATE_PREAMBLE_EHT,
		[WMI_TPC_PREAM_EHT320]  = WMI_RATE_PREAMBLE_EHT};

	chan_freq = le32_to_cpu(tpc_stats->tpc_config.chan_freq);
	num_tx_chain = le32_to_cpu(tpc_stats->tpc_config.num_tx_chain);
	caps = le32_to_cpu(tpc_stats->tpc_config.caps);

	active_tx_chains = ar->num_tx_chains;
	he_ext_mcs = ath12k_he_supports_extra_mcs(ar, chan_freq);

	/* mcs 12&13 is sent by FW for certain HWs in rate array, skipping it as
	 * it is not supported
	 */
	if (he_ext_mcs) {
		for (i = WMI_TPC_PREAM_HE20; i <= WMI_TPC_PREAM_HE160; ++i)
			max_rates[i] = ATH12K_HE_RATES;
	}

	if (type == WMI_HALPHY_PDEV_TX_MU_STATS ||
	    type == WMI_HALPHY_PDEV_TX_MUTXBF_STATS) {
		pream_idx = WMI_TPC_PREAM_VHT20;

		for (i = WMI_TPC_PREAM_CCK; i <= WMI_TPC_PREAM_HT40; ++i)
			max_rix += max_nss[i] * max_rates[i];
	}
	/* Enumerate all the rate indices */
	for (i = rate_pream_idx + 1; i < WMI_TPC_PREAM_MAX; i++) {
		nss = (max_nss[i - 1] < num_tx_chain ?
		       max_nss[i - 1] : num_tx_chain);

		rate_idx[i] = rate_idx[i - 1] + max_rates[i - 1] * nss;

		if (pream_type[i] == WMI_RATE_PREAMBLE_EHT) {
			eht_rate_idx[j] = eht_rate_idx[j - 1] + max_rates[i] * nss;
			++j;
		}
	}

	for (i = 0; i < WMI_TPC_PREAM_MAX; i++) {
		nss = (max_nss[i] < num_tx_chain ?
		       max_nss[i] : num_tx_chain);
		total_rates += max_rates[i] * nss;
	}

	len += scnprintf(buf + len, buf_len - len,
			 "No.of rates-%d\n", total_rates);

	len += scnprintf(buf + len, buf_len - len,
			 "**************** %s ****************\n",
			 type_str[type]);
	len += scnprintf(buf + len, buf_len - len,
			 "\t\t\t\tTPC values for Active chains\n");
	len += scnprintf(buf + len, buf_len - len,
			 "Rate idx Preamble Rate code");

	for (i = 1; i <= active_tx_chains; ++i) {
		len += scnprintf(buf + len, buf_len - len,
				 "\t%d-Chain", i);
	}

	len += scnprintf(buf + len, buf_len - len, "\n");
	for (i = pream_idx; i < WMI_TPC_PREAM_MAX; i++) {
		if (chan_freq <= 2483) {
			if (i == WMI_TPC_PREAM_VHT80 ||
			    i == WMI_TPC_PREAM_VHT160 ||
			    i == WMI_TPC_PREAM_HE80 ||
			    i == WMI_TPC_PREAM_HE160 ||
			    (i >= WMI_TPC_PREAM_EHT60 &&
			     i <= WMI_TPC_PREAM_EHT320)) {
				max_rix += max_nss[i] * max_rates[i];
				continue;
			}
		} else {
			if (i == WMI_TPC_PREAM_CCK) {
				max_rix += max_rates[i];
				continue;
			}
		}

		nss = (max_nss[i] < ar->num_tx_chains ? max_nss[i] : ar->num_tx_chains);

		if (!(caps &
		    (1 << ATH12K_TPC_STATS_SUPPORT_BE_PUNC))) {
			if (i == WMI_TPC_PREAM_EHT60 || i == WMI_TPC_PREAM_EHT120 ||
			    i == WMI_TPC_PREAM_EHT140 || i == WMI_TPC_PREAM_EHT200 ||
			    i == WMI_TPC_PREAM_EHT240 || i == WMI_TPC_PREAM_EHT280) {
				max_rix += max_nss[i] * max_rates[i];
				continue;
			}
		}

		len = ath12k_tpc_fill_pream(ar, buf, buf_len, len, i, max_rix, nss,
					    max_rates[i], pream_type[i],
					    type, rate_idx[i], eht_rate_idx[eht_idx]);

		if (pream_type[i] == WMI_RATE_PREAMBLE_EHT)
			/*For fetch the next index eht rates from rates array2*/
			++eht_idx;

		max_rix += max_nss[i] * max_rates[i];
	}
	return len;
}

static void ath12k_tpc_stats_fill(struct ath12k *ar,
				  struct wmi_tpc_stats_arg *tpc_stats,
				  char *buf)
{
	size_t buf_len = ATH12K_TPC_STATS_BUF_SIZE;
	struct wmi_tpc_config_params *tpc;
	size_t len = 0;

	if (!tpc_stats) {
		ath12k_warn(ar->ab, "failed to find tpc stats\n");
		return;
	}

	spin_lock_bh(&ar->data_lock);

	tpc = &tpc_stats->tpc_config;
	len += scnprintf(buf + len, buf_len - len, "\n");
	len += scnprintf(buf + len, buf_len - len,
			 "*************** TPC config **************\n");
	len += scnprintf(buf + len, buf_len - len,
			 "* powers are in 0.25 dBm steps\n");
	len += scnprintf(buf + len, buf_len - len,
			 "reg domain-%d\t\tchan freq-%d\n",
			 tpc->reg_domain, tpc->chan_freq);
	len += scnprintf(buf + len, buf_len - len,
			 "power limit-%d\t\tmax reg-domain Power-%d\n",
			 le32_to_cpu(tpc->twice_max_reg_power) / 2, tpc->power_limit);
	len += scnprintf(buf + len, buf_len - len,
			 "No.of tx chain-%d\t",
			 ar->num_tx_chains);

	ath12k_tpc_stats_print(ar, tpc_stats, buf, len,
			       ar->debug.tpc_stats_type);

	spin_unlock_bh(&ar->data_lock);
}

static int ath12k_open_tpc_stats(struct inode *inode, struct file *file)
{
	struct ath12k *ar = inode->i_private;
	struct ath12k_hw *ah = ath12k_ar_to_ah(ar);
	int ret;

	guard(wiphy)(ath12k_ar_to_hw(ar)->wiphy);

	if (ah->state != ATH12K_HW_STATE_ON) {
		ath12k_warn(ar->ab, "Interface not up\n");
		return -ENETDOWN;
	}

	void *buf __free(kfree) = kzalloc(ATH12K_TPC_STATS_BUF_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = ath12k_debug_tpc_stats_request(ar);
	if (ret) {
		ath12k_warn(ar->ab, "failed to request tpc stats: %d\n",
			    ret);
		return ret;
	}

	if (!wait_for_completion_timeout(&ar->debug.tpc_complete, TPC_STATS_WAIT_TIME)) {
		spin_lock_bh(&ar->data_lock);
		ath12k_wmi_free_tpc_stats_mem(ar);
		ar->debug.tpc_request = false;
		spin_unlock_bh(&ar->data_lock);
		return -ETIMEDOUT;
	}

	ath12k_tpc_stats_fill(ar, ar->debug.tpc_stats, buf);
	file->private_data = no_free_ptr(buf);

	spin_lock_bh(&ar->data_lock);
	ath12k_wmi_free_tpc_stats_mem(ar);
	spin_unlock_bh(&ar->data_lock);

	return 0;
}

static ssize_t ath12k_read_tpc_stats(struct file *file,
				     char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	const char *buf = file->private_data;
	size_t len = strlen(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static int ath12k_release_tpc_stats(struct inode *inode,
				    struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static const struct file_operations fops_tpc_stats = {
	.open = ath12k_open_tpc_stats,
	.release = ath12k_release_tpc_stats,
	.read = ath12k_read_tpc_stats,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static const struct file_operations fops_tpc_stats_type = {
	.write = ath12k_write_tpc_stats_type,
	.open = simple_open,
	.llseek = default_llseek,
};

static ssize_t ath12k_write_extd_rx_stats(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct ath12k *ar = file->private_data;
	struct htt_rx_ring_tlv_filter tlv_filter = {0};
	u32 ring_id, rx_filter = 0;
	bool enable;
	int ret, i;

	if (kstrtobool_from_user(ubuf, count, &enable))
		return -EINVAL;

	wiphy_lock(ath12k_ar_to_hw(ar)->wiphy);

	if (!ar->ab->hw_params->rxdma1_enable) {
		ret = count;
		goto exit;
	}

	if (ar->ah->state != ATH12K_HW_STATE_ON) {
		ret = -ENETDOWN;
		goto exit;
	}

	if (enable == ar->debug.extd_rx_stats) {
		ret = count;
		goto exit;
	}

	if (enable) {
		rx_filter =  HTT_RX_FILTER_TLV_FLAGS_MPDU_START;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_START;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_END;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_END_USER_STATS;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_END_USER_STATS_EXT;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_END_STATUS_DONE;
		rx_filter |= HTT_RX_FILTER_TLV_FLAGS_PPDU_START_USER_INFO;

		tlv_filter.rx_filter = rx_filter;
		tlv_filter.pkt_filter_flags0 = HTT_RX_FP_MGMT_FILTER_FLAGS0;
		tlv_filter.pkt_filter_flags1 = HTT_RX_FP_MGMT_FILTER_FLAGS1;
		tlv_filter.pkt_filter_flags2 = HTT_RX_FP_CTRL_FILTER_FLASG2;
		tlv_filter.pkt_filter_flags3 = HTT_RX_FP_CTRL_FILTER_FLASG3 |
			HTT_RX_FP_DATA_FILTER_FLASG3;
	} else {
		tlv_filter = ath12k_mac_mon_status_filter_default;
	}

	ar->debug.rx_filter = tlv_filter.rx_filter;

	for (i = 0; i < ar->ab->hw_params->num_rxdma_per_pdev; i++) {
		ring_id = ar->dp.rxdma_mon_dst_ring[i].ring_id;
		ret = ath12k_dp_tx_htt_rx_filter_setup(ar->ab, ring_id, ar->dp.mac_id + i,
						       HAL_RXDMA_MONITOR_DST,
						       DP_RXDMA_REFILL_RING_SIZE,
						       &tlv_filter);
		if (ret) {
			ath12k_warn(ar->ab, "failed to set rx filter for monitor status ring\n");
			goto exit;
		}
	}

	ar->debug.extd_rx_stats = !!enable;
	ret = count;
exit:
	wiphy_unlock(ath12k_ar_to_hw(ar)->wiphy);
	return ret;
}

static ssize_t ath12k_read_extd_rx_stats(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct ath12k *ar = file->private_data;
	char buf[32];
	int len = 0;

	wiphy_lock(ath12k_ar_to_hw(ar)->wiphy);
	len = scnprintf(buf, sizeof(buf) - len, "%d\n",
			ar->debug.extd_rx_stats);
	wiphy_unlock(ath12k_ar_to_hw(ar)->wiphy);

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static const struct file_operations fops_extd_rx_stats = {
	.read = ath12k_read_extd_rx_stats,
	.write = ath12k_write_extd_rx_stats,
	.open = simple_open,
};

void ath12k_debugfs_soc_create(struct ath12k_base *ab)
{
	bool dput_needed;
	char soc_name[64] = { 0 };
	struct dentry *debugfs_ath12k;

	debugfs_ath12k = debugfs_lookup("ath12k", NULL);
	if (debugfs_ath12k) {
		/* a dentry from lookup() needs dput() after we don't use it */
		dput_needed = true;
	} else {
		debugfs_ath12k = debugfs_create_dir("ath12k", NULL);
		if (IS_ERR_OR_NULL(debugfs_ath12k))
			return;
		dput_needed = false;
	}

	scnprintf(soc_name, sizeof(soc_name), "%s-%s", ath12k_bus_str(ab->hif.bus),
		  dev_name(ab->dev));

	ab->debugfs_soc = debugfs_create_dir(soc_name, debugfs_ath12k);

	if (dput_needed)
		dput(debugfs_ath12k);
}

void ath12k_debugfs_soc_destroy(struct ath12k_base *ab)
{
	debugfs_remove_recursive(ab->debugfs_soc);
	ab->debugfs_soc = NULL;
	/* We are not removing ath12k directory on purpose, even if it
	 * would be empty. This simplifies the directory handling and it's
	 * a minor cosmetic issue to leave an empty ath12k directory to
	 * debugfs.
	 */
}

static void ath12k_fw_stats_pdevs_free(struct list_head *head)
{
	struct ath12k_fw_stats_pdev *i, *tmp;

	list_for_each_entry_safe(i, tmp, head, list) {
		list_del(&i->list);
		kfree(i);
	}
}

static void ath12k_fw_stats_bcn_free(struct list_head *head)
{
	struct ath12k_fw_stats_bcn *i, *tmp;

	list_for_each_entry_safe(i, tmp, head, list) {
		list_del(&i->list);
		kfree(i);
	}
}

static void ath12k_fw_stats_vdevs_free(struct list_head *head)
{
	struct ath12k_fw_stats_vdev *i, *tmp;

	list_for_each_entry_safe(i, tmp, head, list) {
		list_del(&i->list);
		kfree(i);
	}
}

void ath12k_debugfs_fw_stats_reset(struct ath12k *ar)
{
	spin_lock_bh(&ar->data_lock);
	ar->fw_stats.fw_stats_done = false;
	ath12k_fw_stats_vdevs_free(&ar->fw_stats.vdevs);
	ath12k_fw_stats_bcn_free(&ar->fw_stats.bcn);
	ath12k_fw_stats_pdevs_free(&ar->fw_stats.pdevs);
	spin_unlock_bh(&ar->data_lock);
}

static int ath12k_debugfs_fw_stats_request(struct ath12k *ar,
					   struct ath12k_fw_stats_req_params *param)
{
	struct ath12k_base *ab = ar->ab;
	unsigned long timeout, time_left;
	int ret;

	lockdep_assert_wiphy(ath12k_ar_to_hw(ar)->wiphy);

	/* FW stats can get split when exceeding the stats data buffer limit.
	 * In that case, since there is no end marking for the back-to-back
	 * received 'update stats' event, we keep a 3 seconds timeout in case,
	 * fw_stats_done is not marked yet
	 */
	timeout = jiffies + msecs_to_jiffies(3 * 1000);

	ath12k_debugfs_fw_stats_reset(ar);

	reinit_completion(&ar->fw_stats_complete);

	ret = ath12k_wmi_send_stats_request_cmd(ar, param->stats_id,
						param->vdev_id, param->pdev_id);

	if (ret) {
		ath12k_warn(ab, "could not request fw stats (%d)\n",
			    ret);
		return ret;
	}

	time_left = wait_for_completion_timeout(&ar->fw_stats_complete,
						1 * HZ);
	/* If the wait timed out, return -ETIMEDOUT */
	if (!time_left)
		return -ETIMEDOUT;

	/* Firmware sends WMI_UPDATE_STATS_EVENTID back-to-back
	 * when stats data buffer limit is reached. fw_stats_complete
	 * is completed once host receives first event from firmware, but
	 * still end might not be marked in the TLV.
	 * Below loop is to confirm that firmware completed sending all the event
	 * and fw_stats_done is marked true when end is marked in the TLV
	 */
	for (;;) {
		if (time_after(jiffies, timeout))
			break;

		spin_lock_bh(&ar->data_lock);
		if (ar->fw_stats.fw_stats_done) {
			spin_unlock_bh(&ar->data_lock);
			break;
		}
		spin_unlock_bh(&ar->data_lock);
	}
	return 0;
}

void
ath12k_debugfs_fw_stats_process(struct ath12k *ar,
				struct ath12k_fw_stats *stats)
{
	struct ath12k_base *ab = ar->ab;
	struct ath12k_pdev *pdev;
	bool is_end;
	static unsigned int num_vdev, num_bcn;
	size_t total_vdevs_started = 0;
	int i;

	if (stats->stats_id == WMI_REQUEST_VDEV_STAT) {
		if (list_empty(&stats->vdevs)) {
			ath12k_warn(ab, "empty vdev stats");
			return;
		}
		/* FW sends all the active VDEV stats irrespective of PDEV,
		 * hence limit until the count of all VDEVs started
		 */
		rcu_read_lock();
		for (i = 0; i < ab->num_radios; i++) {
			pdev = rcu_dereference(ab->pdevs_active[i]);
			if (pdev && pdev->ar)
				total_vdevs_started += pdev->ar->num_started_vdevs;
		}
		rcu_read_unlock();

		is_end = ((++num_vdev) == total_vdevs_started);

		list_splice_tail_init(&stats->vdevs,
				      &ar->fw_stats.vdevs);

		if (is_end) {
			ar->fw_stats.fw_stats_done = true;
			num_vdev = 0;
		}
		return;
	}
	if (stats->stats_id == WMI_REQUEST_BCN_STAT) {
		if (list_empty(&stats->bcn)) {
			ath12k_warn(ab, "empty beacon stats");
			return;
		}
		/* Mark end until we reached the count of all started VDEVs
		 * within the PDEV
		 */
		is_end = ((++num_bcn) == ar->num_started_vdevs);

		list_splice_tail_init(&stats->bcn,
				      &ar->fw_stats.bcn);

		if (is_end) {
			ar->fw_stats.fw_stats_done = true;
			num_bcn = 0;
		}
	}
	if (stats->stats_id == WMI_REQUEST_PDEV_STAT) {
		list_splice_tail_init(&stats->pdevs, &ar->fw_stats.pdevs);
		ar->fw_stats.fw_stats_done = true;
	}
}

static int ath12k_open_vdev_stats(struct inode *inode, struct file *file)
{
	struct ath12k *ar = inode->i_private;
	struct ath12k_fw_stats_req_params param;
	struct ath12k_hw *ah = ath12k_ar_to_ah(ar);
	int ret;

	guard(wiphy)(ath12k_ar_to_hw(ar)->wiphy);

	if (!ah)
		return -ENETDOWN;

	if (ah->state != ATH12K_HW_STATE_ON)
		return -ENETDOWN;

	void *buf __free(kfree) = kzalloc(ATH12K_FW_STATS_BUF_SIZE, GFP_ATOMIC);
	if (!buf)
		return -ENOMEM;

	param.pdev_id = ath12k_mac_get_target_pdev_id(ar);
	/* VDEV stats is always sent for all active VDEVs from FW */
	param.vdev_id = 0;
	param.stats_id = WMI_REQUEST_VDEV_STAT;

	ret = ath12k_debugfs_fw_stats_request(ar, &param);
	if (ret) {
		ath12k_warn(ar->ab, "failed to request fw vdev stats: %d\n", ret);
		return ret;
	}

	ath12k_wmi_fw_stats_dump(ar, &ar->fw_stats, param.stats_id,
				 buf);

	file->private_data = no_free_ptr(buf);

	return 0;
}

static int ath12k_release_vdev_stats(struct inode *inode, struct file *file)
{
	kfree(file->private_data);

	return 0;
}

static ssize_t ath12k_read_vdev_stats(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	const char *buf = file->private_data;
	size_t len = strlen(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_vdev_stats = {
	.open = ath12k_open_vdev_stats,
	.release = ath12k_release_vdev_stats,
	.read = ath12k_read_vdev_stats,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static int ath12k_open_bcn_stats(struct inode *inode, struct file *file)
{
	struct ath12k *ar = inode->i_private;
	struct ath12k_link_vif *arvif;
	struct ath12k_fw_stats_req_params param;
	struct ath12k_hw *ah = ath12k_ar_to_ah(ar);
	int ret;

	guard(wiphy)(ath12k_ar_to_hw(ar)->wiphy);

	if (ah && ah->state != ATH12K_HW_STATE_ON)
		return -ENETDOWN;

	void *buf __free(kfree) = kzalloc(ATH12K_FW_STATS_BUF_SIZE, GFP_ATOMIC);
	if (!buf)
		return -ENOMEM;

	param.pdev_id = ath12k_mac_get_target_pdev_id(ar);
	param.stats_id = WMI_REQUEST_BCN_STAT;

	/* loop all active VDEVs for bcn stats */
	list_for_each_entry(arvif, &ar->arvifs, list) {
		if (!arvif->is_up)
			continue;

		param.vdev_id = arvif->vdev_id;
		ret = ath12k_debugfs_fw_stats_request(ar, &param);
		if (ret) {
			ath12k_warn(ar->ab, "failed to request fw bcn stats: %d\n", ret);
			return ret;
		}
	}

	ath12k_wmi_fw_stats_dump(ar, &ar->fw_stats, param.stats_id,
				 buf);
	/* since beacon stats request is looped for all active VDEVs, saved fw
	 * stats is not freed for each request until done for all active VDEVs
	 */
	spin_lock_bh(&ar->data_lock);
	ath12k_fw_stats_bcn_free(&ar->fw_stats.bcn);
	spin_unlock_bh(&ar->data_lock);

	file->private_data = no_free_ptr(buf);

	return 0;
}

static int ath12k_release_bcn_stats(struct inode *inode, struct file *file)
{
	kfree(file->private_data);

	return 0;
}

static ssize_t ath12k_read_bcn_stats(struct file *file,
				     char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	const char *buf = file->private_data;
	size_t len = strlen(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_bcn_stats = {
	.open = ath12k_open_bcn_stats,
	.release = ath12k_release_bcn_stats,
	.read = ath12k_read_bcn_stats,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static int ath12k_open_pdev_stats(struct inode *inode, struct file *file)
{
	struct ath12k *ar = inode->i_private;
	struct ath12k_hw *ah = ath12k_ar_to_ah(ar);
	struct ath12k_base *ab = ar->ab;
	struct ath12k_fw_stats_req_params param;
	int ret;

	guard(wiphy)(ath12k_ar_to_hw(ar)->wiphy);

	if (ah && ah->state != ATH12K_HW_STATE_ON)
		return -ENETDOWN;

	void *buf __free(kfree) = kzalloc(ATH12K_FW_STATS_BUF_SIZE, GFP_ATOMIC);
	if (!buf)
		return -ENOMEM;

	param.pdev_id = ath12k_mac_get_target_pdev_id(ar);
	param.vdev_id = 0;
	param.stats_id = WMI_REQUEST_PDEV_STAT;

	ret = ath12k_debugfs_fw_stats_request(ar, &param);
	if (ret) {
		ath12k_warn(ab, "failed to request fw pdev stats: %d\n", ret);
		return ret;
	}

	ath12k_wmi_fw_stats_dump(ar, &ar->fw_stats, param.stats_id,
				 buf);

	file->private_data = no_free_ptr(buf);

	return 0;
}

static int ath12k_release_pdev_stats(struct inode *inode, struct file *file)
{
	kfree(file->private_data);

	return 0;
}

static ssize_t ath12k_read_pdev_stats(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	const char *buf = file->private_data;
	size_t len = strlen(buf);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations fops_pdev_stats = {
	.open = ath12k_open_pdev_stats,
	.release = ath12k_release_pdev_stats,
	.read = ath12k_read_pdev_stats,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static
void ath12k_debugfs_fw_stats_register(struct ath12k *ar)
{
	struct dentry *fwstats_dir = debugfs_create_dir("fw_stats",
							ar->debug.debugfs_pdev);

	/* all stats debugfs files created are under "fw_stats" directory
	 * created per PDEV
	 */
	debugfs_create_file("vdev_stats", 0600, fwstats_dir, ar,
			    &fops_vdev_stats);
	debugfs_create_file("beacon_stats", 0600, fwstats_dir, ar,
			    &fops_bcn_stats);
	debugfs_create_file("pdev_stats", 0600, fwstats_dir, ar,
			    &fops_pdev_stats);

	INIT_LIST_HEAD(&ar->fw_stats.vdevs);
	INIT_LIST_HEAD(&ar->fw_stats.bcn);
	INIT_LIST_HEAD(&ar->fw_stats.pdevs);

	init_completion(&ar->fw_stats_complete);
}

void ath12k_debugfs_register(struct ath12k *ar)
{
	struct ath12k_base *ab = ar->ab;
	struct ieee80211_hw *hw = ar->ah->hw;
	char pdev_name[5];
	char buf[100] = {0};

	scnprintf(pdev_name, sizeof(pdev_name), "%s%d", "mac", ar->pdev_idx);

	ar->debug.debugfs_pdev = debugfs_create_dir(pdev_name, ab->debugfs_soc);

	/* Create a symlink under ieee80211/phy* */
	scnprintf(buf, sizeof(buf), "../../ath12k/%pd2", ar->debug.debugfs_pdev);
	ar->debug.debugfs_pdev_symlink = debugfs_create_symlink("ath12k",
								hw->wiphy->debugfsdir,
								buf);

	if (ar->mac.sbands[NL80211_BAND_5GHZ].channels) {
		debugfs_create_file("dfs_simulate_radar", 0200,
				    ar->debug.debugfs_pdev, ar,
				    &fops_simulate_radar);
	}

	debugfs_create_file("tpc_stats", 0400, ar->debug.debugfs_pdev, ar,
			    &fops_tpc_stats);
	debugfs_create_file("tpc_stats_type", 0200, ar->debug.debugfs_pdev,
			    ar, &fops_tpc_stats_type);
	init_completion(&ar->debug.tpc_complete);

	ath12k_debugfs_htt_stats_register(ar);
	ath12k_debugfs_fw_stats_register(ar);

	debugfs_create_file("ext_rx_stats", 0644,
			    ar->debug.debugfs_pdev, ar,
			    &fops_extd_rx_stats);
}

void ath12k_debugfs_unregister(struct ath12k *ar)
{
	if (!ar->debug.debugfs_pdev)
		return;

	/* Remove symlink under ieee80211/phy* */
	debugfs_remove(ar->debug.debugfs_pdev_symlink);
	debugfs_remove_recursive(ar->debug.debugfs_pdev);
	ar->debug.debugfs_pdev_symlink = NULL;
	ar->debug.debugfs_pdev = NULL;
}
