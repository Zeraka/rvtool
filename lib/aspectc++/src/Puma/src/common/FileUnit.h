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

#ifndef PUMA_FileUnit_H
#define PUMA_FileUnit_H

/** \file
 * Unit for files. */

#include "Puma/Unit.h"

namespace Puma {

class Project;

/** \class FileUnit FileUnit.h Puma/FileUnit.h
 * Special Unit for files. Holds its last save location, the
 * absolute path of the file, and the source project the file
 * belongs to.
 * \ingroup common */
class FileUnit : public Unit {
  char *m_lastLocation;
  char *m_absolutePath;
  Project *m_project;
  bool m_isBelow;

public:
  /** Constructor. */
  FileUnit();
  /** Constructor.
   * \param unit The unit representing the file. */
  FileUnit(Unit &unit);
  /** Copy-constructor.
   * \param list The list of tokens representing the file. */
  FileUnit(const List &list);
  /** Destructor. Frees the tokens. */
  virtual ~FileUnit();
  /** Set the name of the unit, i.e. the filename.
   * \param filename The name of the unit. */
  virtual void name(const char *filename);
  /** Set the last save location of the file in the file system.
   * \param location The location in the file system. */
  void setLocation(const char *location);
  /** Get the last save location of the file in the file system.
   * \return The location of the file. */
  char *lastLocation() const;
  /** Get the absolute version of the filename.
   * \return The absolute filename. */
  char *absolutePath() const;
  /** Check if this unit refers to a file.
   * \return Always returns true. */
  bool isFile() const;
  /** Check if the file belongs to the given source project.
   * \param project The source project. */
  bool belongsTo(Project &project);
};

inline FileUnit::FileUnit()
    : Unit(), m_lastLocation(0), m_absolutePath(0), m_project(0), m_isBelow(false) {
}
inline FileUnit::FileUnit(Unit &unit)
    : Unit(unit), m_lastLocation(0), m_absolutePath(0), m_project(0), m_isBelow(false) {
}
inline FileUnit::FileUnit(const List &list)
    : Unit(list), m_lastLocation(0), m_absolutePath(0), m_project(0), m_isBelow(false) {
}
inline FileUnit::~FileUnit() {
  if (m_lastLocation)
    delete[] m_lastLocation;
  if (m_absolutePath)
    delete[] m_absolutePath;
}

inline char *FileUnit::lastLocation() const {
  return m_lastLocation;
}
inline char *FileUnit::absolutePath() const {
  return m_absolutePath;
}
inline bool FileUnit::isFile() const {
  return true;
}

} // namespace Puma

#endif /* PUMA_FileUnit_H */
