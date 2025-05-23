/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Portions of this file
 * Copyright (C) 2019 Intel Corporation
 */

#ifdef CPTCFG_MAC80211_MESSAGE_TRACING

#if !defined(__MAC80211_MSG_DRIVER_TRACE) || defined(TRACE_HEADER_MULTI_READ)
#define __MAC80211_MSG_DRIVER_TRACE

#include <linux/tracepoint.h>
#include <net/mac80211.h>
#include "ieee80211_i.h"

#undef TRACE_SYSTEM
#define TRACE_SYSTEM mac80211_msg

DECLARE_EVENT_CLASS(mac80211_msg_event,
	TP_PROTO(struct va_format *vaf),

	TP_ARGS(vaf),

	TP_STRUCT__entry(
		__vstring(msg, vaf->fmt, vaf->va)
	),

	TP_fast_assign(
		__assign_vstr(msg, vaf->fmt, vaf->va);
	),

	TP_printk("%s", __get_str(msg))
);

DEFINE_EVENT(mac80211_msg_event, mac80211_info,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);
DEFINE_EVENT(mac80211_msg_event, mac80211_dbg,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);
DEFINE_EVENT(mac80211_msg_event, mac80211_err,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);
#endif /* !__MAC80211_MSG_DRIVER_TRACE || TRACE_HEADER_MULTI_READ */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace_msg
#include <trace/define_trace.h>

#endif
