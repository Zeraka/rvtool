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

#include "Puma/CSourceInfo.h"
#include "Puma/MacroUnit.h"
#include "Puma/Unit.h"
#include "Puma/CTree.h"
#include "Puma/Token.h"
#include <string.h>

namespace Puma {


Unit *CSourceInfo::SrcUnit () const { 
  if (! StartToken ()) 
    return (Unit*)0;

  Token *token = StartToken ()->token ();
  if (! token) 
    return (Unit*)0;
    
  Unit *unit = (Unit*)token->belonging_to ();
  while (unit && unit->isMacroExp ()) {
    unit = ((MacroUnit*)unit)->CallingUnit ();
  }
  return unit;
}

const char *CSourceInfo::FileName () const { 
  if (! StartToken ()) 
    return (const char*)0;
  return StartToken ()->token ()->location ().filename ().name ();
}

int CSourceInfo::Line () const { 
  if (! StartToken ()) 
    return 0;
  return StartToken ()->token ()->location ().line (); 
}

int CSourceInfo::Column () const { 
  if (! StartToken ()) 
    return 0;
  return StartToken ()->token ()->location ().column (); 
}

bool CSourceInfo::operator ==(const Token *token) const {
  if (StartToken () && StartToken ()->token () == token) 
    return true;
  return false;
}

bool CSourceInfo::operator ==(const CT_Token *token) const {
  if (StartToken () && token && StartToken ()->Number () == token->Number ()) 
    return true;
  return false;
}

bool CSourceInfo::operator <(const CT_Token *token) const {
  if (StartToken () && token && StartToken ()->Number () < token->Number ()) 
    return true;
  return false;
}

bool CSourceInfo::operator >(const CT_Token *token) const {
  if (StartToken () && token && StartToken ()->Number () > token->Number ()) 
    return true;
  return false;
}

bool CSourceInfo::operator ==(const CSourceInfo &info) const {
  if (! (*this == info.StartToken ()->token ()))
    if (Line () != info.Line () || 
        Column () != info.Column () || 
        ! FileName () || ! info.FileName () ||
        strcmp (FileName (), info.FileName ()) != 0)
      return false; 
  return true;
}

bool CSourceInfo::operator <(const CSourceInfo &info) const {
  if (Line () >= info.Line ()) 
    return false;
  if (Column () >= info.Column ()) 
    return false;
  if (! FileName () || ! info.FileName ()) 
    return false;
  if (strcmp (FileName (), info.FileName ()) != 0) 
    return false;
  return true;  
}

bool CSourceInfo::operator >(const CSourceInfo &info) const {
  if (Line () <= info.Line ()) 
    return false;
  if (Column () <= info.Column ()) 
    return false;
  if (! FileName () || ! info.FileName ()) 
    return false;
  if (strcmp (FileName (), info.FileName ()) != 0) 
    return false;
  return true;   
}


} // namespace Puma
