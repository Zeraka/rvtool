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

#ifndef __MatchCollector__
#define __MatchCollector__

// Specialized CTreeMatcher implementing the action
// method. Collects all matches found.

#include "Puma/Unit.h"
#include "Puma/Array.h"
#include "Puma/Parser.h"
#include "Puma/CProject.h"
#include "Puma/CTreeMatcher.h"

namespace Puma {


class MatchCollector : public CTreeMatcher {
  Unit *pattern;           // match pattern
  Parser *parser;          // for parsing the match pattern
  Array<CMatch*> matches;  // collected matches

  bool own_parser;
  bool own_pattern;

public:
  MatchCollector (Unit &);
  MatchCollector (const char *);
  MatchCollector (Parser &, Unit &);
  MatchCollector (Parser &, const char *);
  virtual ~MatchCollector ();
        
  void defMacro (const char *, const char *);
  void collect (CProject &, CTranslationUnit &, CTree * = (CTree*)0);

  unsigned int Matches () const { return matches.length (); }
  CMatch *Match (unsigned int i) const { return matches.lookup (i); }

private:
  void action (CMatch*);
};


} // namespace Puma

#endif /* __MatchCollector__ */
