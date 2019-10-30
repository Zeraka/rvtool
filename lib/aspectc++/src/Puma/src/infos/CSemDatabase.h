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

#ifndef __CSemDatabase_h__
#define __CSemDatabase_h__

/** \file 
 *  Semantic information database. */

#include "Puma/CScopeInfo.h"
#include "Puma/CProject.h"
#include <iostream>
#include <map>
#include <set>

namespace Puma {


class CObjectInfo;
class CArgumentInfo;
class CAttributeInfo;
class CBaseClassInfo;
class CClassInfo;
class CClassInstance;
class CEnumInfo;
class CEnumeratorInfo;
class CFunctionInfo;
class CFctInstance;
class CLabelInfo;
class CLocalScope;
class CMemberAliasInfo;
class CNamespaceInfo;
class CTemplateInfo;
class CTemplateInstance;
class CTemplateParamInfo;
class CTypedefInfo;
class CUnionInfo;
class CUnionInstance;
class CTypeInfo;
class CFileInfo;
class Token;


/** \class CSemDatabase CSemDatabase.h Puma/CSemDatabase.h
 *  Semantic information database. Contains all semantic objects
 *  created during the semantic analysis for one translation
 *  unit. */
class CSemDatabase {
public:
  typedef std::set<CObjectInfo*> ObjectSet;

private:
  Array<CObjectInfo*> _Classes;    // index of classes
  Array<CObjectInfo*> _Unions;     // index of unions
  Array<CObjectInfo*> _Enums;      // index of enums
  Array<CObjectInfo*> _Typedefs;   // index of typedefs
  Array<CObjectInfo*> _Functions;  // index of functions
  Array<CObjectInfo*> _Files;      // index of files

  CProject *_Project; // associated project

  ObjectSet _Objects; // set of all semantic objects
  std::multimap<int,CFunctionInfo*> _builtin_ops; // map for built-in operator lookup

public:
  /** Constructor. 
   *  \param prj The project information.
   *  \param size Initial size of the database (not yet used!). */
  CSemDatabase (CProject &prj, int size = 997);
  /** Destructor. Destroys all semantic information objects in
   *  the database. */
  virtual ~CSemDatabase ();

  /** Get the set of all semantic objects. */
  ObjectSet& Objects();

  /** Get the number of semantic objects. */
  unsigned ObjectInfos () const;
  /** Get the number of semantic objects for classes. */
  unsigned ClassInfos () const;
  /** Get the number of semantic objects for unions. */
  unsigned UnionInfos () const;
  /** Get the number of semantic objects for enumerations. */
  unsigned EnumInfos () const;
  /** Get the number of semantic objects for typedefs. */
  unsigned TypedefInfos () const;
  /** Get the number of semantic objects for functions. */
  unsigned FunctionInfos () const;
  /** Get the number of semantic objects for translation units (file scope). */
  unsigned FileInfos () const;

  /** Get the n-th semantic object.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CObjectInfo *ObjectInfo (unsigned n) const;
  /** Get the n-th semantic object for classes.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CClassInfo *ClassInfo (unsigned n) const;
  /** Get the n-th semantic object for unions.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CUnionInfo *UnionInfo (unsigned n) const;
  /** Get the n-th semantic object for enumerations.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CEnumInfo *EnumInfo (unsigned n) const;
  /** Get the n-th semantic object for typedefs.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CTypedefInfo *TypedefInfo (unsigned n) const;
  /** Get the n-th semantic object for functions.
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CFunctionInfo *FunctionInfo (unsigned n) const;
  /** Get the n-th semantic object for translation units (file scope).
   *  \param n The index of the object.
   *  \return The object or NULL if \e n is invalid. */
  CFileInfo *FileInfo (unsigned n) const;

  /** Get the semantic object for the entity at the given
   *  source code position (token). 
   *  \param pos The token of the entity. 
   *  \return The semantic object or NULL. */
  CObjectInfo *ObjectInfo (Token *pos) const; 
  /** Get the semantic object for the entity at the given
   *  source code position (token). 
   *  \param pos The token of the entity. 
   *  \return The semantic object or NULL. */
  CObjectInfo *ObjectInfo (CT_Token *pos) const; 

public:
  CArgumentInfo *newArgument ();
  CAttributeInfo *newAttribute ();
  CBaseClassInfo *newBaseClass ();
  CClassInfo *newClass ();
  CClassInstance *newClassInstance ();
  CEnumInfo *newEnum ();
  CEnumeratorInfo *newEnumerator ();
  CFunctionInfo *newFunction ();
  CFctInstance *newFctInstance ();
  CLabelInfo *newLabel ();
  CLocalScope *newLocalScope ();
  CMemberAliasInfo *newMemberAlias ();
  CNamespaceInfo *newNamespace ();
  CTemplateInfo *newTemplate ();
  CTemplateParamInfo *newTemplateParam ();
  CTypedefInfo *newTypedef ();
  CUnionInfo *newUnion ();
  CUnionInstance *newUnionInstance ();
  CUsingInfo *newUsing ();
  CFileInfo *newFile ();

public:
  /** Insert a new semantic object into the database.
   *  \param info The semantic object. */
  void Insert (CObjectInfo *info);
  /** Remove the given semantic object from the database.
   *  \param info The semantic object. */
  void Remove (CObjectInfo *info);
  /** Get the semantic object for the given built-in operator.
   *  \param name The operator name/symbol.
   *  \param tok The operator token type.
   *  \param rtype The result type of the operator.
   *  \param t0 Type of the first operand.
   *  \param t1 Type of the second operand, or NULL if only one operand. */
  CFunctionInfo *BuiltinOperator (const char *name, int tok, CTypeInfo *rtype, CTypeInfo *t0, CTypeInfo *t1);
  /** Dump the contents of the database. The dump is indented as
   *  tree corresponding to the nesting of the semantic objects.
   *  \param out The output stream.
   *  \param depth The maximum indentation depth (0 means infinite). 
   *  \param dump_builtins Dump or ignore builtin function, types and objects. */
  void Dump (std::ostream &out, int depth = 0, bool dump_builtins = true) const;

  /** Get the project information. */
  CProject *Project () const;

private:
  void CreateParameter (CFunctionInfo *fi, CTypeInfo *type) const;

  void Dump (std::ostream &, CStructure *, int, int, bool) const;
  void DumpType (std::ostream &, CObjectInfo *, int) const;
  void DumpUsing (std::ostream &, CUsingInfo *, int) const;
  void DumpFriends (std::ostream &, CStructure *) const;
  void DumpFunction (std::ostream &, CFunctionInfo *, int, int) const;
  void DumpAttribute (std::ostream &, CAttributeInfo *, int) const;
  void DumpNamespace (std::ostream &, CNamespaceInfo *, int) const;
  void DumpQualities (std::ostream &, CObjectInfo *) const;
  void DumpScopeName (std::ostream &, CStructure *) const;
  void DumpLocalScope (std::ostream &, CObjectInfo *, int) const;
  void DumpTemplateParam (std::ostream &, CTemplateParamInfo *, int) const;
  void DumpTemplateInstance (std::ostream &, CObjectInfo *, int) const;
  void indent (std::ostream &, int) const;
};

inline CSemDatabase::CSemDatabase (CProject &p, int size) :
  _Classes (20, 50),
  _Unions (20, 50),
  _Enums (20, 50),
  _Typedefs (20, 50),
  _Functions (20, 50),
  _Files (5, 20),
  _Project (&p)
 {}

inline CSemDatabase::ObjectSet& CSemDatabase::Objects()
 { return _Objects; }

inline unsigned CSemDatabase::ObjectInfos () const
 { return ClassInfos () + UnionInfos () + EnumInfos () + 
          TypedefInfos () + FunctionInfos () + FileInfos (); }
inline unsigned CSemDatabase::ClassInfos () const
 { return _Classes.length (); }
inline unsigned CSemDatabase::UnionInfos () const
 { return _Unions.length (); }
inline unsigned CSemDatabase::EnumInfos () const
 { return _Enums.length (); }
inline unsigned CSemDatabase::TypedefInfos () const
 { return _Typedefs.length (); }
inline unsigned CSemDatabase::FunctionInfos () const
 { return _Functions.length (); }
inline unsigned CSemDatabase::FileInfos () const
 { return _Files.length (); }

inline CClassInfo *CSemDatabase::ClassInfo (unsigned i) const
 { return (CClassInfo*)_Classes.lookup (i); }
inline CUnionInfo *CSemDatabase::UnionInfo (unsigned i) const
 { return (CUnionInfo*)_Unions.lookup (i); }
inline CEnumInfo *CSemDatabase::EnumInfo (unsigned i) const
 { return (CEnumInfo*)_Enums.lookup (i); }
inline CTypedefInfo *CSemDatabase::TypedefInfo (unsigned i) const
 { return (CTypedefInfo*)_Typedefs.lookup (i); }
inline CFunctionInfo *CSemDatabase::FunctionInfo (unsigned i) const
 { return (CFunctionInfo*)_Functions.lookup (i); }
inline CFileInfo *CSemDatabase::FileInfo (unsigned i) const
 { return (CFileInfo*)_Files.lookup (i); }

inline CProject *CSemDatabase::Project () const
 { return _Project; }


} // namespace Puma

#endif /* __CSemDatabase_h__ */
