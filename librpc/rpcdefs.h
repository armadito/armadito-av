/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include <libjrpc/marshall.h>

JRPC_ENUM(a6o_file_status)
	JRPC_ENUM_VALUE(A6O_FILE_UNDECIDED)
	JRPC_ENUM_VALUE(A6O_FILE_CLEAN)
	JRPC_ENUM_VALUE(A6O_FILE_UNKNOWN_TYPE)
	JRPC_ENUM_VALUE(A6O_FILE_EINVAL)
	JRPC_ENUM_VALUE(A6O_FILE_IERROR)
	JRPC_ENUM_VALUE(A6O_FILE_SUSPICIOUS)
	JRPC_ENUM_VALUE(A6O_FILE_WHITE_LISTED)
	JRPC_ENUM_VALUE(A6O_FILE_MALWARE)
JRPC_ENUM_END

JRPC_ENUM(a6o_update_status)
	JRPC_ENUM_VALUE(A6O_UPDATE_OK)
	JRPC_ENUM_VALUE(A6O_UPDATE_LATE)
	JRPC_ENUM_VALUE(A6O_UPDATE_CRITICAL)
	JRPC_ENUM_VALUE(A6O_UPDATE_NON_AVAILABLE)
JRPC_ENUM_END

JRPC_STRUCT(a6o_base_info)
	JRPC_STRUCT_FIELD_STRING(name)
	JRPC_STRUCT_FIELD_INT(time_t, base_update_ts)
	JRPC_STRUCT_FIELD_STRING(version)
	JRPC_STRUCT_FIELD_INT(size_t, signature_count)
	JRPC_STRUCT_FIELD_STRING(full_path)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_module_info)
	JRPC_STRUCT_FIELD_STRING(name)
	JRPC_STRUCT_FIELD_ENUM(a6o_update_status, mod_status)
	JRPC_STRUCT_FIELD_INT(time_t, mod_update_ts)
	JRPC_STRUCT_FIELD_PTR_ARRAY(a6o_base_info, base_infos)
JRPC_STRUCT_END

JRPC_ENUM(a6o_action)
	JRPC_ENUM_VALUE(A6O_ACTION_NONE)
	JRPC_ENUM_VALUE(A6O_ACTION_ALERT)
	JRPC_ENUM_VALUE(A6O_ACTION_QUARANTINE)
	JRPC_ENUM_VALUE(A6O_ACTION_REMOVE)
JRPC_ENUM_END

JRPC_ENUM(a6o_event_type)
	JRPC_ENUM_VALUE(EVENT_DETECTION)
	JRPC_ENUM_VALUE(EVENT_ON_DEMAND_START)
	JRPC_ENUM_VALUE(EVENT_ON_DEMAND_COMPLETED)
	JRPC_ENUM_VALUE(EVENT_ON_DEMAND_PROGRESS)
	JRPC_ENUM_VALUE(EVENT_QUARANTINE)
	JRPC_ENUM_VALUE(EVENT_REAL_TIME_PROT)
	JRPC_ENUM_VALUE(EVENT_AV_UPDATE)
JRPC_ENUM_END

JRPC_ENUM(a6o_detection_context)
	JRPC_ENUM_VALUE(CONTEXT_REAL_TIME)
	JRPC_ENUM_VALUE(CONTEXT_ON_DEMAND)
JRPC_ENUM_END

JRPC_ENUM(a6o_quarantine_action)
	JRPC_ENUM_VALUE(QUARANTINE_ENTER)
	JRPC_ENUM_VALUE(QUARANTINE_EXIT)
JRPC_ENUM_END

JRPC_STRUCT(a6o_detection_event)
	JRPC_STRUCT_FIELD_ENUM(a6o_detection_context, context)
	JRPC_STRUCT_FIELD_STRING(path)
	JRPC_STRUCT_FIELD_ENUM(a6o_file_status, scan_status)
	JRPC_STRUCT_FIELD_ENUM(a6o_action, scan_action)
	JRPC_STRUCT_FIELD_STRING(module_name)
	JRPC_STRUCT_FIELD_STRING(module_report)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_on_demand_start_event)
	JRPC_STRUCT_FIELD_STRING(root_path)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_on_demand_completed_event)
	JRPC_STRUCT_FIELD_INT(size_t, total_malware_count)
	JRPC_STRUCT_FIELD_INT(size_t, total_suspicious_count)
	JRPC_STRUCT_FIELD_INT(size_t, total_scanned_count)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_on_demand_progress_event)
	JRPC_STRUCT_FIELD_INT(int, progress)
	JRPC_STRUCT_FIELD_STRING(path)
	JRPC_STRUCT_FIELD_INT(size_t, malware_count)
	JRPC_STRUCT_FIELD_INT(size_t, suspicious_count)
	JRPC_STRUCT_FIELD_INT(size_t, scanned_count)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_quarantine_event)
	JRPC_STRUCT_FIELD_ENUM(a6o_quarantine_action, quarantine_action)
	JRPC_STRUCT_FIELD_STRING(orig_path)
	JRPC_STRUCT_FIELD_STRING(quarantine_path)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_real_time_prot_event)
	JRPC_STRUCT_FIELD_INT(int, rt_prot_new_state)
JRPC_STRUCT_END

JRPC_STRUCT(a6o_av_update_event)
JRPC_STRUCT_END

JRPC_UNION(u_event)
	JRPC_UNION_FIELD_STRUCT(a6o_detection_event, ev_detection, EVENT_DETECTION)
	JRPC_UNION_FIELD_STRUCT(a6o_on_demand_start_event, ev_on_demand_start, EVENT_ON_DEMAND_START)
	JRPC_UNION_FIELD_STRUCT(a6o_on_demand_completed_event, ev_on_demand_completed, EVENT_ON_DEMAND_COMPLETED)
	JRPC_UNION_FIELD_STRUCT(a6o_on_demand_progress_event, ev_on_demand_progress, EVENT_ON_DEMAND_PROGRESS)
	JRPC_UNION_FIELD_STRUCT(a6o_quarantine_event, ev_quarantine, EVENT_QUARANTINE)
	JRPC_UNION_FIELD_STRUCT(a6o_real_time_prot_event, ev_real_time_prot, EVENT_REAL_TIME_PROT)
	JRPC_UNION_FIELD_STRUCT(a6o_av_update_event, ev_av_update, EVENT_AV_UPDATE)
JRPC_UNION_END

JRPC_STRUCT(a6o_event)
	JRPC_STRUCT_FIELD_INT(time_t, timestamp)
	JRPC_STRUCT_FIELD_ENUM(a6o_event_type, ev_type)
	JRPC_STRUCT_FIELD_UNION(u_event, u_ev, ev_type)
JRPC_STRUCT_END

