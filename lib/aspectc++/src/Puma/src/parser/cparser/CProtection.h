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

#ifndef __CProtection_h__
#define __CProtection_h__

/** \file
 *  C++ class member protection. */

namespace Puma {


/** \class CProtection CProtection.h Puma/CProtection.h
 *  Access protection of C++ class members for the purpose
 *  of member access control. There are three kinds of
 *  protection: private, public, and protected. The
 *  protection either is defined implicitely or explicitely
 *  using member access specifiers. */
struct CProtection {
  /** The member access types. */
  enum Type {
    /** Public member access. */
    PROT_PUBLIC, 
    /** Protected member access. */
    PROT_PROTECTED, 
    /** Private member access. */
    PROT_PRIVATE,
    /** Undefined member access. */
    PROT_NONE
  };
};


} // namespace Puma

#endif /* __CProtection_h__ */
