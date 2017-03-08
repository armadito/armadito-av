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

#include <libarmadito/armadito.h>

#include "core/event.h"

#include <syslog.h>

/* name="/home/malwares/contagio-malware/jar/MALWARE_JAR_200_files/Mal_Java_64FD14CEF0026D4240A4550E6A6F9E83.jar » ZIP » a/kors.class", threat="a variant of Java/Exploit.Agent.OKJ trojan", action="action selection postponed until scan completion", info="" */

static void detection_event_journal(struct a6o_detection_event *ev)
{
#if 0
	ev->context;
	ev->scan_id;
	ev->path;
	ev->scan_status;
	ev->scan_action;
	ev->module_name;
	ev->module_report;
#endif
}

static void on_demand_start_event_journal(struct a6o_on_demand_start_event *ev)
{
#if 0
	ev->root_path
		ev->scan_id
#endif
}

static void on_demand_completed_event_journal(struct a6o_on_demand_completed_event *ev)
{
#if 0
	ev->scan_id
		ev->cancelled
		ev->total_malware_count
		ev->total_suspicious_count
		ev->total_scanned_count
		ev->duration
#endif
}

static void on_demand_progress_event_journal(struct a6o_on_demand_progress_event *ev)
{
#if 0
	ev->scan_id
		ev->progress
		ev->path
		ev->malware_count
		ev->suspicious_count
		ev->scanned_count
#endif
}

static void quarantine_event_journal(struct a6o_quarantine_event *ev)
{
#if 0
	ev->quarantine_action
		ev->orig_path
		ev->quarantine_path
#endif
}

static void real_time_prot_event_journal(struct a6o_real_time_prot_event *ev)
{
#if 0
	ev->rt_prot_new_state
#endif
}

static void journal_event_cb(struct a6o_event *ev, void *data)
{
	switch(ev->type) {
	case EVENT_DETECTION:
		detection_event_journal(&ev->u.ev_detection);
		break;
	case EVENT_ON_DEMAND_START:
		on_demand_start_event_journal(&ev->u.ev_on_demand_start);
		break;
	case EVENT_ON_DEMAND_COMPLETED:
		on_demand_completed_event_journal(&ev->u.ev_on_demand_completed);
		break;
	case EVENT_ON_DEMAND_PROGRESS:
		on_demand_progress_event_journal(&ev->u.ev_on_demand_progress);
		break;
	case EVENT_QUARANTINE:
		quarantine_event_journal(&ev->u.ev_quarantine);
		break;
	case EVENT_REAL_TIME_PROT:
		real_time_prot_event_journal(&ev->u.ev_real_time_prot);
		break;
	case EVENT_AV_UPDATE:
		break;
	}
}

void journal_init(struct armadito *armadito)
{
	int event_mask;

	event_mask = EVENT_DETECTION
		| EVENT_ON_DEMAND_START
		| EVENT_ON_DEMAND_COMPLETED
		| EVENT_ON_DEMAND_PROGRESS
		| EVENT_QUARANTINE
		| EVENT_REAL_TIME_PROT
		| EVENT_AV_UPDATE;

	a6o_event_source_add_cb(a6o_get_event_source(armadito), event_mask, journal_event_cb, NULL);
}
