// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_RegComp_H
#define PUMA_RegComp_H

/** \file
 *  Wrapper for regular expression matching. */

#include <string>
#include "lexertl/state_machine.hpp"
#include "Puma/ErrorStream.h"

namespace Puma {

/** \class RegComp RegComp.h Puma/RegComp.h
 * Regular expression matching. This class compiles the
 * expression when a string is matched for the first time.
 * In case of an error, an error message can be printed on an
 * error stream.
 * \ingroup basics */
class RegComp {
  std::string m_expr;             // the regular expression
  std::string m_error_msg;        // error message in case of problems
  bool m_compiled;                // determines whether the state machine is valid
  lexertl::state_machine m_state; // internal: lexertl compiled regular expression

public:
  /** Constructor.
   * \param expr The regular expression. */
  RegComp(const std::string &expr);
  /** Copy-constructor.
   * \param copy The regular expression to copy. */
  RegComp(const RegComp &copy);

  /** Assign a regular expression.
   * \param copy The regular expression.
   * \return A reference to this object. */
  RegComp &operator =(const RegComp &copy);

  /** Compile the regular expression.
   * \param expr The regular expression.
   * \return False in case of an errors. Use comperror() to print the error message. */
  bool compile(const std::string &expr);

  /** Match against a string.
   * \param str String to be matched.
   * \return True if the regular expression matches the string. */
  bool match(const std::string & str);

  /** Print a regular expression compilation error message on the
   * given error stream. To be called when compile() fails.
   * \param err Error stream on which to print the error message. */
  void comperror(ErrorStream &err) const;

  /** Check whether there was an error compiling the regular expression.
   * \return True in case of an error. Use comperror to print it. */
  bool error() const {
    return !m_error_msg.empty();
  }
};

} // namespace Puma

#endif /* PUMA_RegComp_H */
