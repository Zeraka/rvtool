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

#include "Puma/Tracing.h"
#include "Puma/CTokens.h"
#include "Puma/Token.h"
#include "Puma/CTree.h"
#include <iostream>

namespace Puma {


ostream &Tracing::tos () const {
  return *_tos;
}

void Tracing::trace (std::ostream &os, bool trace_tokens) { 
  _tos = &os; 
  _trace = trace_tokens ? 2 : 1; 
}

void Tracing::trace (const char *rule, int n, bool quote) { 
  if (! _trace) return;
  if (n == -1) _tdepth += n; 
  for (int i = 0; i < _tdepth; i++)
    *_tos << "| ";
  if (quote) *_tos << "`";
  *_tos << rule;
  if (quote) *_tos << "'";
  *_tos << endl;
  if (n == 1) _tdepth += n; 
}

void Tracing::trace (Token *token, const char *ok, const char *failed) { 
  if (! _trace) return;
  if (token) {
    *_tos << token->location () << ": " << ok 
          << ": " << token->text () << std::endl;
  } else 
    *_tos << failed << std::endl;
}

const char *Tracing::token_text (int token_type) const { 
  const char* text = Token::get_static_text (token_type);
  return text ? text : "<unknow>";
}


} // namespace Puma
