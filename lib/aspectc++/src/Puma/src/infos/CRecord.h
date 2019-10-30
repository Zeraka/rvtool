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

#ifndef __CRecord_h__
#define __CRecord_h__

/** \file 
 *  Semantic information about a class or union. */

#include "Puma/CStructure.h"
#include "Puma/Stack.h"
#include <map>
#include <list>

namespace Puma {


class CTemplateInfo;


/** \class CRecord CRecord.h Puma/CRecord.h
 *  Semantic information about a class or union. */
class CRecord : public CStructure {
public:
  typedef std::list<CTree*> TreeList;
  typedef std::map<Token*,TreeList> InstantiateDelayedMap;

private:
  CTemplateInfo *_TemplateInfo;
  bool _isTemplate;
  bool _hiddenFwdDecl;

  // Parts of record to parse delayed (function bodies, initializers, ...)
  Array<CTree*> _Delayed;
  // Stack used to reject delayed parsing of record parts
  Stack<unsigned> _DelayedLength;
  // Delayed record parts instantiation map
  InstantiateDelayedMap _InstantiateDelayed;
  // True if complete parsed
  bool _CompleteParsed;

protected:
  /** Constructor.
   *  \param id The object type. */
  CRecord (ObjectId id);

public:
  /** Destructor. */
  ~CRecord ();

  /** Get the template information if this is a class or union template.
   *  \return The template information or NULL if not a template. */
  CTemplateInfo *TemplateInfo () const;
  /** Set the template information of a class/union template.
   *  \param info The template information. */
  void TemplateInfo (CTemplateInfo *info);

  /** Set the qualification scope of a nested class or union 
   *  or class/union declared in a namespace.
   *  \param scope The qualification scope. */
  void QualifiedScope (CStructure *scope);

  /** Check if the class/union is complete (defined)
   *  at the given source code position. The position
   *  is specified by the unique number of the CT_Token
   *  tree node representing the name of the class/union.
   *  \param pos The source code position. */
  bool isComplete (unsigned long pos = 0) const;
  /** Check if the class/union is defined. */
  bool isDefined () const;
  /** Check if the class/union is a template. */
  bool isTemplate () const;
  /** Check if this is a class/union forward declaration
   *  hidden by another declaration in the same scope. */
  bool isHiddenForwardDecl () const;
  /** Check if this is a trivial class/union */
  bool isTrivial () const;

  /** Yields true if the class has a trivial copy assignment operator. */
  bool hasTrivialAssign () const;
  /** Yields true if the class has a trivial copy constructor. */
  bool hasTrivialCopy () const;
  /** Yields true if the class has a trivial default constructor. */
  bool hasTrivialCtor () const;
  /** Yields true if the class has a trivial destructor. */
  bool hasTrivialDtor () const;

  /** Set whether this class/union is a template.
   *  \param v \e true if it is a template. */
  void isTemplate (bool v);
  /** Set whether this is a class/union forward declaration
   *  hidden by another declaration in the same scope. 
   *  \param v \e true if it is a hidden forward declaration. */
  void isHiddenForwardDecl (bool v);

public: // Parse helper
  /** Return true if record is complete parsed. */
  bool isCompleteParsed () const;
  /** Set if record is complete parsed. */
  void isCompleteParsed (bool v);

  /** Add a part of record to be parse delayed. */
  void addParseDelayed (CTree* tree);
  /** Get the number of parts of record to be parsed delayed. */
  unsigned ParseDelayed () const;
  /** Get the n-th part of record to be parsed delayed. */
  CTree* ParseDelayed (unsigned n) const;

  /** Save the current length of the delayed record parts list. */
  void saveParseDelayedListLength ();
  /** Forget the last saved length of the delayed record parts list. */
  unsigned forgetParseDelayedListLength ();
  /** Reject all parts of record to be parsed delayed added behind
   *  the last saved position in the list. */
  void rejectParseDelayed ();

  /** Add a part of record to be instantiated delayed. */
  void addInstantiateDelayed (Token* start, CTree* tree);
  /** Return true if there are template instances for which to instantiate the record part. */
  bool haveInstantiateDelayed (Token* start) const;
  /** Get the list of instances in which to instantiate the record part. */
  const TreeList& InstantiateDelayed (Token* start) const;

  /** Reset the list of parts of record to be parsed or instantiated delayed. */
  void resetDelayed ();
  /** Remove the part of record to be parsed or instantiated delayed. */
  void removeDelayed (CTree* tree);
};

inline CRecord::CRecord (CObjectInfo::ObjectId id) :
  CStructure (id),
  _TemplateInfo ((CTemplateInfo*)0),
  _isTemplate (false),
  _hiddenFwdDecl (false),
  _Delayed (5, 5),
  _DelayedLength (1, 2),
  _CompleteParsed (false)
 {}
 
inline bool CRecord::isTemplate () const 
 { return _isTemplate; }
inline void CRecord::isTemplate (bool v)
 { _isTemplate = v; }

inline bool CRecord::isHiddenForwardDecl () const 
 { return _hiddenFwdDecl; }
inline void CRecord::isHiddenForwardDecl (bool v)
 { _hiddenFwdDecl = v; }
  
inline CTemplateInfo *CRecord::TemplateInfo () const
 { return _TemplateInfo; }
inline void CRecord::TemplateInfo (CTemplateInfo *info) 
 { _TemplateInfo = info; }

inline void CRecord::QualifiedScope (CStructure *s)
 { _QualScope = s; }

inline bool CRecord::isCompleteParsed () const
 { return _CompleteParsed; }
inline void CRecord::isCompleteParsed (bool v)
 { _CompleteParsed = v; }

inline void CRecord::addParseDelayed (CTree* tree)
 { _Delayed.append (tree); }
inline unsigned CRecord::ParseDelayed () const
 { return _Delayed.length (); }
inline CTree* CRecord::ParseDelayed (unsigned n) const
 { return _Delayed.lookup (n); }

inline void CRecord::saveParseDelayedListLength ()
 { _DelayedLength.push (_Delayed.length ()); }
inline unsigned CRecord::forgetParseDelayedListLength ()
 { unsigned len = _DelayedLength.top(); _DelayedLength.pop (); return len; }

inline void CRecord::addInstantiateDelayed (Token* start, CTree* tree)
 { _InstantiateDelayed[start].push_back (tree); }
inline bool CRecord::haveInstantiateDelayed (Token* start) const
 { return _InstantiateDelayed.find (start) != _InstantiateDelayed.end (); }
inline const CRecord::TreeList& CRecord::InstantiateDelayed (Token* start) const
 { return _InstantiateDelayed.find (start)->second; }

inline void CRecord::resetDelayed ()
 { _Delayed.reset (); _DelayedLength.reset (); _InstantiateDelayed.clear (); }

} // namespace Puma

#endif /* __CRecord_h__ */
