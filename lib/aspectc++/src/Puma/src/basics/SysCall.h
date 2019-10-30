// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#ifndef PUMA_SysCall_H
#define PUMA_SysCall_H

/** \file
 * Encapsulation of system dependent file system operations. */

// System includes
#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <string.h>
#else
#include <dirent.h>
#endif // _MSVC

// PUMA includes
#include "Puma/Filename.h"

namespace Puma {

class ErrorSink;
class SysCall;

/** \class FileInfo SysCall.h Puma/SysCall.h
 * File information. */
class FileInfo {
  friend class SysCall;
#ifndef _MSC_VER
  struct stat info;
#else
  struct _stat info;
#endif

public:
  /** Check if the file is a directory.
   * \return True if it is a directory. */
  bool is_dir() const {
#ifdef _MSC_VER
    return _S_IFDIR & info.st_mode;
#else
    return S_ISDIR(info.st_mode);
#endif // _MSC_VER
  }
  /** Get the last modification time of the file.
   * \return The last modification time. */
  time_t modi_time() const {
    return info.st_mtime;
  }
  /** Get the size of the file.
   * \return The file size. */
  int size() const {
    return info.st_size;
  }
};

#ifdef _MSC_VER
/** \cond internal */
struct DIR {
  long handle;
  DIR(long h) : handle(h) {}
};
/** \endcond */
#endif // _MSC_VER

/** Directory handle type. */
typedef DIR *DirHandle;
/** File handle type. */
typedef FILE *FileHandle;

/** \enum DirEntryType SysCall.h Puma/SysCall.h
 * Directory entry types. */
enum DirEntryType {
  /** Regular file. */
  DET_REGULAR,
  /** Directory. */
  DET_DIRECTORY,
  /** Symbolic link. */
  DET_LINK,
  /** Other kind of directory entry. */
  DET_OTHER,
  /** Not implemented directory entry type. */
  DET_NOT_IMPLEMENTED
};

/** \class SysCall SysCall.h Puma/SysCall.h
 * Encapsulates system dependent file system operations.
 * \ingroup basics */
class SysCall {
  static void printerror(ErrorSink *err, const char *cmd, const char *path = (const char *) 0);
  static void printerror(ErrorSink *err, const char *cmd, int fd);

public:
  /** Create a temporary file name, needs to be freed by the caller.
   * \param prefix The file name prefix.
   * \param err Optional error reporting stream. */
  static const char* mktemp(const char *prefix, ErrorSink *err = (ErrorSink *) 0);

  /** Open a file in exclusive file locking mode.
   * \param path The path to the file.
   * \param flags The file open flags.
   * \param err Optional error reporting stream.
   * \return The file descriptor of the opened file. */
  static int open_excl(const char *path, int flags, ErrorSink *err = (ErrorSink *) 0);
  /** Create and open a file in exclusive file locking mode.
   * \param path The path to the file.
   * \param mode The file creation mode.
   * \param err Optional error reporting stream.
   * \return The file descriptor of the opened file. */
  static int create_excl(const char *path, int mode, ErrorSink *err = (ErrorSink *) 0);
  /** Close a file opened in exclusive file locking mode.
   * \param fd The file descriptor of the file to close.
   * \param err Optional error reporting stream.
   * \return True if closing the file succeeded. */
  static bool close_excl(int fd, ErrorSink *err = (ErrorSink *) 0);

  /** Open a file in non-exclusive mode.
   * \param path The path to the file.
   * \param flags The file open flags.
   * \param err Optional error reporting stream.
   * \return The file descriptor of the opened file. */
  static int open(const char *path, int flags, ErrorSink *err = (ErrorSink *) 0);
  /** Create and open a file in non-exclusive mode.
   * \param path The path to the file.
   * \param mode The file creation mode.
   * \param err Optional error reporting stream.
   * \return The file descriptor of the opened file. */
  static int create(const char *path, int mode, ErrorSink *err = (ErrorSink *) 0);
  /** Close a file opened in non-exclusive mode.
   * \param fd The file descriptor of the file to close.
   * \param err Optional error reporting stream.
   * \return True if closing the file succeeded. */
  static bool close(int fd, ErrorSink *err = (ErrorSink *) 0);

  /** Read n bytes from a file into the given buffer.
   * \param fd The file descriptor of the file to read.
   * \param buffer The buffer to fill with the bytes read.
   * \param n The number of bytes to read.
   * \param err Optional error reporting stream.
   * \return The number of bytes read. */
  static long read(int fd, void *buffer, size_t n, ErrorSink *err = (ErrorSink *) 0);

  /** Get information about a file.
   * \param path The path to the file.
   * \param info The file information object to fill.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool stat(const char *path, FileInfo &info, ErrorSink *err = (ErrorSink *) 0);
  /** Get information about a file.
   * \param fd The file descriptor of the file.
   * \param info The file information object to fill.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool fstat(int fd, FileInfo &info, ErrorSink *err = (ErrorSink *) 0);
  /** Get the path to the file the given link points to.
   * \param path The path to the link.
   * \param buffer The buffer to fill with the result.
   * \param len The size of the buffer.
   * \param err Optional error reporting stream.
   * \return The buffer containing the path to the linked file. */
  static char* readlink(const char *path, char *buffer, size_t len, ErrorSink *err = (ErrorSink *) 0);

  /** Open a file stream.
   * \param path The path to the file to open.
   * \param mode The file open mode.
   * \param err Optional error reporting stream.
   * \return A handle to the file stream. */
  static FileHandle fopen(const char *path, const char *mode, ErrorSink *err = (ErrorSink *) 0);
  /** Close a file stream.
   * \param fh The handle to the file stream.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool fclose(FileHandle fh, ErrorSink *err = (ErrorSink *) 0);

  /** Open a directory stream.
   * \param path The path to the directory to open.
   * \param err Optional error reporting stream.
   * \return A handle to the directory stream. */
  static DirHandle opendir(const char *path, ErrorSink *err = (ErrorSink *) 0);
  /** Close a directory stream.
   * \param dh The handle to the directory stream.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool closedir(DirHandle dh, ErrorSink *err = (ErrorSink *) 0);

  // read from directory stream
  // Pass a pointer to a DirEntryType object if you are interested whether the entry
  // is a regular file, link, directory, or other.
  static const char *readdir(DirHandle dh, DirEntryType *type = 0, ErrorSink *err = (ErrorSink *) 0);

  /** Change the current working directory.
   * \param path The path to the new working directory.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool chdir(const char *path, ErrorSink *err = (ErrorSink *) 0);

  /** Create a directory.
   * \param path The path to the new directory.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool mkdir(const char *path, ErrorSink *err = (ErrorSink *) 0);

  /** Get the current working directory.
   * \param buffer A buffer to be filled with the path to the current working directory.
   * \param len The size of the buffer.
   * \param err Optional error reporting stream.
   * \return The buffer containing the path to the current working directory. */
  static char *getcwd(char *buffer, size_t len, ErrorSink *err = (ErrorSink *) 0);

  /** Get the current time.
   * \param buffer The buffer to be filled with the current time.
   * \param err Optional error reporting stream.
   * \return The current time, or -1 in case of error. */
  static time_t time(time_t *buffer, ErrorSink *err = (ErrorSink *) 0);

  /** Transform the given path into a Unix path by replacing all backslashes with slashes.
   * \param path The path to transform. */
  static void MakeUnixPath(char *path);
  /** Transform the given path into a MS-DOS path by replacing all slashes with backslashes.
   * \param path The path to transform. */
  static void MakeDosPath(char *path);

#ifdef WIN32
  /** Normalize an absolute Win32 filename of an existing file.
   * \param filename The filename to normalize.
   * \param result The resulting normalized filename.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool normalize(Filename filename, Filename &result, ErrorSink *err = 0);
#endif // WIN32

  /** Get the canonical path name for an existing file.
   * \param filename The filename of the file.
   * \param result The resulting canonical path name.
   * \param err Optional error reporting stream.
   * \return True if succeeded. */
  static bool canonical(Filename filename, Filename &result, ErrorSink *err = 0);

  /** Get the absolute path name for an existing file.
   * \param filename The filename of the file.
   * \param err Optional error reporting stream.
   * \return A newly allocated string containing the absolute path name. */
  static char *absolute(const char *filename, ErrorSink *err = (ErrorSink *) 0);
};

} // namespace Puma

#endif /* PUMA_SysCall_H */
