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

#ifndef ARMADITO_H
#define ARMADITO_H

#if defined _WIN32
#define A6O_DLL_IMPORT __declspec(dllimport)
#define A6O_DLL_EXPORT __declspec(dllexport)
#define A6O_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define A6O_DLL_IMPORT __attribute__ ((visibility ("default")))
#define A6O_DLL_EXPORT __attribute__ ((visibility ("default")))
#define A6O_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define A6O_DLL_IMPORT
#define A6O_DLL_EXPORT
#define A6O_DLL_LOCAL
#endif
#endif
#ifdef A6O_DLL_EXPORTS // defined if we are building the ARMADITO DLL (instead of using it)
#define A6O_API A6O_DLL_EXPORT
#else
#define A6O_API A6O_DLL_IMPORT
#endif
#define A6O_LOCAL A6O_DLL_LOCAL

#ifdef __cplusplus
extern "C" {
#endif

#include <libarmadito/log.h>
#include <libarmadito/string.h>
#include <libarmadito/confval.h>
#include <libarmadito/status.h>
#include <libarmadito/info.h>
#include <libarmadito/module.h>
#include <libarmadito/stdpaths.h>

#ifdef __cplusplus
}
#endif

#endif
