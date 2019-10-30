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

#include "Puma/ErrorStream.h"
#include "Puma/FileSource.h"
#include "Puma/FileUnit.h"
#include "Puma/MacroUnit.h"
#include "Puma/RegComp.h"
#include "Puma/Project.h"
#include "Puma/SysCall.h"
#include "Puma/StrCol.h"
#include "Puma/Config.h"
#include "Puma/Unit.h"
#include <string.h>
#include <limits.h>
#ifdef _MSC_VER
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif

namespace Puma {

extern const char *GPL_HEADER;

Project::Project(ErrorStream &e)
    : PathManager(e), m_unitManager(e), m_filenameSuffix((const char*) 0), m_saveMode(OVERWRITE) {
}

Project::~Project() {
  if (m_filenameSuffix)
    delete[] m_filenameSuffix;
}

// Configure the project from the command line or a file.
void Project::configure(const Config &c) {
  const char *suffix = m_filenameSuffix;
  Mode mode = m_saveMode;

  unsigned num = c.Options();
  for (unsigned i = 0; i < num; i++) {
    const ConfOption *o = c.Option(i);
    if (!strcmp(o->Name(), "--save-overwrite"))
      mode = OVERWRITE;
    else if (!strcmp(o->Name(), "--save-rename-old"))
      mode = RENAME_OLD;
    else if (!strcmp(o->Name(), "--save-new-suffix"))
      mode = NEW_SUFFIX;
    else if (!strcmp(o->Name(), "-s") || !strcmp(o->Name(), "--suffix")) {
      if (o->Arguments())
        suffix = o->Argument(0);
    }
  }
  saveMode(mode, suffix);

  // Configure the basic path manager, too.
  PathManager::configure(c);
}

// Set the mode for saving a unit.
void Project::saveMode(Mode mode, const char *suffix) {
  if (suffix) {
    if (m_filenameSuffix)
      delete[] m_filenameSuffix;
    m_filenameSuffix = StrCol::dup(suffix);

    if (mode == OVERWRITE || mode == RENAME_OLD || mode == NEW_SUFFIX)
      m_saveMode = mode;
  } else
    m_saveMode = OVERWRITE;
}

// Save a unit to a file.
bool Project::makeDirHierarchy(Filename path) const {

  // Remember where we are.
  char cwd[PATH_MAX];
  if (SysCall::getcwd(cwd, PATH_MAX, &err()) == 0)
    return false;

  // Change into the directory and create every missing one
  char *path_copy = StrCol::dup(path.name());
  char *p = path_copy;

  // TODO: here is a problem with different drives on Windows!!!
  if (path.is_absolute()) {
    SysCall::chdir(path.root(), &err());
    p += strlen(path.root());
  }

  char *curr = p;
  while (*p != '\0') {
    if (*p == '/') {
      *p = '\0';
      if (strlen(curr) && !makeDir(curr)) {
        delete[] path_copy;
        return false;
      }
      curr = p + 1;
    }
    p++;
  }

  delete[] path_copy;

  // Go back
  SysCall::chdir(cwd, &err());

  return true;
}

// Save a unit to a file.
void Project::save(Unit *unit, bool only_modified) const {
  if (!unit)
    return;
  if (!unit->name())
    return;

  // Save only units that have changed, or all if allowed.
  if (!unit->state().isModified() && only_modified)
    return;

  // Do not write files to protected paths or files from outside the 
  // source directories.
  if (isProtected(unit->name()))
    return;

  // determine the destination path of the file      
  ProjectFile::MapConstIter iter;
  if (!PathManager::isBelow(unit->name(), iter)) {
    assert(false); // if we came here, the file should be registered
    return;
  }
  ProjectFile &project_file = (ProjectFile&) iter->second;
  Filename dest = project_file.dest();
  if (!dest.is_defined()) {
    // determine the destination path
    std::ostringstream path;
    if (!getDestinationPath(unit->name(), path))
      return;
    std::string dest_path = path.str();
    project_file.dest(dest_path.c_str());
    dest = project_file.dest();
  }

  // make sure that the directory for the file exists
  if (!makeDirHierarchy(dest))
    return;

  char *file = (char*) dest.name();
  bool release_file = false;

  // A suffix is given.
  if (m_filenameSuffix && m_saveMode != OVERWRITE) {
    char *newname, *dot;

    // Rename the old file by adding the suffix to the 
    // name of that file.
    if (m_saveMode == RENAME_OLD) {
      newname = new char[strlen(file) + strlen(m_filenameSuffix) + 1];
      sprintf(newname, "%s%s", file, m_filenameSuffix);
      rename(file, newname);
    }

    // Save the file with a new suffix.
    else {
      newname = StrCol::dup(file);
      dot = strrchr(newname, '.');
      if (dot)
        *dot = '\0';
      file = new char[strlen(newname) + strlen(m_filenameSuffix) + 2];
      release_file = true;
      sprintf(file, "%s.%s", newname, m_filenameSuffix);
    }

    delete[] newname;
  }

  // Check whether the file has to be updated.
  if (isFileOutdated(file, unit)) {
    // Write the file to disk.
    std::ofstream out(file, std::ios::out);
    write(unit, out);
  }

  // Update the state of the unit (modification flag,
  // last modification time, last save location).
  updateUnitState(file, unit);

  if (release_file)
    delete[] file;
}

bool Project::isFileOutdated(const char *file, Unit *unit) const {
  if (unit->state().lastModified()) {
    FileInfo fileinfo;
    if (SysCall::stat(file, fileinfo) && unit->state().lastModified() <= fileinfo.modi_time()) {
      return false;
    }
  }
  return true;
}

// Write the unit to the file.
void Project::write(Unit *unit, std::ofstream &out) const {
  if (unit) {
    out << GPL_HEADER;
    unit->print(out);
  }
}

// Update the state of the given unit.
void Project::updateUnitState(const char *file, Unit *unit) const {
  unit->state().unmodified();
  FileInfo fileinfo;
  if (SysCall::stat(file, fileinfo, &err()))
    unit->state().lastModified(fileinfo.modi_time());

  if (unit->isFile()) {
    // Remember the save location.
    char buf[PATH_MAX];
    char *cwd = SysCall::getcwd(buf, PATH_MAX, &err());
    if (!cwd)
      return;

    char *location = new char[strlen(cwd) + strlen(file) + 5];

    if (cwd[strlen(cwd) - 1] == '/')
      sprintf(location, "%s%s", cwd, file);
    else
      sprintf(location, "%s/%s", cwd, file);

    ((FileUnit*) unit)->setLocation(location);

    delete[] location;
  }
}

// Change to the directory, or create it if necessary.
bool Project::makeDir(const char *directory) const {
  while (!SysCall::chdir(directory))
    if (!SysCall::mkdir(directory, &err())) {
      err() << sev_error << "unable to create directory \"" << directory << "\"." << endMessage;
      return false;
    }
  return true;
}

// Save a FileUnit (or all) to a file (or files).
void Project::save(const char *file, bool only_modified, bool is_reg_ex) const {
  UnitManager::UMap &umap = m_unitManager.getTable();

  // No file name is given. Save ALL units.
  if (!file) {
    for (UnitManager::UMapIter iter = umap.begin(); iter != umap.end(); ++iter) {
      Unit *unit = (*iter).second;
      if (unit->isFile())
        save(unit, only_modified);
    }
    // A file name or a name pattern is given.
  } else {
    RegComp *regcomp = 0;
    if (is_reg_ex)
      regcomp = new RegComp(file);

    for (UnitManager::UMapIter iter = umap.begin(); iter != umap.end(); ++iter) {
      Unit *unit = (*iter).second;
      if (unit->isFile()) {
        if (regcomp) {
          if (regcomp->match(unit->name()))
            save(unit, only_modified);
          else
            regcomp->comperror(err());
        } else if (strcmp(unit->name(), file) == 0)
          save(unit, only_modified);
      }
    }

    if (regcomp)
      delete regcomp;
  }
}

// Print an unit, regular expression for more units.
void Project::print(const char *name, std::ostream &out, bool is_reg_ex) const {
  if (!name)
    name = ".*";

  RegComp *regcomp = 0;
  if (is_reg_ex)
    regcomp = new RegComp(name);

  UnitManager::UMap &umap = m_unitManager.getTable();

  for (UnitManager::UMapIter iter = umap.begin(); iter != umap.end(); ++iter) {
    Unit *unit = (*iter).second;
    if (regcomp) {
      if (regcomp->match(unit->name()))
        unit->print(out);
      else
        regcomp->comperror(err());
    } else if (strcmp(unit->name(), name) == 0)
      unit->print(out);
  }

  if (regcomp)
    delete regcomp;
}

// Close a unit, regular expression for more units.
void Project::close(const char *name, bool destroy, bool is_reg_ex) const {
  if (!name) {
    if (destroy)
      m_unitManager.discardAll();
    else
      m_unitManager.discardAll(false);
    return;
  }

  RegComp *regcomp = 0;
  if (is_reg_ex)
    regcomp = new RegComp(name);

  UnitManager::UMap &umap = m_unitManager.getTable();

  for (UnitManager::UMapIter iter = umap.begin(); iter != umap.end();) {
    UnitManager::UMapIter entry = iter;
    ++iter;
    Unit *unit = (*entry).second;
    if (regcomp) {
      if (regcomp->match(unit->name()))
        m_unitManager.discard(unit->name(), unit->isFile(), destroy);
      else
        regcomp->comperror(err());
    } else if (strcmp(unit->name(), name) == 0) {
      m_unitManager.discard(unit->name(), unit->isFile(), destroy);
    }
  }

  if (regcomp)
    delete regcomp;
}

// Add a new file with destination filename to the project.
Unit *Project::addFile(Filename file, Filename dest) {
  Unit *new_unit = new FileUnit;
  new_unit->name(file.name());
  m_unitManager.add(new_unit);
  PathManager::addFile(file, dest);
  return new_unit;
}

// Add a new file with destination filename to the project.
Unit *Project::addFile(Filename file) {
  Unit *new_unit = new FileUnit;
  new_unit->name(file.name());
  m_unitManager.add(new_unit);
  PathManager::addFile(file);
  return new_unit;
}

bool Project::isNewer(const char *file) const {
  if (!file)
    return false;

  FileInfo fileinfo;
  long last_modified;

  // 1. Get the last modification time of the source file.

  Unit *unit = m_unitManager.get(file, true);
  // File already known and scanned.
  if (unit) {
    last_modified = unit->state().lastModified();
    if (!last_modified)
      return true; // Manipulated unit.
    // File not scanned up to now. Examine the file itself.
  } else {
    if (!SysCall::stat(file, fileinfo))
      return false; // File does not exists.
    last_modified = fileinfo.modi_time();
  }

  // 2. Compare it with the last modification time of the file
  //    in the destination directory.

  // determine the destination path of the file      
  ProjectFile::MapConstIter iter;
  if (!PathManager::isBelow(file, iter)) {
    assert(false); // if we came here, the file should be registered
    return false;
  }

  ProjectFile &project_file = (ProjectFile&) iter->second;
  Filename dest = project_file.dest();
  if (!dest.is_defined()) {
    // determine the destination path
    std::ostringstream path;
    if (!getDestinationPath(file, path))
      return false;
    std::string dest_path = path.str();
    project_file.dest(dest_path.c_str());
    dest = project_file.dest();
  }

  char *fullname = StrCol::dup(dest.name());

  if (m_filenameSuffix && m_saveMode == RENAME_OLD) {
    char *dot = strrchr(fullname, '.');
    if (dot)
      *dot = '\0';
    char *tmp = new char[strlen(fullname) + strlen(m_filenameSuffix) + 2];
    sprintf(tmp, "%s.%s", fullname, m_filenameSuffix);
    delete[] fullname;
    fullname = tmp;
  }

  bool newer = true;
  if (SysCall::stat(fullname, fileinfo))
    if (last_modified <= fileinfo.modi_time())
      newer = false;

  delete[] fullname;
  return newer;
}

bool Project::isBelow(Unit *unit) const {
  FileUnit *funit;

  if (unit) {
    // if macro expansion unit, get the file containing the macro call
    while (unit && unit->isMacroExp())
      unit = ((MacroUnit*) unit)->CallingUnit();

    // must be a file unit
    if (unit && unit->isFile()) {
      funit = (FileUnit*) unit;

      // file belongs to project?
      if (funit->belongsTo(*(Project*) this))
        return true;
    }
  }

  return false;
}

} // namespace Puma
