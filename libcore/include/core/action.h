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

#ifndef ARMADITO_CORE_ACTION_H
#define ARMADITO_CORE_ACTION_H

enum a6o_action {
	A6O_ACTION_NONE         = 0,
	A6O_ACTION_ALERT        = 1 << 1,
	A6O_ACTION_QUARANTINE   = 1 << 2,
	A6O_ACTION_REMOVE       = 1 << 3,
};

const char *a6o_action_pretty_str(enum a6o_action action);

#endif
