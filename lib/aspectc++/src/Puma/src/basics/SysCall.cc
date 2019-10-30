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

#include "Puma/SysCall.h"
#include "Puma/StrCol.h"
#include "Puma/ErrorSink.h"
#include <errno.h>
#include <fcntl.h>
#include <map>

#ifdef _MSC_VER
# include <direct.h>
# include <string>
#else
# include <unistd.h>
# include <dirent.h>
#endif // _MSC_VER

#ifdef WIN32
# include <io.h>
# include <windows.h>
#endif // WIN32

#include <string.h>
#include <stdlib.h>

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif // MAX_PATH

namespace Puma {

void SysCall::MakeUnixPath(char *path) {
  if (!path)
    return;
  char *s = path;
  while (*s) {
    if (*s == '\\')
      *s = '/';
    s++;
  }
}

void SysCall::MakeDosPath(char *path) {
#if defined (WIN32)
  if (!path)
    return;
  char *s = path;
  while (*s) {
    if (*s == '/')
      *s = '\\';
    s++;
  }
#endif
}

void SysCall::printerror(ErrorSink *err, const char *cmd, const char *path) {
  if (err) {
    *err << sev_fatal << path << ": command '" << cmd << "' failed, " << strerror(errno) << endMessage;
  }
}

void SysCall::printerror(ErrorSink *err, const char *cmd, int fd) {
  if (err) {
    *err << sev_fatal << fd << ": command '" << cmd << "' failed, " << strerror(errno) << endMessage;
  }
}

const char *SysCall::mktemp(const char *prefix, ErrorSink *err) {
#ifdef WIN32
  // get the system directory for temporary files
  char tempdir[MAX_PATH+1];
  if (!::GetTempPath(MAX_PATH, tempdir)) {
    printerror(err, "mktemp", "cannot get temp directory");
    tempdir[0] = '.';
    tempdir[1] = '\0';
  }

  // get the temp file name
  char filename[MAX_PATH+1];
  if (!::GetTempFileName(tempdir, prefix, 0, filename)) {
    printerror(err, "mktemp", "cannot create temp file name");
    filename[0] = '\0';
  }

  // needs to be freed by called
  return strdup(filename);

#else

  // create filename template
  char const *sys_tempdir = ::getenv("TMPDIR");
  if (!sys_tempdir) {
    sys_tempdir = "/tmp";
  }
  std::string tempdir(sys_tempdir);
  if (tempdir.length() > 0 && tempdir[tempdir.length() - 1] != '/') {
    tempdir += "/";
  }
  tempdir += prefix;
  tempdir += "XXXXXX";
  char *filename = strdup(tempdir.c_str());

  // create file with restrictive permissions 600
  mode_t old_mode = umask(077);
  // create the temp file
  int fd = mkstemp(filename);
  // reset permissions
  umask(old_mode);

  // close file descriptor
  if (fd == -1 || ::close(fd) == -1) {
    printerror(err, "mktemp", filename);
  }

  // needs to be freed by called
  return filename;
#endif
}

int SysCall::open(const char *file, int flags, ErrorSink *err) {
  int fd = -1;
#if defined(WIN32)
  char *n = StrCol::dup(file);
  if (n) {
    MakeDosPath(n);
    fd = ::open(n, flags | O_BINARY);
    delete[] n;
  }
#else
  fd = ::open(file, flags);
#endif
  if (fd == -1)
    printerror(err, "open", file);
  return fd;
}

int SysCall::open_excl(const char *file, int flags, ErrorSink *err) {
  int fd = open(file, flags, err);
#if defined(__GLIBC__)
  if (fd >= 0) {
    flock lock;
    memset(&lock, 0, sizeof lock);
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
      printerror(err, "fcntl lock", file);
  }
#endif 
  return fd;
}

int SysCall::create(const char *file, int mode, ErrorSink *err) {
  int fd = -1;
#if defined(WIN32)
  char *n = StrCol::dup(file);
  if (n) {
    MakeDosPath(n);
    fd = ::open(n, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, mode);
    delete[] n;
  }
#else
  fd = ::open(file, O_CREAT | O_WRONLY | O_TRUNC, mode);
#endif
  if (fd == -1)
    printerror(err, "create", file);
  return fd;
}

int SysCall::create_excl(const char *file, int mode, ErrorSink *err) {
  int fd = create(file, mode, err);
#if defined(__GLIBC__)
  if (!(fd < 0)) {
    flock lock;
    memset(&lock, 0, sizeof lock);
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
      printerror(err, "fcntl lock", file);
  }
#endif 
  return fd;
}

bool SysCall::close(int fd, ErrorSink *err) {
  int ret = ::close(fd);
  if (ret == -1) {
    printerror(err, "close", fd);
    return false;
  }
  return true;
}

bool SysCall::close_excl(int fd, ErrorSink *err) {
#if defined(__GLIBC__)
  flock lock;
  memset(&lock, 0, sizeof lock);
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  if (fcntl(fd, F_SETLKW, &lock) == -1) {
    printerror(err, "fcntl unlock", fd);
    return false;
  }
#endif
  return close(fd, err);
}

long SysCall::read(int fd, void *buffer, size_t n, ErrorSink *err) {
  long bytes = ::read(fd, buffer, n);
  if (bytes == -1)
    printerror(err, "read", fd);
  return bytes;
}

bool SysCall::chdir(const char *dir, ErrorSink *err) {
  int ret = -1;
#if defined(WIN32)
  char *n = StrCol::dup(dir);
  if (n) {
    MakeDosPath(n);
#ifdef _MSC_VER
    ret = ::_chdir(n);
#else
    ret = ::chdir(n);
#endif // _MSC_VER
    delete[] n;
  }
#else
  ret = ::chdir(dir);
#endif
  if (ret == -1) {
    printerror(err, "chdir", dir);
    return false;
  }
  return true;
}

char *SysCall::getcwd(char *buffer, size_t len, ErrorSink *err) {
#ifdef _MSC_VER
  char *cwd = ::_getcwd(buffer, len);
#else
  char *cwd = ::getcwd(buffer, len);
#endif // _MSC_VER
#if defined(WIN32)
  if (cwd != 0)
  MakeUnixPath(cwd);
#endif
  if (!cwd)
    printerror(err, "getcwd");
  return cwd;
}

// Get the current working directory.
char *SysCall::readlink(const char *link, char *buffer, size_t len, ErrorSink *err) {
#if defined(WIN32)
  buffer = 0;
#else
  int pos = ::readlink(link, buffer, len);
  if (pos == -1) {
    printerror(err, "readlink", link);
    return (char*) 0;
  }
  buffer[pos] = '\0';
#endif
  return buffer;
}

time_t SysCall::time(time_t *t, ErrorSink *err) {
  time_t curtime = ::time(t);
  if (curtime == ((time_t) -1))
    printerror(err, "time");
  return curtime;
}

bool SysCall::fstat(int fd, FileInfo &fi, ErrorSink *err) {
#ifdef _MSC_VER
  int ret = ::_fstat(fd, &fi.info);
#else
  int ret = ::fstat(fd, &fi.info);
#endif
  if (ret == -1) {
    printerror(err, "fstat", fd);
    return false;
  }
  return true;
}

bool SysCall::mkdir(const char *dir, ErrorSink *err) {
  int ret = -1;
#if defined(WIN32)
  char *n = StrCol::dup(dir);
  if (n) {
    MakeDosPath(n);
    ret = ::mkdir(n);
    delete[] n;
  }
#else
  mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
  ret = ::mkdir(dir, mode);
#endif
  if (ret == -1) {
    printerror(err, "mkdir", dir);
    return false;
  }
  return true;
}

DirHandle SysCall::opendir(const char *dir, ErrorSink *err) {
  DirHandle dp = NULL;
#if defined(WIN32)
  char *n = StrCol::dup(dir);
  if (n) {
    MakeDosPath(n);
#ifdef _MSC_VER
    std::string find_pattern(n);
    if (find_pattern[find_pattern.length() - 1] != '\\')
    find_pattern += "\\";
    find_pattern += "*";
    struct _finddata_t find_data;
    long hdl = _findfirst(find_pattern.c_str(), &find_data);
    if (hdl != -1L)
    dp = new DIR(hdl);
#else
    dp = ::opendir(n);
#endif // _MSC_VER
    delete[] n;
  }
#else
  dp = ::opendir(dir);
#endif
  if (!dp)
    printerror(err, "opendir", dir);
  return dp;
}

#ifdef _MSC_VER
const char *SysCall::readdir(DirHandle dp, DirEntryType *det, ErrorSink *err) {
  // try to get the next entry
  static struct _finddata_t find_data;
  const char *entry_name = 0;
  while (!entry_name) {
    if (_findnext(dp->handle, &find_data) == -1)
      break;
    if (strcmp(find_data.name, ".") != 0 && strcmp(find_data.name, "..") != 0)
      entry_name = find_data.name;
  }
  if (det && entry_name) {
    switch (find_data.attrib) {
      case _A_NORMAL: *det = DET_REGULAR; break;
      case _A_SUBDIR: *det = DET_DIRECTORY; break;
      default: *det = DET_OTHER;
    }
  }
  return entry_name;
}
#else
const char *SysCall::readdir(DirHandle dp, DirEntryType *det, ErrorSink *err) {
  struct dirent *entry;
  while (true) {
    entry = ::readdir(dp);
    // Ignore the `.' and `..' entries.
    if (!entry || !(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))
      break;
  }
  if (det && entry) {
#ifdef _DIRENT_HAVE_D_TYPE
    switch (entry->d_type) {
      case DT_REG:
        *det = DET_REGULAR;
        break;
      case DT_DIR:
        *det = DET_DIRECTORY;
        break;
      case DT_LNK:
        *det = DET_LINK;
        break;
      default:
        *det = DET_OTHER;
    }
#else
    *det = DET_NOT_IMPLEMENTED;
#endif
  }
  return entry ? entry->d_name : 0;
}
#endif // _MSC_VER

bool SysCall::closedir(DirHandle dp, ErrorSink *err) {
#ifdef _MSC_VER
  int ret = ::_findclose(dp->handle);
  delete dp;
#else
  int ret = ::closedir(dp);
#endif // _MSC_VER
  if (ret == -1) {
    printerror(err, "closedir");
    return false;
  }
  return true;
}

bool SysCall::stat(const char *file, FileInfo &fi, ErrorSink *err) {
  int ret = -1;
#if defined(WIN32)
  char *n = StrCol::dup(file);
  if (n) {
    MakeDosPath(n);
#ifdef _MSC_VER
    ret = ::_stat(n, &fi.info);
#else
    ret = ::stat(n, &fi.info);
#endif
    delete[] n;
  }
#else
  ret = ::stat(file, &fi.info);
#endif
  if (ret == -1) {
    if (errno == ENOMEM)
      printerror(err, "stat", file);
    return false;
  }
  return true;
}

bool SysCall::fclose(FileHandle fstr, ErrorSink *err) {
  int ret = ::fclose(fstr);
  if (ret == EOF) {
    printerror(err, "fclose");
    return false;
  }
  return true;
}

FileHandle SysCall::fopen(const char *file, const char *mode, ErrorSink *err) {
  FileHandle fstr = NULL;
#if defined(WIN32)
  char *n = StrCol::dup(file);
  if (n) {
    MakeDosPath (n);
    fstr = ::fopen(n, mode);
    delete[] n;
  }
#else
  fstr = ::fopen(file, mode);
#endif
  if (fstr == NULL)
    printerror(err, "fopen", file);
  return fstr;
}

#ifdef WIN32
bool SysCall::normalize(Filename filename, Filename &norm, ErrorSink *err) {
  const char *name = filename.name();
  std::string result;

  assert(strlen(name) >= 2);
  // first check for the end of the recursion: 
  if (strlen(name) == 2 && name[1] == ':' && isalpha(name[0])) {
    result = toupper(name[0]);
    result += ":";
  } else {
    // normalize the directory part recursively ...
    Filename norm_path;
    if (!canonical(filename.path(), norm_path, err)) {
      return false;
    }
    // .. and find the normlized filename with _findfirst
    struct _finddata_t find_data;
    long hdl = _findfirst(name, &find_data);
    if (hdl == -1L) {
      return false;
    }
    result = norm_path.name();
    result += "/";
    result += find_data.name;
    _findclose(hdl);
  }
  norm.name(result.c_str());
  return true;
}
#endif // WIN32

typedef std::map<DString, Filename> FilenameMap;
typedef FilenameMap::value_type FilenameMapPair;
FilenameMap canonical_cache;

bool SysCall::canonical(Filename filename, Filename &result, ErrorSink *err) {
  // first check if the searched canonical name is already cached  
  FilenameMap::iterator fn = canonical_cache.find(filename.name());
  if (fn != canonical_cache.end()) {
    result = fn->second;
    return true;
  }

  // build a canonical path name
  const char *file_abs = filename.name();
  char file_buf[MAX_PATH];

#if defined (WIN32)
  if ((strlen(file_abs) == 2 && file_abs[1] == ':' && isalpha(file_abs[0])) ||
      (file_abs = _fullpath(file_buf, file_abs, MAX_PATH))) {
    Filename abs(file_abs);
    if (!normalize(abs, result, err)) {
      file_abs = 0;
    }
  }
#else
  file_abs = realpath(file_abs, file_buf);
  if (file_abs) {
    result.name(file_abs);
  }
#endif

  if (!file_abs) {
    printerror(err, "canonical", filename.name());
    return false;
  }

  canonical_cache.insert(FilenameMapPair(filename.name(), result));
  return true;
}

char *SysCall::absolute(const char *filename, ErrorSink *err) {
  char link_buf[4096];

  char *link = SysCall::readlink(filename, link_buf, sizeof(link_buf), err);

  // no link return simple absolute path
  if (!link) {
    Filename result;
    if (!SysCall::canonical(filename, result, err))
      return 0;
    return StrCol::dup(result.name());
  }

  // an absolute link, resolve this file/link again
  if (link[0] == '/')
    return absolute(link, err);

  // relative name, concatenate directory with link and resolve
  char name_buf[4096];
  strcpy(name_buf, filename);
  char *delim = strrchr(name_buf, (int) '/');
  if (!delim) {
    strcpy(name_buf, link);
  } else {
    *(delim + 1) = '\0';
    strcat(name_buf, link);
  }
  return absolute(name_buf, err);
}

} // namespace Puma
