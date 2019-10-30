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

#ifndef __CSpecifiers__
#define __CSpecifiers__

/** \file
 *  C/C++ declaration specifiers. */

namespace Puma {


/** \class CSpecifiers CSpecifiers.h Puma/CSpecifiers.h
 *  C/C++ declaration specifiers for the declaration 
 *  of an entity. */
class CSpecifiers {
public:
  /** Declaration specifiers. */
  enum Spec {
    /** No declaration specifier. */
    SPEC_NONE     = 0x0,
    /** Declaration specifier \c virtual. */
    SPEC_VIRTUAL  = 0x1,
    /** Declaration specifier \c static. */
    SPEC_STATIC   = 0x2,
    /** Declaration specifier \c extern. */
    SPEC_EXTERN   = 0x4,
    /** Declaration specifier \c mutable. */
    SPEC_MUTABLE  = 0x8,
    /** Declaration specifier \c register. */
    SPEC_REGISTER = 0x10,
    /** Declaration specifier \c explicit. */
    SPEC_EXPLICIT = 0x20,
    /** Declaration specifier \c auto. */
    SPEC_AUTO     = 0x40,
    /** Declaration specifier \c inline. */
    SPEC_INLINE   = 0x80,
    /** Declaration specifier \c __thread. */
    SPEC_THREAD   = 0x100
  };

private:
  Spec _specs;
  
public:
  /** Constructor. */
  CSpecifiers ();
  /** Copy-constructor.
   *  \param copy The declaration specifiers to copy. */
  CSpecifiers (const CSpecifiers &copy);
  /** Assignment operator.
   *  \param s The assigned declaration specifiers. */
  CSpecifiers &operator =(const CSpecifiers &s);

  /** Add a declaration specifier. 
   *  \param s The declaration specifier. */
  CSpecifiers &operator +=(Spec s);
  /** Remove a declaration specifier. 
   *  \param s The declaration specifier. */
  CSpecifiers &operator -=(Spec s);
  
  /** Check if the given declaration specifier was specified.
   *  \param s The declaration specifier. */
  bool operator ==(Spec) const;
  /** Check if the given declaration specifier was not specified.
   *  \param s The declaration specifier. */
  bool operator !=(Spec) const;
};

inline CSpecifiers::CSpecifiers () : _specs (SPEC_NONE) 
 {}
inline CSpecifiers::CSpecifiers (const CSpecifiers &s)
 { _specs = s._specs; }
inline CSpecifiers &CSpecifiers::operator =(const CSpecifiers &s)
 { _specs = s._specs; return *(CSpecifiers*)this; }

inline CSpecifiers &CSpecifiers::operator +=(CSpecifiers::Spec s)
 { _specs = (Spec)(_specs | s); return *(CSpecifiers*)this; }
inline CSpecifiers &CSpecifiers::operator -=(CSpecifiers::Spec s)
 { _specs = (Spec)(_specs ^ (_specs & s)); return *(CSpecifiers*)this; }

inline bool CSpecifiers::operator ==(CSpecifiers::Spec s) const
 { return (_specs & s); }
inline bool CSpecifiers::operator !=(CSpecifiers::Spec s) const
 { return ! (_specs & s); }


} // namespace Puma

#endif /* __CSpecifiers__ */
