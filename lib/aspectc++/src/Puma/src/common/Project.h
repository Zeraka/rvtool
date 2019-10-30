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

#ifndef PUMA_Project_H
#define PUMA_Project_H

/** \file
 * Project file handling. */

/** \page project File Handling
 */

#include "Puma/PathManager.h"
#include "Puma/UnitManager.h"
#include "Puma/SaveMode.h"
#include <fstream>
#include <iostream>

namespace Puma {

/** \class Project Project.h Puma/Project.h
 * A project is a set of source directories. It provides
 * methods for searching and writing files.
 * \ingroup common */
class Project : public PathManager, public SaveMode {
  UnitManager m_unitManager;
  const char *m_filenameSuffix;
  Mode m_saveMode;

private:
  /** Change current directory to the given, or create it if necessary.
   * \param path The path to the directory.
   * \return True if successfully changed the current directory. */
  bool makeDir(const char *path) const;
  /** Create the directory hierarchy as given by path.
   * \param path The path to the directory.
   * \return True if directory created successfully. */
  bool makeDirHierarchy(Filename path) const;
  /** Check if the given file needs to be updated.
   * \param file The path to the file.
   * \param unit The unit containing the contents of the file.
   * \return True if the file needs to be updated. */
  bool isFileOutdated(const char *file, Unit *unit) const;
  /** Update the modification state and save location of the given unit.
   * \param file The path to the file.
   * \param unit The unit containing the contents of the file. */
  void updateUnitState(const char *file, Unit *unit) const;

protected:
  /** Write a unit on the given file output stream.
   * \param unit The input unit.
   * \param file The output file. */
  virtual void write(Unit *unit, std::ofstream &file) const;

public:
  /** Constructor.
   * \param err An error stream for reporting errors. */
  Project(ErrorStream &err);
  /** Destructor. */
  virtual ~Project();

  /** Set the mode for saving an unit.
   * Mode OVERWRITE: Save the unit under its name and overwrite existing
   * files.
   * Mode RENAME_OLD: An existing file will be renamed. A suffix will be
   * added to the name of the file, like 'main.cc.old'
   * for the file 'main.cc' and the suffix '.old'.
   * Mode NEW_SUFFIX: The unit will be saved with a new suffix, like
   * 'main.cpp' for the unit 'main.cc' and the suffix
   * 'cpp'.
   * \param mode The save mode.
   * \param suffix Optional filename suffix. */
  void saveMode(Mode mode = OVERWRITE, const char *suffix = 0);

  /** Save a unit using the name of the unit to get the full
   * name of the file and path to save to. The save location
   * must not be protected by a protect pattern and the original
   * file must be located in one of the source directories.
   * \param unit The unit to save.
   * \param only_modified If true only modified units are saved. */
  virtual void save(Unit *unit, bool only_modified = false) const;

  /** Save one, all or all matching files.
   * \param file Optional file name or file name pattern.
   * \param only_modified If true only modified files are saved.
   * \param is_reg_ex If true the given filename is a regular expression. */
  virtual void save(const char *file = 0, bool only_modified = false, bool is_reg_ex = false) const;

  /** Print one, all or all matching units.
   * \param name Optional unit name or unit name pattern.
   * \param out Output stream on which to print the unit(s), defaults to std::cout.
   * \param is_reg_ex If true the given unit name is a regular expression. */
  void print(const char *name = 0, std::ostream &out = std::cout, bool is_reg_ex = false) const;

  /** Close one, all or all matching units.
   * \param name Optional unit name or unit name pattern.
   * \param destroy If true destroy the units when closed.
   * \param is_reg_ex If true the given unit name is a regular expression. */
  void close(const char *name = 0, bool destroy = false, bool is_reg_ex = false) const;

  /** Add a new file to the project.
   * \param file The path to the file to add.
   * \return An empty file unit for the file to add. */
  Unit *addFile(Filename file);
  /** Add a new file to the project.
   * \param srcFile The path to the source file to add.
   * \param destFile The path to the destination file.
   * \return An empty file unit for the file to add. */
  Unit *addFile(Filename srcFile, Filename destFile);

  /** Configure the project from the command line or a file.
   * \param config The configuration. */
  virtual void configure(const Config &config);

  /** Get the unit manager.
   * \return A reference to the unit manager. */
  UnitManager &unitManager();

  /** Check if the given file in the source directory
   * or the unit is newer than the corresponding
   * file in the destination directory.
   * \param file The path to the file.
   * \return True if destination file is older. */
  bool isNewer(const char *file) const;

  /** Check if a given file is found below any of the source paths of this project.
   * \param file The file.
   * \return True if the file was found. */
  virtual bool isBelow(const char *file) const;
  /** Check if a given file is found below any of the source paths of this project.
   * \param file The file.
   * \return True if the file was found. */
  virtual bool isBelow(Unit *file) const;
};

inline UnitManager &Project::unitManager() {
  return m_unitManager;
}

inline bool Project::isBelow(const char *file) const {
  return PathManager::isBelow(file);
}

} // namespace Puma

#endif /* PUMA_Project_H */
