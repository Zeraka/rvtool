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

#ifndef __lexer_h__
#define __lexer_h__

/** \page lexer Lexical Analysis
 */

#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"
#include "lexertl/rules.hpp"
#include "lexertl/state_machine.hpp"

#include "Puma/LexerMap.h"

namespace Puma {

#define LID(x) ((lexertl::rules::id_type)(const char*) x)

class Lexer {
protected:
  // bit mask that can be used to describe different lexer configurations
  unsigned int _config_mask;

  // implementation of the lexer: a lexertl state machine
  lexertl::state_machine _state_machine;

protected:
  // create the state machine that is needed to scan
  // the tokens (to be done only once!)
  void setup (unsigned int config) {
    // setup the state machine upon the first access
    _config_mask = config;
    lexertl::rules rules;
    add_rules (rules);
    lexertl::generator::build (rules, _state_machine);
  }

  // singleton pattern; only one instance of a given ID!
  template<class L>
  static L& instance(unsigned int id) {
    typedef LexerMap<L> Map;
    static Map lexer_map;
    typename Map::iterator elem = lexer_map.find (id);
    if (elem == lexer_map.end ()) {
      elem = lexer_map.create (id);
      elem->second->setup (id);
    }
    return *(elem->second);
  }

public:
  // virtual destructor needed to avoid warning
  virtual ~Lexer () {}

  // define all regular expressions for the tokens
  virtual void add_rules (lexertl::rules &rules) {}

  // function called by 'add_rules' used to define the keywords
  virtual void add_keywords (lexertl::rules &rules) {}
};

} // namespace Puma

#endif /* __lexer_h__ */
