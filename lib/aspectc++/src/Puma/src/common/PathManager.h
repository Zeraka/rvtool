// This file is part of PUMA.
// Copyright (C) 1999-2016  The PUMA developer team.
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

#ifndef PUMA_PathManager_H
#define PUMA_PathManager_H

/** \file
 * Source and destination path and file management. */

#include "Puma/Array.h"
#include "Puma/PathInfo.h"
#include "Puma/Filename.h"
#include "Puma/ErrorStream.h"
#include "Puma/PathIterator.h"
#include "Puma/ProjectFile.h"
#include <list>
#include <string>

namespace Puma {

class RegComp;
class Config;
class Unit;

/** \class PathManager PathManager.h Puma/PathManager.h
 * Source and destination path and file management. Source paths are
 * mapped to their destination paths. The source tree can be iterated
 * and new files can be added.
 * \ingroup common */
class PathManager {
  ErrorStream *m_err;
  Array<PathInfo> m_paths;
  Array<RegComp*> m_writeProtectedPathPatterns;
  mutable ProjectFile::Map m_files;

private:
  /** Find all files of a directory tree matching the given pattern
   * and add them to the project file map.
   * \param path The path to the directory to traverse.
   * \param pathLength The length of the path name.
   * \param filenamePattern Regular expression for the filenames to match.
   * \param hits List of files found. */
  void traverse(char *path, size_t pathLength, RegComp *filenamePattern, std::list<std::string> &hits) const;

  /** Calculate the canonical filename representation for a file
   * and write it into abs_name.
   * \param name The filename.
   * \param abs_name Container for the calculated canonical filename. */
  bool canonFilename(Filename name, Filename &abs_name) const;

  /** Add a new file to the project file list.
   * \param file The file to add.
   * \return An iterator pointing to the added file. */
  ProjectFile::MapConstIter addFile(const ProjectFile &file);

protected:
  /** Get the destination path for a given source path.
   * \param sourcePath The source path.
   * \param destinationPath Optional output stream for the destination path.
   * \return The destination path. */
  const char *getDestination(Filename sourcePath, std::ostream *destinationPath = 0) const;

  /** Called for every matched file when traversing a source path.
   * \param iterator The path iterator pointing to the current file. */
  virtual void action(PathIterator &iterator) {
  }

  /** Add the separator '/' to the end of the given path.
   * \param path The path.
   * \return A copy of the path followed by the separator. */
  char *addSeparator(const char *path) const;
  /** Add the separator '/' to the end of the given path
   * and store the resulting path in the given buffer.
   * \param path The path.
   * \param buffer The buffer.
   * \return The buffer containing the path followed by the separator. */
  char *addSeparator(const char *path, char *buffer) const;

public:
  /** Constructor.
   * \param err Error stream used to report errors. */
  PathManager(ErrorStream &err)
      : m_err(&err) {
  }
  /** Copy-constructor.
   * \param other The other path manager to copy. */
  PathManager(PathManager &other) {
    m_err = other.m_err;
    join(other);
  }
  /** Destructor. */
  virtual ~PathManager();

  /** Search files in the managed path matching the given filename pattern or every
   * file if no pattern is given. Calls action() for every match.
   * \param filenamePattern Optional regular expression for the filenames to match. */
  void glob(char *filenamePattern = 0);

  /** Iterate the contents of the managed paths. Calls action() for every file found.
   * \param iterator The path iterator to use. */
  bool iterate(PathIterator &iterator) const;

  /** Add a source and destination paths pair.
   * \param srcPath The source path.
   * \param destPath The corresponding destination path. */
  virtual void addPath(const char *srcPath, const char *destPath = 0);

  /** Add a new file to the project file list
   * \param filename The filename.
   * \return An iterator pointing to the file added. */
  ProjectFile::MapConstIter addFile(Filename filename);

  /** Add a new file to the project file list
   * \param filename The filename.
   * \param destPath The corresponding destination path.
   * \return An iterator pointing to the file added. */
  ProjectFile::MapConstIter addFile(Filename filename, Filename destPath);

  /** Set the destination path of the given source path.
   * \param srcPath The source path.
   * \param destPath The destination path. */
  void setDest(const char *srcPath, const char *destPath);

  /** Add a pattern for a write-protected path.
   * \param pathPattern Regular expression for a path to write-protect. */
  void protect(const char *pathPattern);

  /** Get the number of managed paths.
   * \return The number of managed paths. */
  long numPaths() const {
    return m_paths.length();
  }
  /** Get the number of managed write-protected paths.
   * \return The number of write-protected paths. */
  long numProts() const {
    return m_writeProtectedPathPatterns.length();
  }
  /** Get the n-th managed source path.
   * \param n The position of the path in the path list.
   * \return The source path. */
  const char *src(long n) const {
    return m_paths.lookup(n).src();
  }
  /** Get canonical version of the n-th managed source path.
   * \param n The position of the path in the path list.
   * \return The canonical source path. */
  const char *src_canon(long n) const {
    return m_paths.lookup(n).srcCanonical();
  }
  /** Get the n-th managed destination path.
   * \param n The position of the path in the path list.
   * \return The destination path. */
  const char *dest(long n) const {
    return m_paths.lookup(n).dest();
  }
  /** Get the n-th managed write-protected path pattern.
   * \param n The position of the path pattern in the path pattern list.
   * \return The write-protected path pattern. */
  RegComp *prot(long n) const {
    return m_writeProtectedPathPatterns.lookup(n);
  }

  /** Check if the given path is write-protected.
   * \param path The path.
   * \return True if the path is write-protected. */
  bool isProtected(const char *path) const;

  /** Check if a given file is directly managed by this path manager or
   * is found below any of the managed paths. If the file is found then
   * its position is stored in the given iterator.
   * \param filename The filename.
   * \param iterator The iterator to store the position of the file.
   * \return True if the file was found. */
  bool isBelow(const char *filename, ProjectFile::MapConstIter &iterator) const;

  /** Check if a given file is directly managed by this path manager or
   * is found below any of the managed paths.
   * \param filename The filename.
   * \return True if the file was found. */
  virtual bool isBelow(const char *filename) const;

  /** Configure the path manager.
   * \param config The configuration. */
  virtual void configure(const Config &config);

  /** Join the paths of the given manager with the paths
   * of this path manager.
   * \param other The other path manager. */
  virtual void join(PathManager &other);

  /** Get the error stream used by this path manager.
   * \return A reference to the error stream. */
  ErrorStream &err() const {
    return *m_err;
  }

  /** Get the destination path of a given source path and write
   * it on the given output stream.
   * \param sourcePath The source path.
   * \param out The output stream.
   * \return True if the destination path was found. */
  bool getDestinationPath(const char *sourcePath, std::ostream &out) const;
};

inline bool PathManager::isBelow(const char *file) const {
  ProjectFile::MapConstIter iter;
  return isBelow(file, iter);
}

} // namespace Puma

#endif /* PUMA_PathManager_H */
