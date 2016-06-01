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

#ifndef _NETDEFAULTS_H_
#define _NETDEFAULTS_H_

/* for compatibility with node.js, we don't use abstract socket address (see man 7 unix) */
/* despite the fact that they have the advantage that they do not require to remove the  */
/* file before bind()ing */
#define DEFAULT_SOCKET_PATH   "/tmp/.armadito-daemon"
/* use this definition for abstract socket address */
/* #define DEFAULT_SOCKET_PATH   "@/tmp/.armadito-daemon" */

#endif
