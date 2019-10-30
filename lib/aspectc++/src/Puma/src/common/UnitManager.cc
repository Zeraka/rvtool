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

#include "Puma/Unit.h"
#include "Puma/Source.h"
#include "Puma/SysCall.h"
#include "Puma/FileUnit.h"
#include "Puma/FileSource.h"
#include "Puma/ErrorStream.h"
#include "Puma/UnitManager.h"
#include "Puma/Tokenizer.h"
#include <string.h>
#include <assert.h>

namespace Puma {

// Destructor: Delete all units.
UnitManager::~UnitManager() {
  discardNonames();
  discardAll();
}

// Destroy all noname units.
void UnitManager::discardNonames() {
  for (std::list<Unit*>::iterator iter = _nonames.begin(); iter != _nonames.end(); ++iter)
    delete *iter;
  _nonames.clear();
}

// Get a unit by its name.
Unit *UnitManager::get(const char *name, bool is_file) const {
  if (!name)
    return (Unit*) 0;

  char *abs_name = (char*) 0;
  if (is_file)
    abs_name = SysCall::absolute(name);

  UMapIter entry = _umap.find(abs_name ? abs_name : name);
  Unit *unit = 0;
  if (entry != _umap.end())
    unit = (*entry).second;

  if (abs_name)
    delete[] abs_name;

  // If that unit is a closed file unit, return NULL, because
  // it has to be filled again.
  if (unit && unit->isFile() && unit->empty())
    return (Unit*) 0;

  return unit;
}

// Add a unit to manager.
void UnitManager::add(Unit *unit) {
  if (!unit)
    return;

  // Noname units will be stored separately.
  if (!unit->name())
    _nonames.push_back(unit);
  else {
    const char *abs_name = 0;
    if (unit->isFile())
      abs_name = SysCall::absolute(unit->name());

    _umap.insert(UMapEntry(abs_name ? abs_name : unit->name(), unit));

    if (abs_name)
      delete[] abs_name;
  }
}

// Remove a unit from manager.
void UnitManager::discard(const char *name, bool is_file, bool destroy) const {
  char *abs_name = (char*) 0;
  if (is_file)
    abs_name = SysCall::absolute(name);

  UMapIter entry = _umap.find(abs_name ? abs_name : name);
  if (entry != _umap.end()) {
    Unit *unit = (*entry).second;
    if (destroy) {
      delete unit;
      _umap.erase(entry);
    } else
      unit->clear();
  }
  if (abs_name)
    delete[] abs_name;
}

// Remove all units from manager.
void UnitManager::discardAll(bool destroy) const {
  for (UMapIter iter = _umap.begin(); iter != _umap.end(); ++iter) {
    if (destroy)
      delete (*iter).second;
    else
      (*iter).second->clear();
  }
  if (destroy)
    _umap.clear();
}

// Build a new unit. The source will be deleted.
Unit *UnitManager::scanSource(const char *name, Source *in, bool is_file) {
  Unit *unit;

  // Bad arguments.
  if (!name && !in)
    return (Unit*) 0;

  if (name) {
    char *abs_name = (char*) 0;
    if (is_file)
      abs_name = SysCall::absolute(name);

    UMapIter entry = _umap.find(abs_name ? abs_name : name);
    Unit *unit = 0;
    if (entry != _umap.end())
      unit = (*entry).second;
    char *location = (char*) 0;

    // If that unit is a closed file unit, it has to be 
    // filled again but from its last location.
    if (unit && unit->isFile() && unit->empty())
      if (((FileUnit*) unit)->lastLocation())
        location = ((FileUnit*) unit)->lastLocation();

    if (location && in) {
      delete in;
      in = (Source*) 0;
    }

    if (!in) {
      if (!location)
        location = (char*) name;
      in = new FileSource(location);
    }

    // Destroy the old version of that very unit.
    if (unit) {
      delete unit;
      _umap.erase(abs_name ? abs_name : name);
    }

    if (abs_name)
      delete[] abs_name;
  }

  if (is_file) {
    unit = new FileUnit();

    // Remember the last modification time of the file.
    FileInfo fileinfo;
    if (SysCall::fstat(((FdSource*) in)->fd(), fileinfo, _err))
      unit->state().lastModified(fileinfo.modi_time());
  } else
    unit = new Unit();

  if (name)
    unit->name(name);

  assert(_tokenizer);
  _tokenizer->fill_unit(*in, *unit);

  add(unit);

  delete in;
  return unit;
}

// Print units.
void UnitManager::print(const char *name, bool is_file, std::ostream &out) const {
  if (name) {
    char *abs_name = (char*) 0;
    if (is_file)
      abs_name = SysCall::absolute(name);

    UMapIter entry = _umap.find(abs_name ? abs_name : name);
    if (entry != _umap.end())
      (*entry).second->print(out);

    if (abs_name)
      delete[] abs_name;
  }
}

void UnitManager::init() {
}

} // namespace Puma
