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

#ifndef __CLinkage_h__
#define __CLinkage_h__

/** \file
 *  Entity name linkage. */

namespace Puma {


/** \class CLinkage CLinkage.h Puma/CLinkage.h
 *  Linkage of an entity name (object, function, etc). The 
 *  linkage controls where a name is visible. There are
 *  three types of linkage: internal, external, and
 *  no linkage. Names with external linkage are visible outside
 *  the object file where they occur. Names with internal or 
 *  no linkage are only visible in one object file.
 *
 *  The linkage is implicitely defined by the scope in which
 *  the entity is declared. With the linkage specifier 'extern'
 *  an entity name can be explicitely declared to have external 
 *  linkage. */
struct CLinkage {
  /** The linkage types. */
  enum Type {
    /** Internal linkage. */
    LINK_INTERNAL,  
    /** External linkage. */
    LINK_EXTERNAL,  
    /** No linkage. */
    LINK_NONE       
  };
};


} // namespace Puma

#endif /* __CLinkage_h__ */
