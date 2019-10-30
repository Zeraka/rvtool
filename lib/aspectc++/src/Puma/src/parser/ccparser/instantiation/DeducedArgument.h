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

#ifndef __DeducedArgument__
#define __DeducedArgument__

#include "Puma/CTypeInfo.h"
#include "Puma/CConstant.h"
#include "Puma/Printable.h"
#include <ostream>

namespace Puma {


class CTemplateParamInfo;
class CTree;

struct DeducedArgument : public Printable {
  CTypeInfo *type;
  CConstant *value;
  CTemplateParamInfo *param;
  CTree *arg;
  bool del;
  bool default_arg;
  bool direct_arg;

public:
  DeducedArgument (CTemplateParamInfo *, CTypeInfo *, CTree * = 0, bool = false, bool = false);
  DeducedArgument (CTemplateParamInfo *, CConstant *, CTree * = 0, bool = false, bool = false);
  DeducedArgument (CTemplateParamInfo *, long);
  ~DeducedArgument ();

  bool operator ==(const DeducedArgument &);
  bool operator !=(const DeducedArgument &);
  bool equals (const DeducedArgument &, bool matchTemplateParams = false);

  CTypeInfo *Type () const;
  CConstant *Value () const;
  CTemplateParamInfo *TemplateParam () const;
  CTree *TemplateArg () const;
  bool isDefaultArg () const;
  bool isDirectArg () const;

  /** Print the deduced argument, i.e. the type or constant value.
   *  \param os The output stream. */
  virtual void print (std::ostream &os) const;

  /** Print the deduced argument, i.e. the type or constant value.
   *  \param os The output stream.
   *  \param abs If 'true' names are printed with absolute names (leading '::')*/
  void print (std::ostream &os, bool abs) const;
};

inline CTypeInfo *DeducedArgument::Type () const
 { return type; }
inline CConstant *DeducedArgument::Value () const
 { return value; }
inline CTemplateParamInfo *DeducedArgument::TemplateParam () const
 { return param; }
inline CTree *DeducedArgument::TemplateArg () const
 { return arg; }
inline bool DeducedArgument::isDefaultArg () const
 { return default_arg; }
inline bool DeducedArgument::isDirectArg () const
 { return direct_arg; }
inline bool DeducedArgument::operator ==(const DeducedArgument &d)
 { return equals (d); }
inline bool DeducedArgument::operator !=(const DeducedArgument &d)
 { return ! equals (d); }


} // namespace Puma

#endif /* __DeducedArgument__ */
