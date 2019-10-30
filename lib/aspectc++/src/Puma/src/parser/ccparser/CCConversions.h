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

#ifndef __CCConversions_h__
#define __CCConversions_h__

// §4, §13.3.3.1 implicit conversion sequences

#include "Puma/ErrorSink.h"

namespace Puma {


class CTree;
class CTypeInfo;
class CCConvSeq;
class CClassInfo;
class CCOverloading;

class CCConversions {
  ErrorSink &err;
 
public:
  // usage: (error stream, is_operator_overloading)
  CCConversions (ErrorSink &);
  
  // determine conversions
  CCConvSeq *implicitConversions (CTypeInfo *, CTypeInfo *, CTree *, CTree *, bool = true);
  CCConvSeq *standardConversions (CTypeInfo *, CTypeInfo *, CTree *, CTree *);
  CCConvSeq *userDefinedConversions (CTypeInfo *, CTypeInfo *, CTree *, CTree *);
  CCConvSeq *referenceBinding (CTypeInfo *, CTypeInfo *, CTree *, CTree *, bool = true);
  CCConvSeq *ellipsisConversions ();

  static CTypeInfo *integralPromotion (CTypeInfo *);
  static CTypeInfo *arithmeticPromotion (CTypeInfo *);
  static CTypeInfo *usualArithmeticConv (CTypeInfo *, CTypeInfo *);

  // compare conversion sequences
  static int compareConvSeq (CCConvSeq *, CCConvSeq *);

  // conversion related operations on types
  static bool similarTypes (CTypeInfo *, CTypeInfo *);
  static bool referenceCompatible (CTypeInfo *, CTypeInfo *);
  static bool referenceRelated (CTypeInfo *, CTypeInfo *);
  static bool equalOrMoreQualified (CTypeInfo *, CTypeInfo *);

protected:
  // compare conversions
  static int compareSameRankConvSeq (CCConvSeq *, CCConvSeq *);
  static unsigned convSequenceRank (CCConvSeq *);
  static bool properSubsequence (CCConvSeq *, CCConvSeq *);

  // helper
  bool isValidConversion (CTree *, CTypeInfo *, CTypeInfo *, CCOverloading &) const;
  static bool baseClass (CTypeInfo *, CTypeInfo *);
  static bool baseClass (CClassInfo *, CClassInfo *);
  static CClassInfo *extractClass (CTypeInfo *, bool);
};

inline CCConversions::CCConversions (ErrorSink &e) : err (e) {}


} // namespace Puma

#endif /* __CCConversions_h__ */
