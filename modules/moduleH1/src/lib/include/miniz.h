/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

/* miniz.c v1.15 - public domain deflate/inflate, zlib-subset, ZIP reading/writing/appending, PNG writing
   See "unlicense" statement at the end of this file.
   Rich Geldreich <richgel99@gmail.com>, last updated Oct. 13, 2013
   Implements RFC 1950: http:

   Most API's defined in miniz.c are optional. For example, to disable the archive related functions just define
   MINIZ_NO_ARCHIVE_APIS, or to get rid of all stdio usage define MINIZ_NO_STDIO (see the list below for more macros).

   * Change History
   10/13/13 v1.15 r4 - Interim bugfix release while I work on the next major release with Zip64 support (almost there!):
   - Critical fix for the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY bug (thanks kahmyong.moon@hp.com) which could cause locate files to not find files. This bug
   would only have occured in earlier versions if you explicitly used this flag, OR if you used mz_zip_extract_archive_file_to_heap() or mz_zip_add_mem_to_archive_file_in_place()
   (which used this flag). If you can't switch to v1.15 but want to fix this bug, just remove the uses of this flag from both helper funcs (and of course don't use the flag).
   - Bugfix in mz_zip_reader_extract_to_mem_no_alloc() from kymoon when pUser_read_buf is not NULL and compressed size is > uncompressed size
   - Fixing mz_zip_reader_extract_*() funcs so they don't try to extract compressed data from directory entries, to account for weird zipfiles which contain zero-size compressed data on dir entries.
   Hopefully this fix won't cause any issues on weird zip archives, because it assumes the low 16-bits of zip external attributes are DOS attributes (which I believe they always are in practice).
   - Fixing mz_zip_reader_is_file_a_directory() so it doesn't check the internal attributes, just the filename and external attributes
   - mz_zip_reader_init_file() - missing MZ_FCLOSE() call if the seek failed
   - Added cmake support for Linux builds which builds all the examples, tested with clang v3.3 and gcc v4.6.
   - Clang fix for tdefl_write_image_to_png_file_in_memory() from toffaletti
   - Merged MZ_FORCEINLINE fix from hdeanclark
   - Fix <time.h> include before config #ifdef, thanks emil.brink
   - Added tdefl_write_image_to_png_file_in_memory_ex(): supports Y flipping (super useful for OpenGL apps), and explicit control over the compression level (so you can
   set it to 1 for real-time compression).
   - Merged in some compiler fixes from paulharris's github repro.
   - Retested this build under Windows (VS 2010, including static analysis), tcc  0.9.26, gcc v4.6 and clang v3.3.
   - Added example6.c, which dumps an image of the mandelbrot set to a PNG file.
   - Modified example2 to help test the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY flag more.
   - In r3: Bugfix to mz_zip_writer_add_file() found during merge: Fix possible src file fclose() leak if alignment bytes+local header file write faiiled
   - In r4: Minor bugfix to mz_zip_writer_add_from_zip_reader(): Was pushing the wrong central dir header offset, appears harmless in this release, but it became a problem in the zip64 branch
   5/20/12 v1.14 - MinGW32/64 GCC 4.6.1 compiler fixes: added MZ_FORCEINLINE, #include <time.h> (thanks fermtect).
   5/19/12 v1.13 - From jason@cornsyrup.org and kelwert@mtu.edu - Fix mz_crc32() so it doesn't compute the wrong CRC-32's when mz_ulong is 64-bit.
   - Temporarily/locally slammed in "typedef unsigned long mz_ulong" and re-ran a randomized regression test on ~500k files.
   - Eliminated a bunch of warnings when compiling with GCC 32-bit/64.
   - Ran all examples, miniz.c, and tinfl.c through MSVC 2008's /analyze (static analysis) option and fixed all warnings (except for the silly
   "Use of the comma-operator in a tested expression.." analysis warning, which I purposely use to work around a MSVC compiler warning).
   - Created 32-bit and 64-bit Codeblocks projects/workspace. Built and tested Linux executables. The codeblocks workspace is compatible with Linux+Win32/x64.
   - Added miniz_tester solution/project, which is a useful little app derived from LZHAM's tester app that I use as part of the regression test.
   - Ran miniz.c and tinfl.c through another series of regression testing on ~500,000 files and archives.
   - Modified example5.c so it purposely disables a bunch of high-level functionality (MINIZ_NO_STDIO, etc.). (Thanks to corysama for the MINIZ_NO_STDIO bug report.)
   - Fix ftell() usage in examples so they exit with an error on files which are too large (a limitation of the examples, not miniz itself).
   4/12/12 v1.12 - More comments, added low-level example5.c, fixed a couple minor level_and_flags issues in the archive API's.
   level_and_flags can now be set to MZ_DEFAULT_COMPRESSION. Thanks to Bruce Dawson <bruced@valvesoftware.com> for the feedback/bug report.
   5/28/11 v1.11 - Added statement from unlicense.org
   5/27/11 v1.10 - Substantial compressor optimizations:
   - Level 1 is now ~4x faster than before. The L1 compressor's throughput now varies between 70-110MB/sec. on a
   - Core i7 (actual throughput varies depending on the type of data, and x64 vs. x86).
   - Improved baseline L2-L9 compression perf. Also, greatly improved compression perf. issues on some file types.
   - Refactored the compression code for better readability and maintainability.
   - Added level 10 compression level (L10 has slightly better ratio than level 9, but could have a potentially large
   drop in throughput on some files).
   5/15/11 v1.09 - Initial stable release.

   * Low-level Deflate/Inflate implementation notes:

   Compression: Use the "tdefl" API's. The compressor supports raw, static, and dynamic blocks, lazy or
   greedy parsing, match length filtering, RLE-only, and Huffman-only streams. It performs and compresses
   approximately as well as zlib.

   Decompression: Use the "tinfl" API's. The entire decompressor is implemented as a single function
   coroutine: see tinfl_decompress(). It supports decompression into a 32KB (or larger power of 2) wrapping buffer, or into a memory
   block large enough to hold the entire file.

   The low-level tdefl/tinfl API's do not make any use of dynamic memory allocation.

   * zlib-style API notes:

   miniz.c implements a fairly large subset of zlib. There's enough functionality present for it to be a drop-in
   zlib replacement in many apps:
   The z_stream struct, optional memory allocation callbacks
   deflateInit/deflateInit2/deflate/deflateReset/deflateEnd/deflateBound
   inflateInit/inflateInit2/inflate/inflateEnd
   compress, compress2, compressBound, uncompress
   CRC-32, Adler-32 - Using modern, minimal code size, CPU cache friendly routines.
   Supports raw deflate streams or standard zlib streams with adler-32 checking.

   Limitations:
   The callback API's are not implemented yet. No support for gzip headers or zlib static dictionaries.
   I've tried to closely emulate zlib's various flavors of stream flushing and return status codes, but
   there are no guarantees that miniz.c pulls this off perfectly.

   * PNG writing: See the tdefl_write_image_to_png_file_in_memory() function, originally written by
   Alex Evans. Supports 1-4 bytes/pixel images.

   * ZIP archive API notes:

   The ZIP archive API's where designed with simplicity and efficiency in mind, with just enough abstraction to
   get the job done with minimal fuss. There are simple API's to retrieve file information, read files from
   existing archives, create new archives, append new files to existing archives, or clone archive data from
   one archive to another. It supports archives located in memory or the heap, on disk (using stdio.h),
   or you can specify custom file read/write callbacks.

   - Archive reading: Just call this function to read a single file from a disk archive:

   void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name,
   size_t *pSize, mz_uint zip_flags);

   For more complex cases, use the "mz_zip_reader" functions. Upon opening an archive, the entire central
   directory is located and read as-is into memory, and subsequent file access only occurs when reading individual files.

   - Archives file scanning: The simple way is to use this function to scan a loaded archive for a specific file:

   int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

   The locate operation can optionally check file comments too, which (as one example) can be used to identify
   multiple versions of the same file in an archive. This function uses a simple linear search through the central
   directory, so it's not very fast.

   Alternately, you can iterate through all the files in an archive (using mz_zip_reader_get_num_files()) and
   retrieve detailed info on each file by calling mz_zip_reader_file_stat().

   - Archive creation: Use the "mz_zip_writer" functions. The ZIP writer immediately writes compressed file data
   to disk and builds an exact image of the central directory in memory. The central directory image is written
   all at once at the end of the archive file when the archive is finalized.

   The archive writer can optionally align each file's local header and file data to any power of 2 alignment,
   which can be useful when the archive will be read from optical media. Also, the writer supports placing
   arbitrary data blobs at the very beginning of ZIP archives. Archives written using either feature are still
   readable by any ZIP tool.

   - Archive appending: The simple way to add a single file to an archive is to call this function:

   mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name,
   const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

   The archive will be created if it doesn't already exist, otherwise it'll be appended to.
   Note the appending is done in-place and is not an atomic operation, so if something goes wrong
   during the operation it's possible the archive could be left without a central directory (although the local
   file headers and file data will be fine, so the archive will be recoverable).

   For more complex archive modification scenarios:
   1. The safest way is to use a mz_zip_reader to read the existing archive, cloning only those bits you want to
   preserve into a new archive using using the mz_zip_writer_add_from_zip_reader() function (which compiles the
   compressed file data as-is). When you're done, delete the old archive and rename the newly written archive, and
   you're done. This is safe but requires a bunch of temporary disk space or heap memory.

   2. Or, you can convert an mz_zip_reader in-place to an mz_zip_writer using mz_zip_writer_init_from_reader(),
   append new files as needed, then finalize the archive which will write an updated central directory to the
   original archive. (This is basically what mz_zip_add_mem_to_archive_file_in_place() does.) There's a
   possibility that the archive's central directory could be lost with this method if anything goes wrong, though.

   - ZIP archive support limitations:
   No zip64 or spanning support. Extraction functions can only handle unencrypted, stored or deflated files.
   Requires streams capable of seeking.

   * This is a header file library, like stb_image.c. To get only a header file, either cut and paste the
   below header, or create miniz.h, #define MINIZ_HEADER_FILE_ONLY, and then include miniz.c from it.

   * Important: For best perf. be sure to customize the below macros for your target platform:
   #define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
   #define MINIZ_LITTLE_ENDIAN 1
   #define MINIZ_HAS_64BIT_REGISTERS 1

   * On platforms using glibc, Be sure to "#define _LARGEFILE64_SOURCE 1" before including miniz.c to ensure miniz
   uses the 64-bit variants: fopen64(), stat64(), etc. Otherwise you won't be able to process large files
   (i.e. 32-bit stat() fails for me on files > 0x7FFFFFFF bytes).
   */

#ifndef MINIZ_HEADER_INCLUDED
#define MINIZ_HEADER_INCLUDED

// If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
// get/set file times, and the C run-time funcs that get/set times won't be called.
// The current downside is the times written to your archives will be from 1979.
#define MINIZ_NO_TIME

// Define MINIZ_NO_ARCHIVE_APIS to disable all writing related ZIP archive API's.
#define MINIZ_NO_ARCHIVE_WRITING_APIS

// Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's.
//#define MINIZ_NO_ZLIB_APIS

// Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib.
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#if defined(__TINYC__) && (defined(__linux) || defined(__linux__))
#define MINIZ_NO_TIME
#endif

#if !defined(MINIZ_NO_TIME) && !defined(MINIZ_NO_ARCHIVE_APIS)
#include <time.h>
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
#define MINIZ_X86_OR_X64_CPU 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
#define MINIZ_LITTLE_ENDIAN 1
#endif

#if MINIZ_X86_OR_X64_CPU
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
#define MINIZ_HAS_64BIT_REGISTERS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef unsigned long mz_ulong;

	void mz_free(void *p);

#define MZ_ADLER32_INIT (1)
	mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define MZ_CRC32_INIT (0)
	mz_ulong mz_crc32(mz_ulong crc, const unsigned char *ptr, size_t buf_len);

	enum { MZ_DEFAULT_STRATEGY = 0, MZ_FILTERED = 1, MZ_HUFFMAN_ONLY = 2, MZ_RLE = 3, MZ_FIXED = 4 };

#define MZ_DEFLATED 8

#ifndef MINIZ_NO_ZLIB_APIS

	typedef void *(*mz_alloc_func)(void *opaque, size_t items, size_t size);
	typedef void(*mz_free_func)(void *opaque, void *address);
	typedef void *(*mz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

#define MZ_VERSION          "9.1.15"
#define MZ_VERNUM           0x91F0
#define MZ_VER_MAJOR        9
#define MZ_VER_MINOR        1
#define MZ_VER_REVISION     15
#define MZ_VER_SUBREVISION  0

	enum { MZ_NO_FLUSH = 0, MZ_PARTIAL_FLUSH = 1, MZ_SYNC_FLUSH = 2, MZ_FULL_FLUSH = 3, MZ_FINISH = 4, MZ_BLOCK = 5 };

	enum { MZ_OK = 0, MZ_STREAM_END = 1, MZ_NEED_DICT = 2, MZ_ERRNO = -1, MZ_STREAM_ERROR = -2, MZ_DATA_ERROR = -3, MZ_MEM_ERROR = -4, MZ_BUF_ERROR = -5, MZ_VERSION_ERROR = -6, MZ_PARAM_ERROR = -10000 };

	enum { MZ_NO_COMPRESSION = 0, MZ_BEST_SPEED = 1, MZ_BEST_COMPRESSION = 9, MZ_UBER_COMPRESSION = 10, MZ_DEFAULT_LEVEL = 6, MZ_DEFAULT_COMPRESSION = -1 };

#define MZ_DEFAULT_WINDOW_BITS 15

	struct mz_internal_state;

	typedef struct mz_stream_s
	{
		const unsigned char *next_in;
		unsigned int avail_in;
		mz_ulong total_in;

		unsigned char *next_out;
		unsigned int avail_out;
		mz_ulong total_out;

		char *msg;
		struct mz_internal_state *state;

		mz_alloc_func zalloc;
		mz_free_func zfree;
		void *opaque;

		int data_type;
		mz_ulong adler;
		mz_ulong reserved;
	} mz_stream;

	typedef mz_stream *mz_streamp;

	const char *mz_version(void);

	int mz_deflateInit(mz_streamp pStream, int level);

	int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy);

	int mz_deflateReset(mz_streamp pStream);

	int mz_deflate(mz_streamp pStream, int flush);

	int mz_deflateEnd(mz_streamp pStream);

	mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len);

	int mz_compress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);
	int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level);

	mz_ulong mz_compressBound(mz_ulong source_len);

	int mz_inflateInit(mz_streamp pStream);

	int mz_inflateInit2(mz_streamp pStream, int window_bits);

	int mz_inflate(mz_streamp pStream, int flush);

	int mz_inflateEnd(mz_streamp pStream);

	int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);

	const char *mz_error(int err);

#ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES
	typedef unsigned char Byte;
	typedef unsigned int uInt;
	typedef mz_ulong uLong;
	typedef Byte Bytef;
	typedef uInt uIntf;
	typedef char charf;
	typedef int intf;
	typedef void *voidpf;
	typedef uLong uLongf;
	typedef void *voidp;
	typedef void *const voidpc;
#define Z_NULL                0
#define Z_NO_FLUSH            MZ_NO_FLUSH
#define Z_PARTIAL_FLUSH       MZ_PARTIAL_FLUSH
#define Z_SYNC_FLUSH          MZ_SYNC_FLUSH
#define Z_FULL_FLUSH          MZ_FULL_FLUSH
#define Z_FINISH              MZ_FINISH
#define Z_BLOCK               MZ_BLOCK
#define Z_OK                  MZ_OK
#define Z_STREAM_END          MZ_STREAM_END
#define Z_NEED_DICT           MZ_NEED_DICT
#define Z_ERRNO               MZ_ERRNO
#define Z_STREAM_ERROR        MZ_STREAM_ERROR
#define Z_DATA_ERROR          MZ_DATA_ERROR
#define Z_MEM_ERROR           MZ_MEM_ERROR
#define Z_BUF_ERROR           MZ_BUF_ERROR
#define Z_VERSION_ERROR       MZ_VERSION_ERROR
#define Z_PARAM_ERROR         MZ_PARAM_ERROR
#define Z_NO_COMPRESSION      MZ_NO_COMPRESSION
#define Z_BEST_SPEED          MZ_BEST_SPEED
#define Z_BEST_COMPRESSION    MZ_BEST_COMPRESSION
#define Z_DEFAULT_COMPRESSION MZ_DEFAULT_COMPRESSION
#define Z_DEFAULT_STRATEGY    MZ_DEFAULT_STRATEGY
#define Z_FILTERED            MZ_FILTERED
#define Z_HUFFMAN_ONLY        MZ_HUFFMAN_ONLY
#define Z_RLE                 MZ_RLE
#define Z_FIXED               MZ_FIXED
#define Z_DEFLATED            MZ_DEFLATED
#define Z_DEFAULT_WINDOW_BITS MZ_DEFAULT_WINDOW_BITS
#define alloc_func            mz_alloc_func
#define free_func             mz_free_func
#define internal_state        mz_internal_state
#define z_stream              mz_stream
#define deflateInit           mz_deflateInit
#define deflateInit2          mz_deflateInit2
#define deflateReset          mz_deflateReset
#define deflate               mz_deflate
#define deflateEnd            mz_deflateEnd
#define deflateBound          mz_deflateBound
#define compress              mz_compress
#define compress2             mz_compress2
#define compressBound         mz_compressBound
#define inflateInit           mz_inflateInit
#define inflateInit2          mz_inflateInit2
#define inflate               mz_inflate
#define inflateEnd            mz_inflateEnd
#define uncompress            mz_uncompress
#define crc32                 mz_crc32
#define adler32               mz_adler32
#define MAX_WBITS             15
#define MAX_MEM_LEVEL         9
#define zError                mz_error
#define ZLIB_VERSION          MZ_VERSION
#define ZLIB_VERNUM           MZ_VERNUM
#define ZLIB_VER_MAJOR        MZ_VER_MAJOR
#define ZLIB_VER_MINOR        MZ_VER_MINOR
#define ZLIB_VER_REVISION     MZ_VER_REVISION
#define ZLIB_VER_SUBREVISION  MZ_VER_SUBREVISION
#define zlibVersion           mz_version
#define zlib_version          mz_version()
#endif

#endif

	typedef unsigned char mz_uint8;
	typedef signed short mz_int16;
	typedef unsigned short mz_uint16;
	typedef unsigned int mz_uint32;
	typedef unsigned int mz_uint;
	typedef long long mz_int64;
	typedef unsigned long long mz_uint64;
	typedef int mz_bool;

#define MZ_FALSE (0)
#define MZ_TRUE (1)

#ifdef _MSC_VER
#define MZ_MACRO_END while (0, 0)
#else
#define MZ_MACRO_END while (0)
#endif

#ifndef MINIZ_NO_ARCHIVE_APIS

	enum
	{
		MZ_ZIP_MAX_IO_BUF_SIZE = 64 * 1024,
		MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 260,
		MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 256
	};

	typedef struct
	{
		mz_uint32 m_file_index;
		mz_uint32 m_central_dir_ofs;
		mz_uint16 m_version_made_by;
		mz_uint16 m_version_needed;
		mz_uint16 m_bit_flag;
		mz_uint16 m_method;
#ifndef MINIZ_NO_TIME
		time_t m_time;
#endif
		mz_uint32 m_crc32;
		mz_uint64 m_comp_size;
		mz_uint64 m_uncomp_size;
		mz_uint16 m_internal_attr;
		mz_uint32 m_external_attr;
		mz_uint64 m_local_header_ofs;
		mz_uint32 m_comment_size;
		char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
		char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];
	} mz_zip_archive_file_stat;

	typedef size_t(*mz_file_read_func)(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n);
	typedef size_t(*mz_file_write_func)(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n);

	struct mz_zip_internal_state_tag;
	typedef struct mz_zip_internal_state_tag mz_zip_internal_state;

	typedef enum
	{
		MZ_ZIP_MODE_INVALID = 0,
		MZ_ZIP_MODE_READING = 1,
		MZ_ZIP_MODE_WRITING = 2,
		MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
	} mz_zip_mode;

	typedef struct mz_zip_archive_tag
	{
		mz_uint64 m_archive_size;
		mz_uint64 m_central_directory_file_ofs;
		mz_uint m_total_files;
		mz_zip_mode m_zip_mode;

		mz_uint m_file_offset_alignment;

		mz_alloc_func m_pAlloc;
		mz_free_func m_pFree;
		mz_realloc_func m_pRealloc;
		void *m_pAlloc_opaque;

		mz_file_read_func m_pRead;
		mz_file_write_func m_pWrite;
		void *m_pIO_opaque;

		mz_zip_internal_state *m_pState;
	} mz_zip_archive;

	typedef enum
	{
		MZ_ZIP_FLAG_CASE_SENSITIVE = 0x0100,
		MZ_ZIP_FLAG_IGNORE_PATH = 0x0200,
		MZ_ZIP_FLAG_COMPRESSED_DATA = 0x0400,
		MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800
	} mz_zip_flags;

	mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 flags);
	mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 flags);

#ifndef MINIZ_NO_STDIO
	mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint32 flags);
#endif

	mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip);

	mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index, mz_zip_archive_file_stat *pStat);

	mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip, mz_uint file_index);
	mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip, mz_uint file_index);

	mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index, char *pFilename, mz_uint filename_buf_size);

	int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

	mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
	mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

	mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags);
	mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags);

	void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index, size_t *pSize, mz_uint flags);
	void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip, const char *pFilename, size_t *pSize, mz_uint flags);

	mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);
	mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);

#ifndef MINIZ_NO_STDIO

	mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags);
	mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, mz_uint flags);
#endif

	mz_bool mz_zip_reader_end(mz_zip_archive *pZip);

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

	mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size);
	mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size);

#ifndef MINIZ_NO_STDIO
	mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint64 size_to_reserve_at_beginning);
#endif

	mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip, const char *pFilename);

	mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, mz_uint level_and_flags);
	mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32);

#ifndef MINIZ_NO_STDIO

	mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);
#endif

	mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint file_index);

	mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip);
	mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **pBuf, size_t *pSize);

	mz_bool mz_zip_writer_end(mz_zip_archive *pZip);

	mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

	void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint zip_flags);

#endif

#endif

	enum
	{
		TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
		TINFL_FLAG_HAS_MORE_INPUT = 2,
		TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
		TINFL_FLAG_COMPUTE_ADLER32 = 8
	};

	void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
	size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

	typedef int(*tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
	int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

	struct tinfl_decompressor_tag; typedef struct tinfl_decompressor_tag tinfl_decompressor;

#define TINFL_LZ_DICT_SIZE 32768

	typedef enum
	{
		TINFL_STATUS_BAD_PARAM = -3,
		TINFL_STATUS_ADLER32_MISMATCH = -2,
		TINFL_STATUS_FAILED = -1,
		TINFL_STATUS_DONE = 0,
		TINFL_STATUS_NEEDS_MORE_INPUT = 1,
		TINFL_STATUS_HAS_MORE_OUTPUT = 2
	} tinfl_status;

#define tinfl_init(r) do { (r)->m_state = 0; } MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

	tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags);

	enum
	{
		TINFL_MAX_HUFF_TABLES = 3, TINFL_MAX_HUFF_SYMBOLS_0 = 288, TINFL_MAX_HUFF_SYMBOLS_1 = 32, TINFL_MAX_HUFF_SYMBOLS_2 = 19,
		TINFL_FAST_LOOKUP_BITS = 10, TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
	};

	typedef struct
	{
		mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
		mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
	} tinfl_huff_table;

#if MINIZ_HAS_64BIT_REGISTERS
#define TINFL_USE_64BIT_BITBUF 1
#endif

#if TINFL_USE_64BIT_BITBUF
	typedef mz_uint64 tinfl_bit_buf_t;
#define TINFL_BITBUF_SIZE (64)
#else
	typedef mz_uint32 tinfl_bit_buf_t;
#define TINFL_BITBUF_SIZE (32)
#endif

	struct tinfl_decompressor_tag
	{
		mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
		tinfl_bit_buf_t m_bit_buf;
		size_t m_dist_from_out_buf_start;
		tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
		mz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
	};

#define TDEFL_LESS_MEMORY 0

	enum
	{
		TDEFL_HUFFMAN_ONLY = 0, TDEFL_DEFAULT_MAX_PROBES = 128, TDEFL_MAX_PROBES_MASK = 0xFFF
	};

	enum
	{
		TDEFL_WRITE_ZLIB_HEADER = 0x01000,
		TDEFL_COMPUTE_ADLER32 = 0x02000,
		TDEFL_GREEDY_PARSING_FLAG = 0x04000,
		TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
		TDEFL_RLE_MATCHES = 0x10000,
		TDEFL_FILTER_MATCHES = 0x20000,
		TDEFL_FORCE_ALL_STATIC_BLOCKS = 0x40000,
		TDEFL_FORCE_ALL_RAW_BLOCKS = 0x80000
	};

	void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

	size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

	void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip);
	void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

	typedef mz_bool(*tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);

	mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

	enum { TDEFL_MAX_HUFF_TABLES = 3, TDEFL_MAX_HUFF_SYMBOLS_0 = 288, TDEFL_MAX_HUFF_SYMBOLS_1 = 32, TDEFL_MAX_HUFF_SYMBOLS_2 = 19, TDEFL_LZ_DICT_SIZE = 32768, TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1, TDEFL_MIN_MATCH_LEN = 3, TDEFL_MAX_MATCH_LEN = 258 };

#if TDEFL_LESS_MEMORY
	enum { TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 12, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#else
	enum { TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 15, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#endif

	typedef enum
	{
		TDEFL_STATUS_BAD_PARAM = -2,
		TDEFL_STATUS_PUT_BUF_FAILED = -1,
		TDEFL_STATUS_OKAY = 0,
		TDEFL_STATUS_DONE = 1
	} tdefl_status;

	typedef enum
	{
		TDEFL_NO_FLUSH = 0,
		TDEFL_SYNC_FLUSH = 2,
		TDEFL_FULL_FLUSH = 3,
		TDEFL_FINISH = 4
	} tdefl_flush;

	typedef struct
	{
		tdefl_put_buf_func_ptr m_pPut_buf_func;
		void *m_pPut_buf_user;
		mz_uint m_flags, m_max_probes[2];
		int m_greedy_parsing;
		mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
		mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
		mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
		mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
		tdefl_status m_prev_return_status;
		const void *m_pIn_buf;
		void *m_pOut_buf;
		size_t *m_pIn_buf_size, *m_pOut_buf_size;
		tdefl_flush m_flush;
		const mz_uint8 *m_pSrc;
		size_t m_src_buf_left, m_out_buf_ofs;
		mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
		mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
		mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
		mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
		mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
		mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
		mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
		mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
	} tdefl_compressor;

	tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

	tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush);

	tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush);

	tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d);
	mz_uint32 tdefl_get_adler32(tdefl_compressor *d);

#ifndef MINIZ_NO_ZLIB_APIS

	mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);
#endif

#ifdef __cplusplus
}
#endif

#endif
