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

#include "Puma/MatchCollector.h"
#include "Puma/ErrorStream.h"
#include "Puma/CCParser.h"
#include "Puma/Config.h"
#include "Puma/CUnit.h"

namespace Puma {


MatchCollector::MatchCollector (const char *match_pattern) {
  if (! match_pattern) 
    return;

  ErrorStream err;
  CUnit *unit;
  
  unit = new CUnit (err);
  *unit << match_pattern;
  unit->scan ();
  unit->name ("%match pattern%");

  own_pattern = true;
  pattern = unit;
  
  own_parser = true;
  parser = new CCParser;
}


MatchCollector::MatchCollector (Parser &p, const char *match_pattern) {
  if (! match_pattern) 
    return;

  ErrorStream err;
  CUnit *unit;
  
  unit = new CUnit (err);
  *unit << match_pattern;
  unit->scan ();
  unit->name ("%match pattern%");

  own_pattern = true;
  pattern = unit;
  
  own_parser = false;
  parser = &p;
}


MatchCollector::MatchCollector (Unit &unit) {
  own_pattern = false;
  pattern = &unit;
  
  own_parser = true;
  parser = new CCParser;
}


MatchCollector::MatchCollector (Parser &p, Unit &unit) {
  own_pattern = false;
  pattern = &unit;

  own_parser = false;
  parser = &p;
}


MatchCollector::~MatchCollector () {
  for (int i = 0; i < matches.length (); i++)
    delete matches[i];
  if (own_parser) 
    delete parser;
  if (own_pattern)
    delete pattern;
}


void MatchCollector::action (CMatch *match) {
  matches.append (match);
}


void MatchCollector::collect (CProject &p, CTranslationUnit &tu, CTree *node) {
  // reset match array
  for (int i = 0; i < matches.length (); i++)
    delete matches[i];
  matches.reset ();
  
  // configure parser
  ErrorStream err;
  Config c (err);
  c.Add ("--match-expr");
  parser->configure (c);

  // parse match pattern
  CTranslationUnit *mtu;
  mtu = parser->parse (*pattern, p);

  // start matching
  match (tu, *mtu, node);
  
  delete mtu;
}


void MatchCollector::defMacro (const char *name, const char *body) {
  if (! name) 
    return;
    
  ErrorStream err;
  Config c (err);
  c.Add ("-D", name, body);
  parser->configure (c);
}


} // namespace Puma
