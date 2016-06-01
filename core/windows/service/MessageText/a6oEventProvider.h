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

 // Message Text File for armadito.
 // a6oEventProvider.mc
 // Header section
 // The following are the categories of events.
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: LIBARMADITO_CATEGORY
//
// MessageText:
//
// Libarmadito
//
#define LIBARMADITO_CATEGORY             ((WORD)0x00000001L)

//
// MessageId: MODULES_CATEGORY
//
// MessageText:
//
// Modules
//
#define MODULES_CATEGORY                 ((WORD)0x00000002L)

//
// MessageId: SERVICE_CATEGORY
//
// MessageText:
//
// Service
//
#define SERVICE_CATEGORY                 ((WORD)0x00000003L)

 // The following are the message definitions.
 // Error messages
//
// MessageId: MSG_ERROR
//
// MessageText:
//
// ArmaditoAV :: Error :: %1
//
#define MSG_ERROR                        ((DWORD)0xC0000100L)

 // Warning messages
//
// MessageId: MSG_WARNING
//
// MessageText:
//
// ArmaditoAV :: Warning :: %1
//
#define MSG_WARNING                      ((DWORD)0x80000200L)

 // Information messages
//
// MessageId: MSG_INFO
//
// MessageText:
//
// ArmaditoAV :: Information :: %1
//
#define MSG_INFO                         ((DWORD)0x40000300L)

 // Success messages
//
// MessageId: MSG_SUCCESS
//
// MessageText:
//
// ArmaditoAV :: Success :: %1
//
#define MSG_SUCCESS                      ((DWORD)0x00000400L)

