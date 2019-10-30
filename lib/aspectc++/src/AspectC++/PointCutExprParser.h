// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#ifndef __point_cut_expr_parser_h__
#define __point_cut_expr_parser_h__

#include <string>
#include "ACConfig.h"

#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"
#include "lexertl/rules.hpp"
#include "lexertl/state_machine.hpp"

class PointCutExpr;
class PointCutSearcher;

class PointCutExprParser {
  lexertl::state_machine state_machine_;
  ACConfig &config_;

  // private constructor; this is a singleton
  PointCutExprParser( ACConfig &config );
  PointCutExpr *parse_or_expr (lexertl::smatch &, PointCutSearcher &searcher);
  PointCutExpr *parse_and_expr (lexertl::smatch &, PointCutSearcher &searcher);
  PointCutExpr *parse_unary_expr (lexertl::smatch &, PointCutSearcher &searcher);
  PointCutExpr *parse_primary_expr (lexertl::smatch &, PointCutSearcher &searcher);

public:
  static PointCutExprParser *instance( ACConfig &config ) {
    static PointCutExprParser inst( config );
    return &inst;
  }
  PointCutExpr *parse (const std::string &input, PointCutSearcher &searcher);
};

#endif // __point_cut_expr_parser_h__
