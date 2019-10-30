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

#ifndef __CCOverloading_h__
#define __CCOverloading_h__

#include "Puma/Array.h"
#include "Puma/ErrorSink.h"
#include "Puma/CCConversions.h"
#include "Puma/CCandidateInfo.h"

namespace Puma {


class CTree;
class CRecord;
class CTypeList;
class CFunctionInfo;
class CCNameLookup;
class CTypeFunction;
class CSemDatabase;
class CT_SimpleName;
class CT_ArgDeclList;

class CCOverloading {
  Array<CCandidateInfo*> _Candidates;
  CTree *_Arguments; // ordinary function argument list
  CTree *_Arg0;      // operator arguments
  CTree *_Arg1;
  CTree *_Arg2;
  CTree *_Base;
  CTree *_Poi;
  unsigned _NumArgs;
  
  ErrorSink &err;
  bool user_def;     // user-defined conversions allowed
  bool operator_ovl; // operator overloading
  bool dependent;    // dependent call
 
  CCConversions cvs; // implicit conversions

  CTypeInfo *obj_type;

public:
  // usage: (error stream, is_operator_overloading)
  inline CCOverloading (ErrorSink &, bool = false);
  
  // destructor: delete all CCandidateInfos
  inline ~CCOverloading ();
  
  // set qualifiers
  void ObjectType (CTypeInfo *);
  void PointOfInst (CTree *);

  // usage: resolve(argument(s), allow_user_defined_conversions)
  void resolve (CTree *, bool = true);
  void resolve (CTree *, CTree *, bool = true);
  void resolve (CTree *, CTree *, CTree *, bool = true);
  void resolveMember (CTree *, CTree *);

  // the resulting function to call
  CFunctionInfo *Function () const;
  bool isAmbiguous () const;
  bool isDependent () const;

  unsigned Candidates () const;
  CCandidateInfo *Candidate (unsigned) const;
  void addCandidate (CFunctionInfo *);
  void removeCandidate (unsigned);

  void collectConvFunctions (CRecord *);
  void collectConstructors (CRecord *, bool default_constr = false);
  void addCandidates (CCNameLookup &);
  
  // create built-in operators used as candidates for 
  // operator overloading, operator overloading mode
  // must be enabled (else nothing is done)
  void createBuiltinOperators (CSemDatabase *db, const char *,
    int, CTree *, CTree *);

  unsigned Arguments () const;
  CTree *Argument (unsigned) const;

private: // built-in operators
  void createIncrOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createDecrOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createMulOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createPlusOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createMinusOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createTildeOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createMembPtrOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createIndexOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createIfThenOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createLogOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createRelOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createBinOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createEqAssOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createAssOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createDivOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createEqOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createMulAssOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);
  void createAddAssOp (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *);

private:
  void chooseViableFcts ();
  void chooseBestViableFct ();
  int compareCandidates (CCandidateInfo *, CCandidateInfo *) const;

  int moreSpecialized (CFunctionInfo *, CFunctionInfo *) const;
  bool equalOrMoreQualified (CTypeFunction *, CTypeInfo *) const;
  bool hasDefaultArgument (CFunctionInfo *, CTypeList *, unsigned) const;
  void collectConvFunctions (CRecord *, unsigned);
  void collectConvTypes (CRecord *, Array<CTypeInfo*> &);
  CCStandardConvSeq *implicitThisConv (CTypeInfo *, CFunctionInfo *) const;
  CCConvSeq *implicitConversions (CTypeInfo *, CTree *, CTypeInfo *, CTree *);
  CT_SimpleName* isAddressOfFct (CTree *, bool = false) const;
  CT_ArgDeclList* getArgumentList (CTree* tree) const;

  void createOperator (CSemDatabase *, int, const char *, CTypeInfo *, CTypeInfo *, CTypeInfo * = 0);
};

inline CCOverloading::CCOverloading (ErrorSink &e, bool oovl) : 
  _Arguments (0),
  _Arg0 (0),
  _Arg1 (0),
  _Arg2 (0),
  _Poi (0),
  _NumArgs (0),
  err (e), 
  user_def (true), 
  operator_ovl (oovl),
  dependent (false),
  cvs (e),
  obj_type (0)
 {}

inline CCOverloading::~CCOverloading () {
  for (int c = (int)Candidates () - 1; c >= 0; c--)
    removeCandidate ((unsigned)c);
}

inline CFunctionInfo *CCOverloading::Function () const
 { return Candidates () ? Candidate (0)->Function () : (CFunctionInfo*)0; }
inline unsigned CCOverloading::Arguments () const 
 { return _NumArgs; }

inline bool CCOverloading::isAmbiguous () const
 { return (Candidates () > 1); }
inline bool CCOverloading::isDependent () const 
 { return dependent; }

inline unsigned CCOverloading::Candidates () const
 { return (unsigned)_Candidates.length (); }
inline CCandidateInfo *CCOverloading::Candidate (unsigned c) const
 { return _Candidates.lookup (c); }
inline void CCOverloading::removeCandidate (unsigned c) {
  delete _Candidates.lookup (c);
  _Candidates.remove (c);
}


} // namespace Puma

#endif /* __CCOverloading_h__ */
