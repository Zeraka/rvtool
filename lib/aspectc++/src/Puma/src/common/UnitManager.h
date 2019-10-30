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

#ifndef PUMA_UnitManager_H
#define PUMA_UnitManager_H

/** \file
 * Token unit management. */

#include <iostream>
#include <list>
#include <map>
#include <string>
#include "Puma/ErrorStream.h"

namespace Puma {

class Unit;
class Source;
class Tokenizer;

/** \class UnitManager UnitManager.h Puma/UnitManager.h
 * Class to manage the units built to parse the files of
 * the source project.
 * \ingroup common */
class UnitManager {
public:
  /** Name to unit map type. */
  typedef std::map<std::string, Unit*> UMap;
  /** Name to unit map entry type. */
  typedef UMap::value_type UMapEntry;
  /** Name to unit map iterator type. */
  typedef UMap::iterator UMapIter;

private:
  ErrorStream *_err;
  std::list<Unit*> _nonames;
  mutable UMap _umap;
  Tokenizer *_tokenizer;

public:
  /** Constructor.
   * \param err Error stream to use for reporting errors. */
  UnitManager(ErrorStream &err)
      : _err(&err), _tokenizer(0) {
  }
  /** Destructor. Deletes all managed units. */
  virtual ~UnitManager();

  /** Get the unit management table.
   * \return A reference to the unit table. */
  UMap &getTable() const {
    return _umap;
  }

  /** Get the tokenizer (scanner) that is used by scanSource()
   * to tokenize sources.
   * \return A pointer to the tokenizer used. */
  Tokenizer *tokenizer() const {
    return _tokenizer;
  }
  /** Set the tokenizer (scanner) to be used by scanSource()
   * to tokenize sources.
   * \param tokenizer The tokenizer to use. */
  void tokenizer(Tokenizer *tokenizer) {
    _tokenizer = tokenizer;
  }

  /** Add a token unit to the manager.
   * \param unit The unit to add. */
  void add(Unit *unit);

  /** Get a managed unit by name.
   * \param name The name of the unit to get.
   * \param isfile True to indicate that the given name refers to a file (defaults to false).
   * \return A pointer to the unit. */
  Unit *get(const char *name, bool isfile = false) const;

  /** Discard a unit managed by this unit manager.
   * \param name The name of the unit to remove.
   * \param isfile True to indicate that the given name refers to a file (defaults to false).
   * \param destroy True to indicate that the unit shall be destroyed (defaults to true). */
  void discard(const char *name, bool isfile = false, bool destroy = true) const;

  /** Discard all named units managed by this unit manager.
   * \param destroy True to indicate that the units shall be destroyed (defaults to true). */
  void discardAll(bool destroy = true) const;

  /** Discard all no-name units managed by this unit manager. */
  void discardNonames();

  /** Scan the tokens from the given input source and create a unit from it.
   * The new unit is added to the unit manager.
   * \param name The name of the unit to create.
   * \param in The input source.
   * \param isfile True to indicate that the given name refers to a file (defaults to false).
   * \return A pointer to the new unit created. */
  Unit *scanSource(const char *name, Source *in = 0, bool isfile = false);

  /** Print a named unit on the given output stream.
   * \param name The name of the unit to print.
   * \param isfile True to indicate that the given name refers to a file (defaults to false).
   * \param out The output stream, defaults to std::cout. */
  void print(const char *name, bool isfile = false, std::ostream &out = std::cout) const;

  /** Initialize the unit manager and the managed units for the next parse process. */
  void init();
};

} // namespace Puma

#endif /* PUMA_UnitManager_H */
