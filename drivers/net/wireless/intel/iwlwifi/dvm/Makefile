# SPDX-License-Identifier: GPL-2.0
# DVM
obj-$(CPTCFG_IWLDVM)	+= iwldvm.o
iwldvm-objs		+= main.o rs.o mac80211.o ucode.o tx.o
iwldvm-objs		+= lib.o calib.o tt.o sta.o rx.o eeprom.o

iwldvm-objs		+= power.o
iwldvm-objs		+= scan.o
iwldvm-objs		+= rxon.o devices.o

iwldvm-$(CPTCFG_IWLWIFI_LEDS) += led.o
iwldvm-$(CPTCFG_IWLWIFI_DEBUGFS) += debugfs.o

ccflags-y += -I $(src)/../
