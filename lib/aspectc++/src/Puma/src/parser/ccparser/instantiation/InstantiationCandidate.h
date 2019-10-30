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

#ifndef __InstantiationCandidate__
#define __InstantiationCandidate__

#include "Puma/Array.h"
#include "Puma/ErrorSink.h"
#include "Puma/DeducedArgument.h"

namespace Puma {


class CTree;
class Token;
class CRecord;
class CProject;
class CTypeInfo;
class CConstant;
class CStructure;
class CT_ExprList;
class CObjectInfo;
class CT_SimpleName;
class CTemplateInfo;
class CT_TemplateName;
class CCInstantiation;
class CTemplateInstance;
class CT_TemplateArgList;
class CTemplateParamInfo;
class InstantiationParser;
class CT_TemplateParamDecl;

class InstantiationCandidate {
  typedef Array<CTree*> ArgumentList;
  typedef Array<DeducedArgument*> DeducedArgumentList;

  ArgumentList         arguments;
  DeducedArgumentList  darguments;
  DeducedArgumentList  iarguments;
  CObjectInfo         *obj_info;
  CTemplateInfo       *tpl_info;
  CTree               *poi;
  ErrorSink           *err;
  CStructure          *inst_scope;
  CTemplateInstance   *instance;
  InstantiationParser *parser;
  char                 anon_name_buf[100];
  bool                 detected_delayed_parse_node;

public:
  InstantiationCandidate ();
  ~InstantiationCandidate ();

  CTree* instantiate(CCInstantiation* inst);
  InstantiationParser& getParser();

  void reset ();
  void initialize (ErrorSink *e);
  void initialize (CTree *p, CObjectInfo *o);
  void initialize (CTree *p, CObjectInfo *o, CTemplateInfo *t, ErrorSink *e = 0);
  void addArgument (CTree *);
  void addDeducedArg (DeducedArgument *);
  void addInstantiationArg (DeducedArgument *);
  void forgetDeducedArgs ();
  void TemplateInstance (CTemplateInstance *);

  bool deduceArguments (bool real_inst);
  bool deduceArgumentsFromFctCall (unsigned skip = 0, bool exact_match = false, bool partial_ordering = false);
  bool match (InstantiationCandidate &);
  int compare (InstantiationCandidate &);

  bool canBeInstantiated ();

  CObjectInfo *ObjectInfo () const;
  CTemplateInfo *TemplateInfo () const;
  CTree *PointOfInstantiation () const;
  CStructure *InstanceScope () const;
  CTemplateInstance *TemplateInstance () const;

  CStructure *createInstanceScope();

  unsigned Arguments () const;
  unsigned DeducedArgs () const;
  unsigned InstantiationArgs () const;
  CTree *Argument (unsigned) const;
  DeducedArgument *DeducedArg (unsigned) const;
  DeducedArgument *InstantiationArg (unsigned) const;

  int getPosition (CTemplateParamInfo *) const;
  Token* getPointOfInstantiationToken(CTree* point_of_inst = 0) const;

  void printInstantiationHeader() const;
  void printInstantiationErrorHeader (Token* token) const;
  void printArgumentList (std::ostream &out, bool print_default_args = false) const;
  bool maxInstDepthReached ();
  bool detectedDelayedParseNode () const;

private:
  int deduceArguments (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceFromQualifiedType (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceFromArray (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceFromFunction (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceFromMemberPointer (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceFromRecord (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceTemplateParam (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering);
  int deduceArgumentsFromTemplateArgList (CTemplateInstance *fi, CTemplateInstance *ai, DeducedArgumentList &dargs, bool partial_ordering);
  bool matchDirectArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args);
  bool parseDirectArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args);
  bool parseArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args);
  bool joinDeducedArguments (DeducedArgumentList &dargs, DeducedArgumentList &curr_dargs, bool &matching_args, bool &ambiguous_type);
  void getTemplateInstances(CRecord* c, Array<CTemplateInstance*>& instances);
  bool matchArgument (CTree *pexpr, CTree *aexpr);
  bool matchArgument (CTypeInfo *ptype, CTypeInfo *atype, CTree *aexpr);
  bool sameValue (int pos, CConstant *value);
  bool sameType (int pos, CTypeInfo *type);
  bool setDeducedArgument (DeducedArgumentList &args, int pos, DeducedArgument *arg);
  bool matchingTemplateParameters (CTemplateParamInfo *param, CObjectInfo *ainfo);
  CTemplateInfo *getTemplateInfo (CObjectInfo *info);
  CT_SimpleName *findName (CTree *node);
  CT_SimpleName *findPrivateName (CTree *node);
  bool parseNonTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CConstant* value);
  bool parseTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type);
  bool parseTemplateTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type);
  CTemplateParamInfo* getMatchingParameter(CTemplateParamInfo *param);
  void dumpInstanceHeader();
  void calculateDesturbing(Array<CTree*>& desturbing);
  CProject* getProject();
  void printCodeLine(Token* token) const;
  CTypeQualified* getQualifiers(CTypeInfo* type);
  CTypeInfo* setQualification(CTypeInfo* type, bool is_const, bool is_volatile, bool is_restrict);
  CObjectInfo* introduceObjectForNonTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CConstant* value);
  CObjectInfo* introduceObjectForTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type);
  CObjectInfo* introduceObjectForTemplateTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type);
  bool isDependent(CTemplateInfo* tpl) const;
  bool isDependent(CT_TemplateParamDecl* pd) const;
};

inline CTemplateInstance *InstantiationCandidate::TemplateInstance () const
 { return instance; }
inline CObjectInfo *InstantiationCandidate::ObjectInfo () const
 { return obj_info; }
inline CTemplateInfo *InstantiationCandidate::TemplateInfo () const
 { return tpl_info; }
inline CTree *InstantiationCandidate::PointOfInstantiation () const
 { return poi; }
inline CStructure *InstantiationCandidate::InstanceScope () const
 { return inst_scope; }

inline unsigned InstantiationCandidate::Arguments () const 
 { return arguments.length (); }
inline unsigned InstantiationCandidate::DeducedArgs () const
 { return darguments.length (); }
inline unsigned InstantiationCandidate::InstantiationArgs () const
 { return iarguments.length (); }
inline CTree *InstantiationCandidate::Argument (unsigned i) const
 { return arguments.lookup (i); }
inline DeducedArgument *InstantiationCandidate::DeducedArg (unsigned i) const
 { return darguments.lookup (i); }
inline DeducedArgument *InstantiationCandidate::InstantiationArg (unsigned i) const
 { return iarguments.lookup (i); }

inline void InstantiationCandidate::addArgument (CTree *a)
 { arguments.append (a); }
inline void InstantiationCandidate::addDeducedArg (DeducedArgument *darg)
 { darguments.append (darg); }
inline void InstantiationCandidate::addInstantiationArg (DeducedArgument *darg)
 { iarguments.append (darg); }
inline void InstantiationCandidate::forgetDeducedArgs () 
 { darguments.reset (); }
inline bool InstantiationCandidate::detectedDelayedParseNode () const
 { return detected_delayed_parse_node; }

} // namespace Puma

#endif /* __InstantiationCandidate__ */
