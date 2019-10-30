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

#include "Puma/DeducedArgument.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CCSemExpr.h"

namespace Puma {


DeducedArgument::DeducedArgument (CTemplateParamInfo *p, CTypeInfo *t, CTree *a, bool da, bool dia) {
  del = false;
  type = t->Duplicate ();
  value = 0;
  param = p;
  arg = a;
  default_arg = da;
  direct_arg = dia;
}

DeducedArgument::DeducedArgument (CTemplateParamInfo *p, CConstant *v, CTree *a, bool da, bool dia) {
  del = false;
  type = 0;
  value = p->ValueType () ? v->cast_to (p->ValueType ()) : v->duplicate ();
  param = p;
  arg = a;
  default_arg = da;
  direct_arg = dia;
}

DeducedArgument::DeducedArgument (CTemplateParamInfo *p, long val) {
  del = false;
  type = 0;
  param = p;
  value = new CConstant ((LONG_LONG)val, &CTYPE_LONG);
  arg = 0;
  default_arg = false;
  direct_arg = false;
}

DeducedArgument::~DeducedArgument () {
  if (type)
    CTypeInfo::Destroy (type);
  if (value)
    delete value;
}

bool DeducedArgument::equals (const DeducedArgument &a, bool matchTemplateParams) {
  if ((((bool)Type ()) != ((bool)a.Type ())) ||
      (((bool)Value ()) != ((bool)a.Value ())))
    return false;
  if (Type () && ! Type ()->equals (*a.Type (), matchTemplateParams))
    return false;
  if (Value () && *Value () != *a.Value ())
    return false;
  return true;
}

void DeducedArgument::print(std::ostream &os) const {
  print (os, true);
}

void DeducedArgument::print(std::ostream &os, bool abs) const {
  if (Type()) {
    Type()->TypeText(os, 0, abs);
  }
  else if (Value()) {
    CTree* arg = TemplateArg();
    CTypeInfo* type = arg ? arg->Type() : 0;
    bool addr = arg ? CCSemExpr::isAddrExpr(arg) : false;
    if (addr || (type && (type->isArray() || type->isFunction()))) {
      // ��14.3.2 address as non-type template argument
      CObjectInfo* obj = CCSemExpr::findObject(arg);
      if (obj)
        os << (addr ? "&" : "") << obj->QualName(abs);
      else
        os << *Value();
    } else {
      os << *Value();
    }
  }
}


} // namespace Puma
