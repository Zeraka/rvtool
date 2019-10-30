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

#ifndef __CFileInfo_h__
#define __CFileInfo_h__

/** \file 
 *  Semantic information about a source file (translation unit). */

#include "Puma/CNamespaceInfo.h"

namespace Puma {


class Unit;

/** \class CFileInfo CFileInfo.h Puma/CFileInfo.h
 *  Semantic information about a source file (translation unit). 
 *  A source file has its own scope, the so-called file scope. */
class CFileInfo : public CNamespaceInfo {
  Unit *_Primary;
  bool _haveCBuiltins;
  bool _haveCCBuiltins;

public: 
  /** Constructor. */
  CFileInfo ();
  /** Destructor. If the object type is CObjectInfo::FILE_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CFileInfo ();

  /** Get the token chain of the source file. */
  Unit *Primary () const;
  /** Set the token chain of the source file.
   *  \param unit The token chain. */
  void Primary (Unit *unit);
  
  /** Check if C compiler build-in functions are defined
   *  for the source file. */
  bool haveCBuiltins () const;
  /** Check if C++ compiler build-in functions are defined 
   *  for the source file. */
  bool haveCCBuiltins () const;

  /** Set whether C compiler build-in functions are defined 
   *  for the source file. 
   *  \param v \e true if C build-in functions are defined. */
  void haveCBuiltins (bool v);
  /** Set whether C++ compiler build-in functions are defined 
   *  for the source file. 
   *  \param v \e true if C++ build-in functions are defined. */
  void haveCCBuiltins (bool v);
};

inline CFileInfo::CFileInfo () :
  CNamespaceInfo (CObjectInfo::FILE_INFO),
  _Primary (0), 
  _haveCBuiltins (false),
  _haveCCBuiltins (false)
 {}
inline CFileInfo::~CFileInfo ()
 {}
 
inline Unit *CFileInfo::Primary () const 
 { return _Primary; }
inline void CFileInfo::Primary (Unit *unit) 
 { _Primary = unit; }
 
inline bool CFileInfo::haveCBuiltins () const 
 { return _haveCBuiltins; }
inline void CFileInfo::haveCBuiltins (bool v) 
 { _haveCBuiltins = v; }
inline bool CFileInfo::haveCCBuiltins () const 
 { return _haveCCBuiltins; }
inline void CFileInfo::haveCCBuiltins (bool v) 
 { _haveCCBuiltins = v; }


} // namespace Puma

#endif /* __CFileInfo_h__ */
