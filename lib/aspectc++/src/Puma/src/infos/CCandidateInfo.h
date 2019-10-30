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

#ifndef __CCandidateInfo_h__
#define __CCandidateInfo_h__

/** \internal Internal part of the overload resolution mechanism. */

/** \file 
 *  Representation of a C++ overload resolution candidate. */

#include "Puma/CCConvSeq.h"
#include "Puma/Array.h"

namespace Puma {

class CFunctionInfo;

/** \class CCandidateInfo CCandidateInfo.h Puma/CCandidateInfo.h
 *  Representation of a candidate for the C++ function overload 
 *  resolution mechanism. Such a candidate stores the candidate 
 *  function and the conversion sequences of the function call 
 *  arguments needed to match the function parameter types of
 *  the overloaded function.
 *  In special situations it is possible that the overload
 *  resolution selects a function that hasn't been parsed, yet,
 *  because of delayed parsing of member functions and the
 *  initializers of their arguments. Here is an example:
 *
 *  \code
 * struct has_deref {
 *   template <typename Type> static long test(Type *, Type * = 42);
 *   static char test(void *);
 *   static const int value = sizeof(test((long *) 0));
 * };
 *  \endcode
 *
 *  Here the first 'test' is selected, but an error message will
 *  have to be issued. This is not considered as a substitution
 *  fault, which would have to be ignored ("SFINAE").
 *  The '_delayed' attribute indicates this situation. */

class CCandidateInfo {
  Array<CCConvSeq*> _ConvSequences;
  CFunctionInfo *_Function;
  CCConvSeq *_SecondConvSeq;
  bool _delayed;

public: 
  /** Constructor.
   *  \param fi The candidate function. */
  CCandidateInfo (CFunctionInfo *fi);
  /** Destructor. */
  ~CCandidateInfo ();

  /** Get the candidate function. */
  CFunctionInfo *Function () const;
  /** Set the candidate function. 
   *  \param fi The candidate function. */
  void Function (CFunctionInfo *fi);

  /** Mark the candidate as erroneous due to delayed parsing. */
  void markDelayedParseProblem ();
  
  /** Get the delayed parsing problem flag. */
  bool hasDelayedParseProblem () const;

  /** Add a conversion sequence for a function argument. 
   *  The conversion sequences are added in the order of 
   *  the arguments of the function call.
   *  \param seq The conversion sequence. */
  void addConvSequence (CCConvSeq *seq);
  
  /** Get the number of the conversion sequences. */
  unsigned ConvSequences () const;
  /** Get the n-th conversion sequence. The n-th conversion sequence 
   *  corresponds to the n-th function call argument. 
   *  \param n The conversion sequence or NULL if \e n is invalid. */
  CCConvSeq *ConvSequence (unsigned n);
  
  /** Set the \e second conversion sequence. This is a user-defined
   *  conversion by means of a user-defined conversion function. 
   *  \param seq The conversion sequence. */
  void SecondConvSeq (CCConvSeq *seq);
  /** Get the \e second conversion sequence. */
  CCConvSeq *SecondConvSeq () const;
};

inline CCandidateInfo::CCandidateInfo (CFunctionInfo *f) :
  _Function (f),
  _SecondConvSeq ((CCConvSeq*)0),
  _delayed (false)
 {}
inline CCandidateInfo::~CCandidateInfo () 
 { 
   for (long i = 0; i < _ConvSequences.length (); i++) {
     if (_ConvSequences.fetch (i))
       delete _ConvSequences.fetch (i);
   }
   if (_SecondConvSeq) delete _SecondConvSeq;
 }

inline CFunctionInfo *CCandidateInfo::Function () const
 { return _Function; }
inline void CCandidateInfo::Function (CFunctionInfo *f) 
 { _Function = f; }

inline void CCandidateInfo::markDelayedParseProblem ()
 { _delayed = true; }

inline bool CCandidateInfo::hasDelayedParseProblem () const
 { return _delayed; }

inline void CCandidateInfo::addConvSequence (CCConvSeq *s)
 { _ConvSequences.append (s); }

inline unsigned CCandidateInfo::ConvSequences () const
 { return (unsigned)_ConvSequences.length (); }
inline CCConvSeq *CCandidateInfo::ConvSequence (unsigned s)
 { return _ConvSequences.lookup (s); }

inline void CCandidateInfo::SecondConvSeq (CCConvSeq *s) 
 { _SecondConvSeq = s; }
inline CCConvSeq *CCandidateInfo::SecondConvSeq () const
 { return _SecondConvSeq; }


} // namespace Puma

#endif /* __CCandidateInfo_h__ */
