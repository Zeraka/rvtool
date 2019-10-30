// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#ifndef __LineDirectiveMgr_h__
#define __LineDirectiveMgr_h__

#include <iostream>
using namespace std;

#include "ACErrorStream.h"

#ifdef FRONTEND_PUMA
namespace Puma {
  class Unit;
} // namespace Puma
#else
namespace clang {
  class PresumedLoc;
}
#endif

#include "ACConfig.h"
#include "ACToken.h"

class LineDirectiveMgr {
  ACErrorStream &_err;
  ACConfig &_config;
  
public:
  LineDirectiveMgr (ACErrorStream &e, ACConfig &config) :
    _err (e), _config (config) {}
#ifdef FRONTEND_PUMA
  bool insert (Puma::Unit *unit, PumaToken token, bool after = false);
  bool directive (ostream &out, Puma::Unit *unit = 0, PumaToken token = PumaToken ());
#else
  bool directive (ostream &out, const clang::PresumedLoc &loc);
#endif
  bool noline() const { return _config.noline (); }
};

#endif // __LineDirectiveMgr_h__
