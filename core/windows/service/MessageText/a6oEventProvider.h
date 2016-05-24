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
// Armadito-av :: Error :: %1
//
#define MSG_ERROR                        ((DWORD)0xC0000100L)

 // Warning messages
//
// MessageId: MSG_WARNING
//
// MessageText:
//
// Armadito-av :: Warning :: %1
//
#define MSG_WARNING                      ((DWORD)0x80000200L)

 // Information messages
//
// MessageId: MSG_INFO
//
// MessageText:
//
// Armadito-av :: Information :: %1
//
#define MSG_INFO                         ((DWORD)0x40000300L)

 // Success messages
//
// MessageId: MSG_SUCCESS
//
// MessageText:
//
// Armadito-av :: Success :: %1
//
#define MSG_SUCCESS                      ((DWORD)0x00000400L)

