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
#include "core/handle.h"
#include "core/status.h"

#include <syslog.h>
#include <stdio.h>

/* format like:
name="/home/malwares/contagio-malware/jar/MALWARE_JAR_200_files/Mal_Java_64FD14CEF0026D4240A4550E6A6F9E83.jar » ZIP » a/kors.class", threat="a variant of Java/Exploit.Agent.OKJ trojan", action="action selection postponed until scan completion", info="" */

static void detection_event_journal(struct a6o_event *ev)
{
	syslog(LOG_INFO,
		"type=\"detection\", path=\"%s\", scan_status=\"%s\", scan_action=\"%s\", module_name=\"%s\", module_report=\"%s\", scan_id=%d",
		ev->u.ev_detection.path,
		a6o_file_status_pretty_str(ev->u.ev_detection.scan_status),
		a6o_action_pretty_str(ev->u.ev_detection.scan_action),
		ev->u.ev_detection.module_name,
		ev->u.ev_detection.module_report,
		ev->u.ev_detection.scan_id);
}

static void on_demand_start_event_journal(struct a6o_event *ev)
{
	syslog(LOG_INFO,
		"type=\"on_demand_start\", root_path=\"%s\", scan_id=%d",
		ev->u.ev_on_demand_start.root_path,
		ev->u.ev_on_demand_start.scan_id);
}

static void on_demand_completed_event_journal(struct a6o_event *ev)
{
	syslog(LOG_INFO,
		"type=\"on_demand_completed\", scan_id=%d, cancelled=%d, total_malware_count=%ld, total_suspicious_count=%ld, total_scanned_count=%ld, duration=%ld",
		ev->u.ev_on_demand_completed.scan_id,
		ev->u.ev_on_demand_completed.cancelled,
		ev->u.ev_on_demand_completed.total_malware_count,
		ev->u.ev_on_demand_completed.total_suspicious_count,
		ev->u.ev_on_demand_completed.total_scanned_count,
		ev->u.ev_on_demand_completed.duration);
}

static void quarantine_event_journal(struct a6o_event *ev)
{
	syslog(LOG_INFO,
		"type=\"quarantine\", action=\"%s\", orig_path=\"%s\", quarantine_path=\"%s\"",
		ev->u.ev_quarantine.quarantine_action == QUARANTINE_ENTER ? "enter" : "exit",
		ev->u.ev_quarantine.orig_path,
		ev->u.ev_quarantine.quarantine_path);
}

static void real_time_prot_event_journal(struct a6o_event *ev)
{
	syslog(LOG_INFO,
		"type=\"real_time_prot\", rt_prot_new_state=\"%d\"",
		ev->u.ev_real_time_prot.rt_prot_new_state);
}

static void journal_event_cb(struct a6o_event *ev, void *data)
{
	switch(ev->type) {
	case EVENT_DETECTION:
		detection_event_journal(ev);
		break;
	case EVENT_ON_DEMAND_START:
		on_demand_start_event_journal(ev);
		break;
	case EVENT_ON_DEMAND_COMPLETED:
		on_demand_completed_event_journal(ev);
		break;
	case EVENT_ON_DEMAND_PROGRESS:
		/* not logged */
		break;
	case EVENT_QUARANTINE:
		quarantine_event_journal(ev);
		break;
	case EVENT_REAL_TIME_PROT:
		real_time_prot_event_journal(ev);
		break;
	case EVENT_AV_UPDATE:
		break;
	}
}

void journal_init(struct armadito *armadito)
{
	int event_mask;

	openlog("armadito-journal", LOG_PID, LOG_USER);

	event_mask = EVENT_DETECTION
		| EVENT_ON_DEMAND_START
		| EVENT_ON_DEMAND_COMPLETED
		| EVENT_QUARANTINE
		| EVENT_REAL_TIME_PROT
		| EVENT_AV_UPDATE;

	a6o_event_source_add_cb(a6o_get_event_source(armadito), event_mask, journal_event_cb, NULL);
}
