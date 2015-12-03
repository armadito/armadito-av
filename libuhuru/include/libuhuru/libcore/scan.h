/**
 * \file scan.h
 *
 * \brief definition of the scan API
 *
 * The scan API allows to scan files or directories, possibly using a pool of thread.
 * 
 * A scan assigns a uhuru_file_status to a file or to each file located under a directory, by calling the scan function of the Uhuru modules.
 * During the scan, callbacks are used 
 *
 * Basic operations are:
 * - create a scan using uhuru_scan_new()
 * - add callbacks to the created scan using uhuru_scan_add_callback(); callbacks are functions that will be called after
 * a file has been scanned by all the scan modules
 * - launch the scan and wait for its completion using uhuru_scan_run(); during the execution of this function, callbacks may be called
 * - free the scan using uhuru_scan_free()
 *
 * A helper function is provided for on-access scan; this function does not create threads and does not allow to register callbacks.
 *
 */

#ifndef _LIBUHURU_LIBCORE_SCAN_H_
#define _LIBUHURU_LIBCORE_SCAN_H_

#include <libuhuru/common/status.h>
#include <libuhuru/libcore/handle.h>
#include <libuhuru/libcore/filectx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct struct uhuru_report
 * \brief a structure containing the result of a file scan
 *
 * This structure is filled during a scan and passed to the scan callbacks after a
 * file has been scanned.
 *
 */

#define REPORT_PROGRESS_UNKNOWN (-1)

struct uhuru_report {
  int scan_id;                          /*!< the id of the scan this report belongs to                          */
  char *path;                           /*!< the path of the scanned file                                       */
  int progress;                         /*!< the progress, can be an int (0 <= <= 100), or PROGRESS_UNKNOWN     */
  enum uhuru_file_status status;        /*!< the scan status of the file (i.e. clean, malware, etc)             */
  enum uhuru_action action;             /*!< the action that was executed on this file (alert, quarantine, etc) */
  char *mod_name;                       /*!< name of the module that decided the file scan status               */
  char *mod_report;                     /*!< the report of this module, usually a malware name                  */
};

/**
 * \struct struct uhuru_scan
 * \brief an opaque structure for file or directory scanning
 */
struct uhuru_scan;

enum uhuru_scan_flags {
  UHURU_SCAN_THREADED   = 1 << 0,
  UHURU_SCAN_RECURSE    = 1 << 1,
  UHURU_SCAN_STANDARD   = UHURU_SCAN_THREADED | UHURU_SCAN_RECURSE,
};

/**
 * \fn struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, int scan_id, const char *root_path, enum uhuru_scan_flags flags);
 * \brief allocate and initialize a scan
 *
 * This function allocates and initializes a scan, but does not start it.
 * BEWARE: after calling this function, the scan has not started. It must be started with uhuru_scan_run()
 *
 * This function uses malloc() to allocate the structure
 *
 * \param[in] uhuru        uhuru handle that was returned by uhuru_open()
 * \param[in] scan_id      the scan id for the user interface
 * \param[in] root_path    the root path of the scan, can be a file or a directory
 * \param[in] flags        the scan flags, specifying in particular if directory must be scanned recursively. It is recommended to pass UHURU_SCAN_STANDARD as flags value
 *
 * \return  a pointer to the scan opaque structure
 *
 */
struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, int scan_id, const char *path, enum uhuru_scan_flags flags);

/**
 * \var typedef void (*uhuru_scan_callback_t)(struct uhuru_report *report, void *callback_data);
 * \brief the type definition for a scan callback
 *
 * A scan callback will be called during a scan.
 * The callbacks are called:
 * - after the definitive scan status of a file is known
 * - and after all the scan modules have done their work
 * 
 * BEWARE: a scan callback can be called from a different thread than the thread in which uhuru_scan_run was called
 * 
 * A callback arguments are:
 * \param[in] report          a pointer to a uhuru_report structure (see above)
 * \param[in] callback_data   a generic pointer, the same that was passed to uhuru_scan_add_callback
 * 
 */
typedef void (*uhuru_scan_callback_t)(struct uhuru_report *report, void *callback_data);

/**
 * \fn void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data);
 * \brief adds a callback to the scan
 *
 * This function registers a callback for the scan.
 * The registered callback will be called from the scan threads, once the scan has been started by uhuru_scan_run()
 *
 * \param[in] scan            pointer to the scan opaque structure
 * \param[in] callback        the callback to be registered
 * \param[in] callback_data   a generic pointer that will be passed to the callback call
 *
 */
void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data);

/**
 * \fn void uhuru_scan_run(struct uhuru_scan *scan);
 * \brief runs a scan
 *
 * This function starts a scan and waits for its completion.
 * After each file scan, the callbacks that were registered by uhuru_scan_add_callback() are called
 * This function can start threads if UHURU_SCAN_THREADED was defined in scan's flags
 * This function will recurse into sub-directories if UHURU_SCAN_RECURSE was defined in scan's flags
 * Execution of this function can be lengthy, if scanning large directories.
 * BEWARE: calling this function will *block* caller, even if scan is multi-threaded
 *
 * \param[in] scan            pointer to the scan opaque structure
 *
 */
void uhuru_scan_run(struct uhuru_scan *scan);

/**
 * \fn void uhuru_scan_run(struct uhuru_scan *scan);
 * \brief frees a scan
 *
 * This function frees a scan. The scan must be completed, i.e. uhuru_scan_run() must have been called
 *
 * \param[in] scan            pointer to the scan opaque structure
 *
 */
void uhuru_scan_free(struct uhuru_scan *scan);

/**
 * \fn enum uhuru_file_status uhuru_scan_simple(struct uhuru *uhuru, const char *path);
 * \brief scan a single file, without callbacks
 *
 * This function makes a simple scan, to scan a single file.
 * It is dedicated to on-access scan, where the scan status must be available directly.
 * BEWARE: the scan will be run inside the caller's thread.
 *
 * \param[in] uhuru        uhuru handle that was returned by uhuru_open()
 * \param[in] path         path of file to scan, must be a regular file
 *
 * \return  the scan status of the file, as defined in file status.h
 *
 */
enum uhuru_file_status uhuru_scan_simple(struct uhuru *uhuru, const char *path, struct uhuru_report * report);

enum uhuru_file_status uhuru_scan_context(struct uhuru_file_context *ctx,  struct uhuru_report *report);

#ifdef __cplusplus
}
#endif

#endif
