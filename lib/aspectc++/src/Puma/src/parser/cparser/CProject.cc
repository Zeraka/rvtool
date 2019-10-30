// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#include "Puma/PreprocessorParser.h"
#include "Puma/PathIterator.h"
#include "Puma/StringSource.h"
#include "Puma/ErrorStream.h"
#include "Puma/FileSource.h"
#include "Puma/CProject.h"
#include "Puma/Unit.h"
#include "Puma/CUnit.h"
#include "Puma/UnitIterator.h"
#include "Puma/CTokens.h"
#include "Puma/CTypeInfo.h"
#include <string.h>
#include <stdio.h>

namespace Puma {


CProject::CProject (ErrorStream &e) : 
  Project (e), _config (e), _scanner (e) {
  unitManager ().tokenizer (&_scanner);
}

// Constructor: Add some source directories.
CProject::CProject (ErrorStream &e, const char *defs, const char *decls) : 
  Project (e), _config (e), _scanner (e) {
  unitManager ().tokenizer (&_scanner);
  if (defs)  addPath (defs);
  if (decls) addPath (decls);
}

// Constructor: Setup preprocessor configuration according to
// command line parameter and the global config file and
// add some source directories
CProject::CProject (ErrorStream &e, int &argc, char **&argv, 
                    const char *defs, const char *decls) : 
  Project (e), _config (e), _scanner (e) {
  if (! config ().CustomSystemConfigFile (argc, argv))
    config ().Read (); // read global config file
  config ().Read (argc, argv);
  configure (config ());
  _scanner.configure (config ());
  unitManager ().tokenizer (&_scanner);
  if (defs)  addPath (defs);
  if (decls) addPath (decls);
}

// Configure the C++ project from the command line or a file.
void CProject::configure (Config &c) {
  // Configure the basic project.
  Project::configure (c); 

  // Configure the CScanner
  _scanner.configure (c);

  // Configure size_t and ptrdiff_t types
  unsigned num = c.Options ();
  for (unsigned i = 0; i < num; i++) {
    const ConfOption *o = c.Option (i);
    // Set the internal representation of size_t
    if (! strcmp (o->Name (), "--size-type")) {
      if (o->Arguments () < 1) continue;
      CTypeInfo *type = mapType (o->Argument (0));
      if (type) {
        CTypeInfo::CTYPE_SIZE_T = type;
      }
    // Set the internal representation of ptrdiff_t
    } else if (! strcmp (o->Name (), "--ptrdiff-type")) {
      if (o->Arguments () < 1) continue;
      CTypeInfo *type = mapType (o->Argument (0));
      if (type) {
        CTypeInfo::CTYPE_PTRDIFF_T = type;
      }
    }
  }
}

// Scan the file `file' or the source if given. When
// scanning a source the resulting unit gets the name `file'.
Unit *CProject::scanFile (const char *file, Source *in, bool isFile) {
  if (! file && ! in) 
    return (Unit*)0;
  Unit *unit = (Unit*)0;

  // this line must be removed from here
  _scanner.allow_wildcards (true);
  if (isFile) {
    // If unit has already been built, return that unit.
    if (! (unit = unitManager ().get (file, true))) {
      if (! in) {
        in = new FileSource (file, &err ());
        if (((FdSource*)in)->fd () == -1) {
          delete in;
          return unit; // NULL
        }
      }
      unit = unitManager ().scanSource (file, in, true);
    }
  } else if (in) {
    unit = new Unit;
    unit->name (file);
    _scanner.fill_unit (*in, *unit);
    delete in;
  }
  return unit;
}

// Scan a string. The resulting unit gets the name `name'. 
// (and will not be registered at the unit manager!)
Unit *CProject::scanString (const char *str, const char *name) {
  if (! str) 
    return (Unit*)0;
  Source* in = new StringSource (str);
  return scanFile (name, in, false);
}        

// Add a new file to the project.
Unit* CProject::addFile (Filename file) {
  Unit* result = Project::addFile (file);
  return result;
}

// Give the string needed to include a particular header file
Filename CProject::getInclString (Filename filename) {
  Filename canon_file;
  if (!SysCall::canonical (filename, canon_file))
    return filename;

  for (unsigned i = config ().Options (); i > 0; i--) {
    const ConfOption *o = config ().Option (i-1);
    if (! strcmp (o->Name (), "-I")) {
      if (! o->Arguments ())
        continue;
      Filename canon_inc_dir;
      if (!SysCall::canonical (o->Argument (0), canon_inc_dir))
        continue;

      int len = strlen (canon_inc_dir.name ());
      if (strncmp (canon_inc_dir.name (), canon_file.name (), len) == 0)
        if (*(canon_file.name () + len) == '/')
          return canon_file.name () +len + 1;
    }
  }
  return filename;
}


// Map the type string from --size-type and --ptrdiff-type to real Puma type.
CTypeInfo *CProject::mapType (const char *ttxt) const {
  if (! ttxt) 
    return 0;
  
  // cut off surrounding quotes
  if (*ttxt == '"') 
    ttxt++;
  char *txt = StrCol::dup (ttxt);
  if (txt[strlen(txt)-1] == '"')
    txt[strlen(txt)-1] = '\0';

  // parse type string
  ErrorStream err;
  CUnit unit (err);
  unit << txt << endu;
  delete[] txt;

  int types[TOK_NO], type; 
  memset ((void*)types, 0, sizeof (types));
  
  UnitIterator iter (unit);
  for (; *iter; ++iter) {
    type = iter->type ();
    if (type < TOK_NO)
      types[type]++;
  }
  
  if (types[TOK_INT128] == 1) {
    if (types[TOK_UNSIGNED] == 1)
      return &CTYPE_UNSIGNED_INT128;
    else
      return &CTYPE_INT128;
  } else if (types[TOK_LONG] == 2) {
    if (types[TOK_UNSIGNED] == 1)
      return &CTYPE_UNSIGNED_LONG_LONG;
    else
      return &CTYPE_LONG_LONG;
  } else if (types[TOK_LONG] == 1) {
    if (types[TOK_UNSIGNED] == 1)
      return &CTYPE_UNSIGNED_LONG;
    else
      return &CTYPE_LONG;
  } else if (types[TOK_SHORT] == 1) {
    if (types[TOK_UNSIGNED] == 1)
      return &CTYPE_UNSIGNED_SHORT;
    else
      return &CTYPE_SHORT;
  } else if (types[TOK_INT] == 1) {
    if (types[TOK_UNSIGNED] == 1) 
      return &CTYPE_UNSIGNED_INT;
    else
      return &CTYPE_INT;
  } else if (types[TOK_CHAR] == 1) {
    if (types[TOK_UNSIGNED] == 1)
      return &CTYPE_UNSIGNED_CHAR;
    else if (types[TOK_SIGNED] == 1)
      return &CTYPE_SIGNED_CHAR;
    else
      return &CTYPE_CHAR;
  } else if (types[TOK_UNSIGNED] == 1) {
    return &CTYPE_UNSIGNED_INT;
  } else if (types[TOK_SIGNED] == 1) {
    return &CTYPE_INT;
  }
  
  return 0;
}


} // namespace Puma
