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

#ifndef PUMA_LanguageID_H
#define PUMA_LanguageID_H

/** \file
 * Language identifier implementation. */

namespace Puma {

/** \class LanguageID LanguageID.h Puma/LanguageID.h
 * A language ID is a unique identifier for a language.
 * It is used to group tokens, like identifiers, keywords,
 * preprocessor directives, comments, and so on.
 * \ingroup common */
class LanguageID {
  const char *m_id;

public:
  /** Construct an empty object. */
  LanguageID()
      : m_id(0) {
  }
  /** Constructor.
   * \param id The identifier. */
  LanguageID(const char *id)
      : m_id(id) {
  }
  /** Conversion operator.
   * \return The identifier string. */
  operator const char *() const {
    return m_id;
  }
  /** Compare with another language identifier.
   * \param id The other language identifier.
   * \return True if it is the same language identifier object. */
  bool operator ==(const LanguageID &id) const {
    return m_id == (const char*) id;
  }
};

} // namespace Puma

#endif /* PUMA_LanguageID_H */
