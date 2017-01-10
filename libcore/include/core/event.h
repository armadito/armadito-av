/***

Copyright (C) 2015, 2016, 2017 Teclib'

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

#ifndef ARMADITO_CORE_EVENT_H
#define ARMADITO_CORE_EVENT_H

#include <time.h>

enum a6o_event_type {
	EVENT_DETECTION,
	EVENT_ON_DEMAND_START,
	EVENT_ON_DEMAND_COMPLETED,
	EVENT_ON_DEMAND_PROGRESS,
	EVENT_QUARANTINE,
	EVENT_REAL_TIME_PROT,
	EVENT_AV_UPDATE,
};

enum a6o_detection_context {
	CONTEXT_REAL_TIME,
	CONTEXT_ON_DEMAND,
};

enum a6o_quarantine_action {
	QUARANTINE_ENTER,
	QUARANTINE_EXIT,
};

struct a6o_detection_event {
	enum a6o_detection_context context;
	const char *path;
	enum a6o_file_status scan_status;
	enum a6o_action scan_action;
	const char *module_name;
	const char *module_report;
};

struct a6o_on_demand_start_event {
	const char *root_path;
};

struct a6o_on_demand_completed_event {
	size_t total_malware_count;
	size_t total_suspicious_count;
	size_t total_scanned_count;
};

struct a6o_on_demand_progress_event {
	int progress;
	const char *path;
	size_t malware_count;
	size_t suspicious_count;
	size_t scanned_count;
};

struct a6o_quarantine_event {
	enum a6o_quarantine_action quarantine_action;
	const char *orig_path;
	const char *quarantine_path;
};

struct a6o_real_time_prot_event {
	int rt_prot_new_state;
};

struct a6o_av_update_event {
};

union u_event {
	struct a6o_detection_event ev_detection;
	struct a6o_on_demand_start_event ev_on_demand_start;
	struct a6o_on_demand_completed_event ev_on_demand_completed;
	struct a6o_on_demand_progress_event ev_on_demand_progress;
	struct a6o_quarantine_event ev_quarantine;
	struct a6o_real_time_prot_event ev_real_time_prot;
	struct a6o_av_update_event ev_av_update;
};

struct a6o_event {
	time_t timestamp;
	enum a6o_event_type ev_type;
	union u_event u_ev;
};

#endif
