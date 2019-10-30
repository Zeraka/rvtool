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

#ifndef __include_expander_h__
#define __include_expander_h__

// class alias for frontend-specific include expander implementation

#ifdef FRONTEND_PUMA

#include "PumaIncludeExpander.h"
typedef PumaIncludeExpander IncludeExpander;

#else // FRONTEND_CLANG

#include "ClangIncludeExpander.h"
typedef ClangIncludeExpander IncludeExpander;

#endif

#endif // __include_expander_h__
