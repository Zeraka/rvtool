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

#ifndef PUMA_Printable_H
#define PUMA_Printable_H

/** \file
 * Object printing. */

#include <iostream>

namespace Puma {

/** \class Printable Printable.h Puma/Printable.h
 * Provides that possibility for an object to be
 * used with output streams (std::ostream). Derived
 * classes have to implement method Printable::print().
 *
 * Example:
 * \code
 * class Name : public Puma::Printable {
 *   const char* the_name;
 * public:
 *   Name(const char* n) : the_name(n) {}
 *   void print(ostream &os) { 
 *     os << "[name: " << the_name << "]"; 
 *   }
 * };
 *
 * Name adam("adam");
 * std::cout << adam << std::endl;  // prints "[name: adam]\n"
 * \endcode 
 * \ingroup basics */
class Printable {
public:
  /** Destructor. */
  virtual ~Printable() {
  }
  /** Print object information on the given output stream.
   * To be implemented by concrete objects.
   * \param os The output stream. */
  virtual void print(std::ostream &os) const = 0;
};

/** Output stream operator for Printable objects.
 * \param os The output stream.
 * \param object The object to print.
 * \return A reference to the output stream. */
inline std::ostream &operator<<(std::ostream &os, const Printable &object) {
  object.print(os);
  return os;
}

} // namespace Puma

#endif /* PUMA_Printable_H */
