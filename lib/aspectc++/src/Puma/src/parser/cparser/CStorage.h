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

#ifndef __CStorage_h__
#define __CStorage_h__

/** \file
 *  C/C++ object storage classes. */

namespace Puma {


/** \class CStorage CStorage.h Puma/CStorage.h
 *  Storage class of an object. Defines the minimum potential 
 *  lifetime of the storage containing the object. There are
 *  three resp. four different storage classes: static, automatic, 
 *  thread, and dynamic. */
struct CStorage {
  /** The storage classes. */
  enum Type {
    /** Static storage class. */
    CLASS_STATIC,
    /** Automatic storage class. */
    CLASS_AUTOMATIC, 
    /** Dynamic storage class. */
    CLASS_DYNAMIC,
    /** Thread local storage (TLS) class. */
    CLASS_THREAD,
    /** Undefined storage class. */
    CLASS_NONE
  };
};


} // namespace Puma

#endif /* __CStorage_h__ */
