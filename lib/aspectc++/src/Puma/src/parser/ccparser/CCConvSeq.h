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

#ifndef __CCConvSeq__
#define __CCConvSeq__

#include "Puma/CCConversion.h"
#include "Puma/Array.h"

namespace Puma {


class CCConvSeq : public CCConversion {
  Array<CCConversion*> _Conversions;
  bool _isReferenceBinding;

protected:
  CCConvSeq (ConvId);

public:
  virtual ~CCConvSeq ();

  void Apply ();
  
  void addConversion (CCConversion *);

  CCConversion *Conversion (unsigned);
  unsigned Conversions () const;
  
  bool isReferenceBinding () const;
  void isReferenceBinding (bool);
};

inline CCConvSeq::CCConvSeq (ConvId id) : 
  CCConversion (id),
  _Conversions (3, 1),
  _isReferenceBinding (false)
 {}
inline CCConvSeq::~CCConvSeq ()
 { for (long i = 0; i < _Conversions.length (); i++)
     if (_Conversions.fetch (i))
       delete _Conversions.fetch (i);
 }

inline void CCConvSeq::addConversion (CCConversion *c) 
 { _Conversions.append (c); }

inline CCConversion *CCConvSeq::Conversion (unsigned c) 
 { return _Conversions.lookup (c); }
inline unsigned CCConvSeq::Conversions () const
 { return (unsigned)_Conversions.length (); }
  
inline bool CCConvSeq::isReferenceBinding () const
 { return _isReferenceBinding; }
inline void CCConvSeq::isReferenceBinding (bool v) 
 { _isReferenceBinding = v; }

// the actual conversion sequences

class CCStandardConvSeq : public CCConvSeq {
public:
  CCStandardConvSeq () :
    CCConvSeq (STANDARD_SEQ) {}
};

class CCUserDefinedConvSeq : public CCConvSeq {
public:
  CCUserDefinedConvSeq () :
    CCConvSeq (USER_DEFINED_SEQ) {}
};

class CCEllipsisConvSeq : public CCConvSeq {
public:
  CCEllipsisConvSeq () :
    CCConvSeq (ELLIPSIS_SEQ) {}
};


} // namespace Puma

#endif /* __CCConvSeq__ */
