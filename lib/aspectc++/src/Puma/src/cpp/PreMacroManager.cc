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

#include "Puma/PreMacroManager.h"
#include "Puma/ErrorStream.h"
#include "Puma/PreMacro.h"
#include "Puma/StrCol.h"
#include <string.h>
#include <stdio.h>

namespace Puma {


void PreMacroManager::clear () {
  MacrosByName::iterator macro = _Macros.begin();
  for (; macro != _Macros.end(); ++macro)
    delete macro->second;
  _Macros.clear();
}

void PreMacroManager::add (PreMacro* m) {
  _Macros[m->getName().c_str()] = m;
}

PreMacro* PreMacroManager::get (const DString& name) const {
  MacrosByName::const_iterator macro = _Macros.find(name.c_str());
  if (macro != _Macros.end())
    return (PreMacro*) macro->second;
  return 0;
}

void PreMacroManager::kill (const DString& name) {
  MacrosByName::iterator macro = _Macros.find(name.c_str());
  if (macro != _Macros.end()) {
    PreMacro* m = macro->second;
    _Macros.erase(macro);
    delete m;
  }
}

// Initialize the macro manager.
void PreMacroManager::init (const char* file)
 {
    // Add special predefined macros to macro table.
    _Line = new PreMacro ("__LINE__", "");
    add (_Line);
    _Date = new PreMacro ("__DATE__", ""); 
    add (_Date);
    _Time = new PreMacro ("__TIME__", "");
    add (_Time);
    _IncLevel = new PreMacro ("__INCLUDE_LEVEL__", "0");
    add (_IncLevel);

    if (! file) {
        _File = new PreMacro ("__FILE__", "");
        _BaseFile = new PreMacro ("__BASE_FILE__", "");
    }
    else {
        char* basefile = new char[strlen (file) + 10];
        sprintf (basefile, "\"%s\"", file);

        _File = new PreMacro ("__FILE__", basefile);
        _BaseFile = new PreMacro ("__BASE_FILE__", basefile);

        delete[] basefile;
    }
    add (_File);
    add (_BaseFile);
 }


// Add a new macro to macro manager.
void PreMacroManager::addMacro (PreMacro *macro) //const
 {
    if (! macro) return;

    // Search for the macro. If found it's a redefinition of the macro.
    if (PreMacro* prev = (PreMacro*) get (macro->getName ())) 
    {
        // Special macro that cannot be (re)defined in the program.
        if (prev->isInhibited () || prev->isLocked ())
            return;

        // A redefinition normally causes a warning. But if the body
        // of the previous version of the macro is empty or not 
        // effectively different from the current macro body, the 
        // redefinition is accepted silently.
        if (! StrCol::onlySpaces (prev->getBody ())) 
        {
            // Effectively different means, that the two bodies differ
            // only in white spaces. It is not allowed to remove white 
            // spaces completely or to add white spaces where no one have
            // been before.
            if (StrCol::effectivelyDifferent (macro->getBody (), 
                                              prev->getBody ())) {
                *_err << macro->location () << sev_warning 
                      << "redefinition of macro `" 
                      << macro->getName () << "'" << endMessage;
                if (~(prev->location ().filename ()))
                  *_err << prev->location () << "previous definition here" 
                        << endMessage;
            }
        }

        // Remove the previous version of the macro.
        kill (macro->getName ());
    }

    // Add the macro to the macro table.
    add (macro);
 }


// Remove macro `name' from macro table.
void PreMacroManager::removeMacro (const DString& name, const Location& location)
 {
    if (name.empty())
        return;

    PreMacro *macro = get (name);

    if (macro) {
        // Macro protected against undefining
        if (macro->isLocked () || macro->isInhibited ()) {
            *_err << location << sev_warning 
                  << "can not undefine macro `" << macro->getName ()
                  << "'" << endMessage;
            return;
        }

        // Undefine special calculated macro
        if (macro == getLineMacro() || macro == getIncLevelMacro() || 
            macro == getDateMacro() || macro == getTimeMacro() || 
            macro == getFileMacro() || macro == getBaseFileMacro()) {

            *_err << location << sev_warning 
                  << "undefining special macro `" << macro->getName ()
                  << "'" << endMessage;

            if (macro == getLineMacro()) _Line = 0;
            if (macro == getIncLevelMacro()) _IncLevel = 0;
            if (macro == getDateMacro()) _Date = 0;
            if (macro == getTimeMacro()) _Time = 0; 
            if (macro == getFileMacro()) _File = 0;
            if (macro == getBaseFileMacro()) _BaseFile = 0;
        }
    }

    kill (name);
 }


// Get macro `name' from macro table.
PreMacro* PreMacroManager::getMacro (const DString& name) const
 {
    if (name.empty())
        return (PreMacro*) 0;

    PreMacro *macro = get (name);

    // Special macro that cannot be defined and 
    // therefore also not be found.
    if (macro && macro->isInhibited ())
        return (PreMacro*) 0;

    return macro;
 }


} // namespace Puma
