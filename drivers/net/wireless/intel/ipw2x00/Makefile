# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Intel Centrino wireless drivers
#

obj-$(CPTCFG_IPW2100) += ipw2100.o
obj-$(CPTCFG_IPW2200) += ipw2200.o

obj-$(CPTCFG_LIBIPW) += libipw.o
libipw-objs := \
	libipw_module.o \
	libipw_tx.o \
	libipw_rx.o \
	libipw_wx.o \
	libipw_geo.o \
	libipw_spy.o \
	libipw_crypto.o \
	libipw_crypto_ccmp.o \
	libipw_crypto_tkip.o \
	libipw_crypto_wep.o
