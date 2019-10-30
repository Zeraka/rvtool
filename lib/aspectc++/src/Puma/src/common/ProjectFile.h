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

#ifndef PUMA_ProjectFile_H
#define PUMA_ProjectFile_H

/** \file
 * Project file information. */

#include <map>
#include <string>

#include "Puma/Filename.h"

namespace Puma {

/** \class ProjectFile ProjectFile.h Puma/ProjectFile.h
 * Project file abstraction used to store all filenames relevant
 * for a specific file.
 * \ingroup common */
class ProjectFile {
  Filename m_name;
  Filename m_destPath;

public:
  /** Project file's name to ProjectFile map type. */
  typedef std::map<std::string, ProjectFile> Map;
  /** Project file's name to ProjectFile map pair type. */
  typedef Map::value_type MapPair;
  /** Project file's name to ProjectFile map constant iterator type. */
  typedef Map::const_iterator MapConstIter;

public:
  /** Construct a project file from the given filename.
   * \param filename The filename. */
  ProjectFile(const Filename &filename)
      : m_name(filename) {
  }
  /** Construct a project file from the given filename
   * and destination path.
   * \param filename The filename.
   * \param destPath The destination path. */
  ProjectFile(const Filename &filename, const Filename &destPath)
      : m_name(filename), m_destPath(destPath) {
  }
  /** Set the project file destination path.
   * \param destPath The destination path. */
  void dest(const Filename &destPath) {
    m_destPath = destPath;
  }
  /** Get the project file's name.
   * \return The filename information. */
  Filename name() const {
    return m_name;
  }
  /** Get the project file's destination path.
   * \return The destination path information. */
  Filename dest() const {
    return m_destPath;
  }
};

} // namespace Puma

#endif // PUMA_ProjectFile_H
