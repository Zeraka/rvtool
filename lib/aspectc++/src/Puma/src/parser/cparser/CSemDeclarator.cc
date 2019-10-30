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

#include "Puma/CSemDeclarator.h"
#include "Puma/CObjectInfo.h"
#include "Puma/ErrorSink.h"
#include "Puma/CTree.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CConstant.h"

namespace Puma {


/*DEBUG*/extern int TRACE_TYPE;

void TypeText (std::ostream &out, CTypeInfo *t, int d) {
  for (int i = 0; i < d; i++)
    out << "| ";
  if (t) {
    if (t->TypeQualified ()) {
      out<<"TypeQualified";
      if (t->TypeQualified ()->isConst ()) out<<" const";
      if (t->TypeQualified ()->isVolatile ()) out<<" volatile";
      if (t->TypeQualified ()->isRestrict ()) out<<" restrict";
      out<<std::endl;
    } else if (t->TypeFunction ()) {
      out<<"TypeFunction";
      if (t->TypeFunction ()->isConst ()) out<<" const";
      if (t->TypeFunction ()->isVolatile ()) out<<" volatile";
      if (t->TypeFunction ()->isRestrict ()) out<<" restrict";
      out<<std::endl;
    } else if (t->TypePointer ()) {
      if (t->TypeMemberPointer ()) 
        out<<"TypeMemberPointer"<<std::endl;
      else
        out<<"TypePointer"<<std::endl;
    } else if (t->TypeAddress ()) {
      out<<"TypeAddress"<<std::endl;
    } else if (t->TypeBitField ()) {
      out<<"TypeBitField "<<t->TypeBitField ()->Dimension ()<<std::endl;
    } else if (t->TypeArray ()) {
      if (t->TypeVarArray ())
        out<<"TypeVarArray"<<std::endl;
      else
        out<<"TypeArray "<<t->TypeArray ()->Dimension ()<<std::endl;
    } else if (t->TypeClass ()) {
      out<<"TypeClass"<<std::endl;
    } else if (t->TypeUnion ()) {
      out<<"TypeUnion"<<std::endl;
    } else if (t->TypeEnum ()) {
      out<<"TypeEnum"<<std::endl;
    } else if (t->TypePrimitive ()) {
      out<<"TypePrimitive "<<t->TypePrimitive ()->TypeName ()<<std::endl;
    } else if (t->TypeEmpty ()) {
      out<<"TypeEmpty"<<std::endl;
    } else if (t->TypeTemplateParam ()) {
      out<<"TypeTemplateParam"<<std::endl;
    } else
      out<<"<unknown>"<<std::endl;
      
    if (t->BaseType () && t != t->BaseType ())
      TypeText (out, t->BaseType (), d + 1);
  } else
    out<<"NULL"<<std::endl;
}

CSemDeclarator::CSemDeclarator (ErrorSink *err, CTypeInfo *t, CTree *d, bool lang_c) :
  _err (err), _type (t), _name (0) {
  CConstant *v;
  CTree *node = d;
  while (true) {
    if (node->NodeName () == CT_InitDeclarator::NodeId ()) {
      node = ((CT_InitDeclarator*)node)->Declarator ();
    } else if (node->NodeName () == CT_RefDeclarator::NodeId ()) {
      _type = new CTypeAddress (_type);
      node = ((CT_RefDeclarator*)node)->Declarator ();
    } else if (node->NodeName () == CT_PtrDeclarator::NodeId ()) {
      _type = new CTypePointer (_type);
      HandleQualifiers (((CT_PtrDeclarator*)node)->Qualifier ());
      node = ((CT_PtrDeclarator*)node)->Declarator ();
    } else if (node->NodeName () == CT_MembPtrDeclarator::NodeId ()) {
      CObjectInfo *cinfo = ((CT_MembPtrDeclarator*)node)->Name ()->Object ();
      if (cinfo && cinfo->TypedefInfo ()) {
        CTypeInfo* realtype = cinfo->TypedefInfo ()->TypeInfo ();
        if (realtype) {
          realtype = realtype->VirtualType ();
          if (realtype->TypeRecord () && realtype->TypeRecord ()->Record ()) {
            cinfo = realtype->TypeRecord ()->Record ();
          }
        }
      }
      _type = new CTypeMemberPointer (_type, cinfo);
      HandleQualifiers (((CT_MembPtrDeclarator*)node)->Qualifier ());
      node = ((CT_MembPtrDeclarator*)node)->Declarator ();
    } else if (node->NodeName () == CT_BitFieldDeclarator::NodeId ()) {
      _type = new CTypeBitField (_type);
      CT_BitFieldDeclarator *bfd = (CT_BitFieldDeclarator*)node;
      if (bfd->Expr () && bfd->Expr ()->Value ())
        if ((v = bfd->Expr ()->Value ()->Constant ()))
          _type->TypeBitField ()->Dimension (v->convert_to_uint ());
      node = bfd->Declarator ();
    } else if (node->NodeName () == CT_BracedDeclarator::NodeId ()) {
      node = ((CT_BracedDeclarator*)node)->Declarator ();
    } else if (node->NodeName () == CT_ArrayDeclarator::NodeId ()) {
      CT_ArrayDeclarator *ad = (CT_ArrayDeclarator*)node;
      CT_ArrayDelimiter *delim = ad->Delimiter (); 
      bool is_const = false, is_volatile = false, is_restrict = false;
      if (delim->Qualifier ())
        for (int i = delim->Qualifier ()->Sons () - 1; i >= 0; i--) {
          if (delim->Qualifier ()->Son (i)->token ()->type () == TOK_CONST)
            is_const = true;
          else if (delim->Qualifier ()->Son (i)->token ()->type () == TOK_VOLATILE)
             is_volatile = true;
          else if (delim->Qualifier ()->Son (i)->token ()->type () == TOK_RESTRICT)
            is_restrict = true;
        }
      CTypeQualified *quals = 0;
      if (is_const || is_volatile || is_restrict)
        quals = new CTypeQualified (&CTYPE_EMPTY, is_const, is_volatile, is_restrict);
      if (delim->Star ())
        _type = new CTypeVarArray (_type, quals); // variable length array
      else {
        if (_type->TypeArray () && ! _type->TypeArray ()->hasDimension () && ! _type->TypeVarArray ()) {
          if (node->token ())
            *_err << node->token ()->location ();
          *_err << sev_error << "multidimensional array must have bounds for all dimensions except the first"
                << endMessage;
        }
        _type = new CTypeArray (_type, quals, delim->Expr ());
        if (delim->Expr ()) {
          if (delim->Expr ()->Value ()) {
            if ((v = delim->Expr ()->Value ()->Constant ()))
              _type->TypeArray ()->Dimension (v->convert_to_uint ());
          } else if (delim->Expr ()->Type () && 
                     delim->Expr ()->Type ()->TypeTemplateParam ()) {
            _type->TypeArray ()->DepDim (
              delim->Expr ()->Type ()->TypeTemplateParam ());
          }
        }
      }
      node = ad->Declarator ();
    } else if (node->NodeName () == CT_FctDeclarator::NodeId ()) {
      CT_FctDeclarator *fd = (CT_FctDeclarator*)node;
      CT_ArgDeclList *adl = fd->Arguments ();
      int args = adl->Entries ();
      CTypeList *tl = new CTypeList (args ? args : 1);
      tl->ArgumentList (adl);
      bool prototype = (adl->NodeName () != CT_ArgNameList::NodeId ());
      if (prototype) {
        for (int i = 0; i < args; i++) {
          CT_ArgDecl *arg = (CT_ArgDecl*) adl->Entry (i);
          if (arg->Ellipsis ()) {
            tl->AddEntry (&CTYPE_ELLIPSIS);
            break;
          }
          // §8.3.5.3 adjust parameter type
          CTypeInfo *argtype = arg->Object ()->TypeInfo ();
          // any cv-qualifier modifying a parameter is deleted
          if (argtype->TypeQualified ())
            argtype = argtype->UnqualType ();
          // §8.3.5.2 the parameter list `(void)' is equivalent to
          // the empty parameter list
          if (argtype->is_void () && args == 1) 
            break;
          // type `function returning T' is adjusted to be 
          // `pointer to function returning T'
          if (argtype->TypeFunction ())
            argtype = new CTypePointer (argtype->Duplicate ());
          // type `array of T' is adjusted to be `pointer to T'
          else if (argtype->TypeArray ()) {
            CTypeInfo *quals = argtype->TypeArray ()->Qualifiers ();
            argtype = argtype->BaseType ();
            argtype = new CTypePointer (argtype->Duplicate ());
            if (quals)
              argtype = new CTypeQualified (argtype, quals->isConst (), 
                quals->isVolatile (), quals->isRestrict ());
          } else
            argtype = argtype->Duplicate ();
          tl->AddEntry (argtype);
        }
      }
      _type = new CTypeFunction (_type, tl, prototype);
      HandleQualifiers (fd->Qualifier (), true);
      node = fd->Declarator ();
    } else if (node->NodeName () == CT_SimpleName::NodeId () ||
               node->NodeName () == CT_TemplateName::NodeId () ||
               node->NodeName () == CT_PrivateName::NodeId () ||
               node->NodeName () == CT_DestructorName::NodeId () ||
               node->NodeName () == CT_ConversionName::NodeId () ||
               node->NodeName () == CT_OperatorName::NodeId () ||
               node->NodeName () == CT_QualName::NodeId () ||
               node->NodeName () == CT_RootQualName::NodeId ()) {
      _name = (CT_SimpleName*)node;
      break;
    } else {
      if (node->token ()) 
        *_err << node->token ()->location ();
      *_err << sev_fatal << "unknown declarator `" 
            << (const char*)node->NodeName () << "'" 
            << endMessage;
      break;
    }
  }
  if (_type && TRACE_TYPE) 
    TypeText (std::cout, _type, 0);
}

void CSemDeclarator::HandleQualifiers (CT_DeclSpecSeq *dss, bool is_fct) {
  if (dss) {
    CTypeQualified* qt = _type->TypeQualified ();
    int c_count = qt && qt->isConst () ? 1 : 0;
    int v_count = qt && qt->isVolatile () ? 1 : 0;
    int r_count = qt && qt->isRestrict () ? 1 : 0;
    for (int s = 0; s < dss->Sons (); s++) {
      CT_PrimDeclSpec *pds = (CT_PrimDeclSpec*)dss->Son (s);
      if (pds->SpecType () == CT_PrimDeclSpec::PDS_CONST) {
        c_count++;
        if (c_count > 1)
          *_err << sev_warning << pds->token ()->location ()
                << "duplicate `const'" << endMessage;
      } else if (pds->SpecType () == CT_PrimDeclSpec::PDS_VOLATILE) {
        v_count++;
        if (v_count > 1)
          *_err << sev_warning << pds->token ()->location ()
                << "duplicate `volatile'" << endMessage;
      } else if (pds->SpecType () == CT_PrimDeclSpec::PDS_RESTRICT) {
        r_count++;
        if (r_count > 1)
          *_err << sev_warning << pds->token ()->location ()
                << "duplicate `restrict'" << endMessage;
      }
    }
    if (c_count || v_count || r_count) {
      if (is_fct || qt) {
        ((CTypeQualified*)_type)->isConst ((c_count));
        ((CTypeQualified*)_type)->isVolatile ((v_count));
        ((CTypeQualified*)_type)->isRestrict ((r_count));
      } else {
        _type = new CTypeQualified (_type, (c_count), (v_count), (r_count));
      }
    }
  }
}


} // namespace Puma
