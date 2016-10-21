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

#include <libarmadito.h>

#include "armadito-config.h"

const char *a6o_action_pretty_str(enum a6o_action action)
{
	switch(action & (ARMADITO_ACTION_ALERT | ARMADITO_ACTION_QUARANTINE | ARMADITO_ACTION_REMOVE)) {
	case ARMADITO_ACTION_ALERT:
		return "alert";
	case ARMADITO_ACTION_ALERT | ARMADITO_ACTION_QUARANTINE:
		return "alert+quarantine";
	case ARMADITO_ACTION_ALERT | ARMADITO_ACTION_REMOVE:
		return "alert+removed";
	}

	return "none";
}
