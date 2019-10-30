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

#include "Puma/CCConversion.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CConstant.h"
#include "Puma/CRecord.h"
#include "Puma/CTree.h"

namespace Puma {


CCConversion::CCConversion (ConvId id, CTypeInfo *p, CTypeInfo *a, CTree *e, CTree *b) {
  _Id = id;
  _ToType = p->Duplicate ();
  _FromType = a->Duplicate ();
  _Expr = 0;
  _Base = b;
  
  if (b) 
    for (unsigned i = b->Sons (); i > 0; i--)
      if (b->Son (i-1) == e) {
        _Expr = e;
        _Base = b;
        break;
      }
}

CCConversion::~CCConversion () {
  if (_ToType)
    CTypeInfo::Destroy (_ToType);
  if (_FromType)
    CTypeInfo::Destroy (_FromType);
}
  
void CCConversion::Update (CTree *e) { 
  if (_Base && _Expr != e) 
    for (unsigned i = _Base->Sons (); i > 0; i--)
      if (_Base->Son (i-1) == e) {
        _Expr = e;
        if (e->Type ()) {
          CTypeInfo::Destroy (_FromType);
          _FromType = _Expr->Type ()->Duplicate ();
        }
        break;
      }
}

CFunctionInfo *CCConversion::ConvFunction () const {
  return isUserDefinedConv () ? ((CCUserDefinedConv*)this)->ConvFunction () : 0;
}


// apply conversion


// standard implementation
void CCConversion::Apply () {
  CT_ImplicitCast *cast;
  CExprValue *value;

  // apply conversion only if types differ
  if (_Expr && _Base && _FromType && _ToType && *_FromType != *_ToType) {
    cast = new CT_ImplicitCast (_Expr);
    cast->setType (_ToType->Duplicate ());
    _Base->ReplaceSon (_Expr, cast);

    // cast constant value if any
    value = _Expr->Value ();
    if (value) {
      if (value->Constant ()) {
        value = value->Constant ()->cast_to (_ToType);
        cast->setValue (value);
      } else
        cast->setValueRef (value);
    }

    _Expr = cast;
  }
}

void CCUserDefinedConv::Apply () {
  CT_ImplicitCall *call;
  CTypeInfo *type;
  
  if (! _ConvFunction)
    CCConversion::Apply ();
  else if (_Expr && _Base && _ToType) {
    if (_ConvFunction->isConversion ())
      type = _ConvFunction->ConversionType ();
    else if (_ConvFunction->isConstructor () && _ConvFunction->Record ())
      type = _ConvFunction->Record ()->TypeInfo ();
    else if (_ConvFunction->ReturnType () && 
             ! _ConvFunction->ReturnType ()->is_undefined ())
      type = _ConvFunction->ReturnType ();
    else
      type = _ToType;  
      
    if (type) {
      call = new CT_ImplicitCall (_Expr);
      call->Object (_ConvFunction);
      if (type == _ToType)
        call->setType (type->Duplicate ());
      else
        call->setTypeRef (type);
      _Base->ReplaceSon (_Expr, call);
      _Expr = call;
    }
  }
}


} // namespace Puma
