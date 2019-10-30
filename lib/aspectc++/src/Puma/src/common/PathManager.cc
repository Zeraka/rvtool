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

#include "Puma/Config.h"
#include "Puma/PathIterator.h"
#include "Puma/ErrorStream.h"
#include "Puma/PathManager.h"
#include "Puma/RegComp.h"
#include "Puma/SysCall.h"
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define PATH_MAX _MAX_PATH
#endif

// ensure that maximal path length is at least 65536 characters
#if !defined(PATH_MAX) || PATH_MAX < 65536
#undef PATH_MAX
#define PATH_MAX 65536
#endif

namespace Puma {

PathManager::~PathManager() {
  for (int i = numProts() - 1; i >= 0; i--)
    delete m_writeProtectedPathPatterns[i];
}

// Join the paths of the given manager with the paths 
// of this path manager.
void PathManager::join(PathManager &pm) {
  for (int i = 0; i < pm.numPaths(); i++)
    addPath(pm.src(i), pm.dest(i));
  for (int i = 0; i < pm.numProts(); i++)
    m_writeProtectedPathPatterns.append(new RegComp(*pm.prot(i)));
}

// Add the separator '/' to the copy of the given string.
char *PathManager::addSeparator(const char *s) const {
  if (!s)
    return (char*) 0;
  int pos = strlen(s);

  char *str = new char[pos + 2];
  strcpy(str, s);
  if (str[pos - 1] != '/') // Add a trailing '/'.
    strcat(str, "/");
  return str;
}

// Add the separator '/' to the copy of the given string.
char *PathManager::addSeparator(const char *s, char *buf) const {
  if (!s)
    return (char*) 0;
  int len = strlen(s);
  if (len >= PATH_MAX)
    len = PATH_MAX - 2;
  memcpy(buf, s, len);
  if (s[len - 1] != '/') {
    buf[len] = '/';      // Add a trailing '/'.
    len++;
  }
  buf[len] = '\0';
  return buf;
}

// Add a new source directory.
void PathManager::addPath(const char *source, const char *destination) {
  if (source) {
    char newsrc[PATH_MAX];
    addSeparator(source, newsrc);

    // Don't add a source directory twice.
    // Don't add a sub-directory of an existing source directory.
    for (int pos = numPaths() - 1; pos >= 0 && src(pos); pos--)
      if (strncmp(src(pos), newsrc, strlen(src(pos))) == 0)
        return;

    // Add canonical filename representation
    Filename dir_abs;
    if (!canonFilename(newsrc, dir_abs))
      return;
    m_paths[numPaths()].srcCanonical(dir_abs.name());

    // Add the source path.
    m_paths[numPaths() - 1].src(newsrc);

    if (destination) {
      // Add the destination path.
      char newdest[PATH_MAX];
      addSeparator(destination, newdest);
      m_paths[numPaths() - 1].dest(newdest);
    }
  } else if (destination) {
    // Add the destination path.
    char newdest[PATH_MAX];
    addSeparator(destination, newdest);
    m_paths[numPaths()].dest(newdest);
    m_paths[numPaths() - 1].src(0);
  }
}

// find the canonical filename representation for a file
bool PathManager::canonFilename(Filename name, Filename &abs_name) const {
  std::string file_abs = "";
  const char *path = name.path();
  if (strcmp(path, "") == 0)
    path = ".";

  Filename canon_file, canon_dir;
  if (SysCall::canonical(name, canon_file))
    file_abs = canon_file.name();
  else if (SysCall::canonical(name.path(), canon_dir)) {
    file_abs = canon_dir.name();
    const char *nodir = strrchr(name.name(), '/');
    if (!nodir)
      nodir = strrchr(name.name(), '\\');
    if (nodir)
      file_abs += nodir;
  } else
    return false; // neither file nor directory exist

  abs_name.name(file_abs.c_str());
  return true;
}

// checks if a give file (by name) is a registered file of this path manager
// In case of success (found!) the an iterator is returned, which can be
// used to access more information about the file.
bool PathManager::isBelow(const char *file, ProjectFile::MapConstIter &iter) const {
  // determine the canonical name (which has to exist)
  Filename file_abs;
  if (!canonFilename(file, file_abs))
    return false;

  // search for the name and return the result
  iter = m_files.find(std::string(file_abs.name()));

  // all files stored in the map belong to the project
  if (iter != m_files.end())
    return true;

  // otherwise check if any project path is a prefix of the canonical filename
  bool found = false;
  for (int pos = numPaths() - 1; pos >= 0 && src(pos); pos--) {
    int prefix_len = strlen(src_canon(pos));
    if ((strncmp(src_canon(pos), file_abs.name(), prefix_len) == 0) && (file_abs.name()[prefix_len] == '/' || file_abs.name()[prefix_len] == '\\')) {
      found = true;
      break;
    }
  }

  // if the file does not belong to the project, return false
  if (!found)
    return false;

  // otherwise store it in the list and return true
  std::pair<ProjectFile::MapConstIter, bool> insert_result = m_files.insert(ProjectFile::MapPair(std::string(file_abs.name()), ProjectFile(file)));
  iter = insert_result.first;
  return true;
}

// Add a new file to the project file list
ProjectFile::MapConstIter PathManager::addFile(const ProjectFile &file) {
  Filename file_abs;
  bool have_canon = canonFilename(file.name(), file_abs);
  if (!have_canon) {
    assert(false);
    return m_files.end();
  }

  // insert the file with its canonical name as the key
  std::pair<ProjectFile::MapConstIter, bool> insert_result = m_files.insert(ProjectFile::MapPair(std::string(file_abs.name()), file));

  // return the iterator
  return insert_result.first;
}

// Add a new file to the project file list
ProjectFile::MapConstIter PathManager::addFile(Filename file) {
  return addFile(ProjectFile(file));
}

// Add a new file to the project file list with destination path
ProjectFile::MapConstIter PathManager::addFile(Filename name, Filename dest) {
  return addFile(ProjectFile(name, dest));
}

// Set the destination directory of the given source directory.
void PathManager::setDest(const char *source, const char *destination) {
  if (!source)
    return;

  // Search the corresponding path info object.
  int pos;
  for (pos = numPaths() - 1; pos >= 0 && src(pos); pos--)
    if (strcmp(src(pos), source) == 0)
      break;
  if (pos < 0)
    return; // Source path doesn't exist.
  if (destination) {
    // Set the destination path.
    char newdest[PATH_MAX];
    addSeparator(destination, newdest);
    m_paths[pos].dest(newdest);
  } else
    m_paths[pos].dest(destination);
}

// Configure the project from the command line or a file.
void PathManager::configure(const Config &c) {
  const ConfOption *d = 0, *p = 0;

  unsigned num = c.Options();
  for (unsigned i = 0; i < num; i++) {
    const ConfOption *o = c.Option(i);
    bool new_p = false, new_d = false;

    if (!strcmp(o->Name(), "-w")) {
      if (o->Arguments() != 1)
        continue;
      protect(o->Argument(0));
    } else if (!strcmp(o->Name(), "-p")) {
      if (o->Arguments() != 1)
        continue;
      new_p = true;
    } else if (!strcmp(o->Name(), "-d")) {
      if (o->Arguments() != 1)
        continue;
      new_d = true;
    }

    if (new_p) {
      if (p) {
        addPath(p->Argument(0), d ? d->Argument(0) : 0);
        if (d)
          d = 0;
      }
      p = o;
    }

    if (new_d) {
      if (d) {
        addPath(p ? p->Argument(0) : 0, d->Argument(0));
        if (p)
          p = 0;
      }
      d = o;
    }
  }

  if (p || d)
    addPath(p ? p->Argument(0) : 0, d ? d->Argument(0) : 0);
}

// Initial globbing implementation.
void PathManager::glob(char *pattern) {
  // Explore the source directories of the manager and for 
  // any file matching the given pattern call the method 
  // action(filename). The default pattern is ".*".
  PathIterator iter(pattern);
  while (iterate(iter))
    action(iter);
}

// Iterate the contents of the paths.
bool PathManager::iterate(PathIterator &iter) const {
  // a new iterator should start at the beginning
  if (iter.m_currentFile.empty()) {
    char path[PATH_MAX + 1];
    for (int pos = numPaths() - 1; pos >= 0 && src(pos); pos--) {
      strcpy(path, src(pos));
      traverse(path, strlen(path), iter.m_searchPattern, iter.m_matchingFiles);
    }
    iter.m_fileIterator = iter.m_matchingFiles.begin();
  }

  bool result = false;
  if (iter.m_fileIterator == iter.m_matchingFiles.end()) {
    iter.m_currentFile = std::string();
  } else {
    iter.m_currentFile = *iter.m_fileIterator;
    ++(iter.m_fileIterator);
    result = true;
  }
  return result;
}

// find all files of a directory tree and add them to _files
void PathManager::traverse(char *path, size_t pathLength, RegComp *filenamePattern, std::list<std::string> &hits) const {
  // Prepare the current path prefix
  if (path[pathLength - 1] != '/') {
    if (pathLength >= PATH_MAX - 1) {
      err() << sev_error << "path name too long: " << path << endMessage;
      return;
    }
    path[pathLength] = '/';
    pathLength++;
    path[pathLength] = '\0';
  }

  // open the current directory
  DirHandle dp = SysCall::opendir(path, &err());
  if (!dp) { // Skip the directory, may be an access problem.
    err() << sev_error << "Couldn't open directory `" << path << "'." << endMessage;
    return;
  }

  // Read the current directory entries.
  const char *entry;
  DirEntryType det;
  while ((entry = SysCall::readdir(dp, &det, &err()))) {
    // Create the full name of the current entry, e.g. /tmp/file.c.
    size_t l = strlen(entry);
    if (pathLength + l >= PATH_MAX) {
      err() << sev_error << "path name too long: " << path << entry << endMessage;
      continue;
    }
    memcpy(path + pathLength, entry, l);
    path[pathLength + l] = '\0';

    // Read the attributes of the current entry.
    if (det == DET_NOT_IMPLEMENTED || det == DET_OTHER) {
      FileInfo fi;
      if (!SysCall::stat(path, fi, &err()))
        continue;
      if (fi.is_dir())
        det = DET_DIRECTORY;
      else
        det = DET_REGULAR;
    }

    // Test whether entry is a directory.
    if (det == DET_DIRECTORY) {
      traverse(path, pathLength + l, filenamePattern, hits); // Dive into the sub-directory.
    } else {
      const char *filename = path;
      // File entry found.
      if (pathLength + l > 1 && *path == '.' && *(path + 1) == '/')
        filename += 2;
      if (filenamePattern->match(filename)) {
        hits.push_back(filename);
      }
    }
  }

  // Close the current directory and go one up.
  SysCall::closedir(dp, &err());
}

// Add a regular pattern specifying a path that has to be
// protected from writing.
void PathManager::protect(const char *path) {
  if (path)
    m_writeProtectedPathPatterns.append(new RegComp(path));
}

// Return true if the given file or path is protected
// from writing or if it isn't located within one of the 
// source directories, because then it's protected, too.
bool PathManager::isProtected(const char *file) const {
  if (!file)
    return false;

  // Protected by protect patterns?
  for (int i = numProts() - 1; i >= 0; i--) {
    if (prot(i)->match(file))
      return true;
  }

  // From outside the source directories?
  return !isBelow(file);
}

const char *PathManager::getDestination(Filename file, std::ostream *name) const {
  // determine the canonical name (which has to exist)
  Filename file_abs;
  if (!canonFilename(file, file_abs))
    return 0;

  for (int i = numPaths() - 1; i >= 0 && src(i); i--) {
    // get the canonical filename representation of the current path
    const char* dir_abs = src_canon(i);
    // is it a prefix of the filename?
    int len = strlen(dir_abs);
    if (strncmp(dir_abs, file_abs.name(), len) == 0 && *(file_abs.name() + len) == '/') {
      if (name)
        *name << file_abs.name() + len + 1;
      return dest(i);
    }
  }
  return 0;
}

bool PathManager::getDestinationPath(const char *filename, std::ostream &out) const {
  std::ostringstream rest;
  const char *dir = getDestination(filename, &rest);
  if (dir) {
    out << dir;
    if (dir[strlen(dir) - 1] != '/')
      out << "/";
    out << rest.str().c_str();
    return true;
  }
  return false;
}

} // namespace Puma
