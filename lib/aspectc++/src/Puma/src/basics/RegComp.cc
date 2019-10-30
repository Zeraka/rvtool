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

#include "Puma/RegComp.h"
#include "Puma/ErrorStream.h"

#include "lexertl/rules.hpp"
#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"

namespace Puma {

// Constructors
RegComp::RegComp(const std::string &expr)
    : m_expr(expr), m_compiled(false) {
}

RegComp::RegComp(const RegComp &rc)
    : m_expr(rc.m_expr), m_compiled(false) {
}

RegComp &RegComp::operator =(const RegComp &rc) {
  m_expr = rc.m_expr;
  m_error_msg.clear();
  m_compiled = false;
  m_state.clear();
  return *this;
}

// Compile the regular expression.
bool RegComp::compile(const std::string &expr) {
  m_compiled = true;
  lexertl::rules rules;
  rules.push(m_expr, 1); // id is '1', because it is not needed; but != 0
  try {
    lexertl::generator::build(rules, m_state); // gen. state machine representation
  } catch (const std::exception &e) {
    m_error_msg = e.what();
    return false;
  }
  return true;
}

// Match against a string.
bool RegComp::match(const std::string &str) {
  if (!m_compiled)
    compile(m_expr);

  std::string input(str);
  std::string::const_iterator iter = input.begin();
  std::string::const_iterator end = input.end();
  lexertl::match_results<std::string::const_iterator, std::size_t> results(iter, end);

  try {
    lexertl::lookup(m_state, results);
  } catch (const std::exception &e) {
    m_error_msg = e.what();
    m_error_msg += std::string("; matched '") + str + "' with regex '" + m_expr + "'";
    return false;
  }
  return (results.id == 1);
}

// To be called on errors.
void RegComp::comperror(ErrorStream &err) const {
  if (error())
    err << sev_error << "RegComp: " << m_error_msg.c_str() << endMessage;
}

} // namespace Puma
