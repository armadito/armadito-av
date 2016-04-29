/**
 * \file scan.h
 *
 * \brief definition of the scan API
 *
 * The scan API allows to scan files or directories, possibly using a pool of thread.
 *
 * A scan assigns a a6o_file_status to a file or to each file located under a directory, by calling the scan function of the Uhuru modules.
 * During the scan, callbacks are used
 *
 * Basic operations are:
 * - create a scan using a6o_scan_new()
 * - add callbacks to the created scan using a6o_scan_add_callback(); callbacks are functions that will be called after
 * a file has been scanned by all the scan modules
 * - launch the scan and wait for its completion using a6o_scan_run(); during the execution of this function, callbacks may be called
 * - free the scan using a6o_scan_free()
 *
 * A helper function is provided for on-access scan; this function does not create threads and does not allow to register callbacks.
 *
 */

#ifndef _LIBARMADITO_SCAN_H_
#define _LIBARMADITO_SCAN_H_

#include <libarmadito/status.h>
#include <libarmadito/handle.h>
#include <libarmadito/filectx.h>

/**
 * \struct struct a6o_report
 * \brief a structure containing the result of a file scan
 *
 * This structure is filled during a scan and passed to the scan callbacks after a
 * file has been scanned.
 *
 */

#define REPORT_PROGRESS_UNKNOWN (-1)

struct a6o_report {
	int scan_id;                          /*!< the id of the scan this report belongs to                          */
	char *path;                           /*!< the path of the scanned file                                       */
	int progress;                         /*!< the progress, can be an int (0 <= <= 100), or PROGRESS_UNKNOWN     */
	enum a6o_file_status status;        /*!< the scan status of the file (i.e. clean, malware, etc)             */
	enum a6o_action action;             /*!< the action that was executed on this file (alert, quarantine, etc) */
	char *mod_name;                       /*!< name of the module that decided the file scan status               */
	char *mod_report;                     /*!< the report of this module, usually a malware name                  */
	int malware_count;                   /*!<  number of malwares detected since scan started                     */
	int suspicious_count;                 /*!< number of suspicious files detected since scan started            */
	int scanned_count;                    /*!< number of scanned files */ 
};

/**
 * \struct struct a6o_scan
 * \brief an opaque structure for file or directory scanning
 */
struct a6o_scan;

enum a6o_scan_flags {
	ARMADITO_SCAN_THREADED   = 1 << 0,
	ARMADITO_SCAN_RECURSE    = 1 << 1,
	ARMADITO_SCAN_STANDARD   = ARMADITO_SCAN_THREADED | ARMADITO_SCAN_RECURSE,
};

/**
 * \fn struct a6o_scan *a6o_scan_new(struct armadito *armadito, int scan_id, const char *root_path, enum a6o_scan_flags flags);
 * \brief allocate and initialize a scan
 *
 * This function allocates and initializes a scan, but does not start it.
 * BEWARE: after calling this function, the scan has not started. It must be started with a6o_scan_run()
 *
 * This function uses malloc() to allocate the structure
 *
 * \param[in] armadito        armadito handle that was returned by a6o_open()
 * \param[in] scan_id      the scan id for the user interface
 * \param[in] root_path    the root path of the scan, can be a file or a directory
 * \param[in] flags        the scan flags, specifying in particular if directory must be scanned recursively. It is recommended to pass ARMADITO_SCAN_STANDARD as flags value
 *
 * \return  a pointer to the scan opaque structure
 *
 */
struct a6o_scan *a6o_scan_new(struct armadito *armadito, int scan_id);

/**
 * \var typedef void (*a6o_scan_callback_t)(struct a6o_report *report, void *callback_data);
 * \brief the type definition for a scan callback
 *
 * A scan callback will be called during a scan.
 * The callbacks are called:
 * - after the definitive scan status of a file is known
 * - and after all the scan modules have done their work
 *
 * BEWARE: a scan callback can be called from a different thread than the thread in which a6o_scan_run was called
 *
 * A callback arguments are:
 * \param[in] report          a pointer to a a6o_report structure (see above)
 * \param[in] callback_data   a generic pointer, the same that was passed to a6o_scan_add_callback
 *
 */
typedef void (*a6o_scan_callback_t)(struct a6o_report *report, void *callback_data);

/**
 * \fn void a6o_scan_add_callback(struct a6o_scan *scan, a6o_scan_callback_t callback, void *callback_data);
 * \brief adds a callback to the scan
 *
 * This function registers a callback for the scan.
 * The registered callback will be called from the scan threads, once the scan has been started by a6o_scan_run()
 *
 * \param[in] scan            pointer to the scan opaque structure
 * \param[in] callback        the callback to be registered
 * \param[in] callback_data   a generic pointer that will be passed to the callback call
 *
 */
void a6o_scan_add_callback(struct a6o_scan *scan, a6o_scan_callback_t callback, void *callback_data);

/**
 * \fn void a6o_scan_run(struct a6o_scan *scan);
 * \brief runs a scan
 *
 * This function starts a scan and waits for its completion.
 * After each file scan, the callbacks that were registered by a6o_scan_add_callback() are called
 * This function can start threads if ARMADITO_SCAN_THREADED was defined in scan's flags
 * This function will recurse into sub-directories if ARMADITO_SCAN_RECURSE was defined in scan's flags
 * Execution of this function can be lengthy, if scanning large directories.
 * BEWARE: calling this function will *block* caller, even if scan is multi-threaded
 *
 * \param[in] scan            pointer to the scan opaque structure
 *
 */
/* void a6o_scan_run(struct a6o_scan *scan); */

enum a6o_file_status a6o_scan_context(struct a6o_scan *scan, struct a6o_file_context *ctx);

/**
 * \fn void a6o_scan_run(struct a6o_scan *scan);
 * \brief frees a scan
 *
 * This function frees a scan. The scan must be completed, i.e. a6o_scan_run() must have been called
 *
 * \param[in] scan            pointer to the scan opaque structure
 *
 */
void a6o_scan_free(struct a6o_scan *scan);

/**
 * \fn enum a6o_file_status a6o_scan_simple(struct armadito *armadito, const char *path);
 * \brief scan a single file, without callbacks
 *
 * This function makes a simple scan, to scan a single file.
 * It is dedicated to on-access scan, where the scan status must be available directly.
 * BEWARE: the scan will be run inside the caller's thread.
 *
 * \param[in] armadito        armadito handle that was returned by a6o_open()
 * \param[in] path         path of file to scan, must be a regular file
 *
 * \return  the scan status of the file, as defined in file status.h
 *
 */
enum a6o_file_status a6o_scan_simple(struct armadito *armadito, const char *path, struct a6o_report * report);

#endif
