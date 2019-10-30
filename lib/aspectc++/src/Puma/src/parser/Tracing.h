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

#ifndef __Tracing__
#define __Tracing__

/** \file
 *  Tracing support for the parser. */

#include <iostream>

namespace Puma {


class Token;

/** \class Tracing Tracing.h Puma/Tracing.h
 *  Implements tracing support for the parser. */
class Tracing {
  int _trace;
  int _tdepth;
  std::ostream *_tos;

protected:
  /** Constructor. */
  Tracing ();
  
public:
  /** Setup the tracer.
   *  \param os The output stream on which to write the tracing output.
   *  \param trace_tokens Turn on/off token level tracing. */
  void trace (std::ostream& os, bool trace_tokens = false);

protected:
  /** Trace the given token. 
   *  \param token The token type. */
  void trace (int token); 
  /** Trace the given parser rule.
   *  \param rule The rule name.
   *  \param n The indentation depth.
   *  \param quote Quote the rule name. */
  void trace (const char *rule, int n = 1, bool quote = false); 
  /** Trace the given token.
   *  \param token The token to trace.
   *  \param ok The message if the token is not NULL.
   *  \param failed The message if the token is NULL. */
  void trace (Token *token, const char *ok, const char *failed); 
  /** Get the current tracing mode. 0 means tracing is disabled,
   *  1 means rule level tracing, 2 means token level tracing. */
  int trace_mode () const;
  /** Get the tracing output stream. */
  std::ostream &tos () const;

private:
  const char *token_text (int) const; 
};

inline Tracing::Tracing () : 
  _trace (0), 
  _tdepth (0),
  _tos ((std::ostream*)0) {}

inline void Tracing::trace (int token) { trace (token_text (token), 1, true); }
inline int Tracing::trace_mode () const { return _trace; }


} // namespace Puma

#endif /* __Tracing__ */
