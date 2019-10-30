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

#ifndef PUMA_PathIterator_H
#define PUMA_PathIterator_H

/** \file
 * Iterator implementation for paths. */

#include <list>
#include <string>

namespace Puma {

class PathManager;
class RegComp;

// A path iterator is used to iterate the contents of the paths
// a project is managing. But only files matching a given pattern
// are iterated.

/** \class PathIterator PathIterator.h Puma/PathIterator.h
 * A path iterator is used to iterate the contents of the paths
 * a project is managing. Only files matching a given pattern
 * are found.
 * To be used with PathManager::iterate().
 * \ingroup common */
class PathIterator {
  friend class PathManager;

  RegComp *m_searchPattern;
  std::list<std::string> m_matchingFiles;
  std::list<std::string>::const_iterator m_fileIterator;
  std::string m_currentFile;

public:
  /** Construct a new iterator.
   * \param pattern Regular expression for the files to match. */
  PathIterator(const char *pattern);
  /** Destroy the iterator. */
  ~PathIterator();

  /** Set the regular expression for the files to match.
   * \param pattern The regular expression. */
  void pattern(const char *pattern);

  /** Get the current file matched.
   * \return The path to the file. */
  const char *file() const;
};

} // namespace Puma

#endif /* PUMA_PathIterator_H */
