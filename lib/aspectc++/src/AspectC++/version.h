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

#ifndef __version_h__
#define __version_h__

#include "infos.h"

static inline const char *ac_version() {return AC_VERSION;}
static inline const char *ac_program() {return "ac++";}

// Version management helpers:
// Returns a version number that can be compared by using >, >=, ==, ...
#define GET_VERSION_NUMBER(MAJOR, MINOR, PATCHLEVEL) \
                            MAJOR * 10000 \
                            + MINOR * 100 \
                            + PATCHLEVEL

#ifdef FRONTEND_CLANG
// Clang version:
#include "clang/Basic/Version.h"
#define CLANG_VERSION_NUMBER GET_VERSION_NUMBER(CLANG_VERSION_MAJOR, \
                                                CLANG_VERSION_MINOR, \
                                                CLANG_VERSION_PATCHLEVEL)

// Some version numbers:
#define VERSION_NUMBER_3_4   GET_VERSION_NUMBER(3, 4, 0)
#define VERSION_NUMBER_3_4_2 GET_VERSION_NUMBER(3, 4, 2)
#define VERSION_NUMBER_3_6_2 GET_VERSION_NUMBER(3, 6, 2)
#define VERSION_NUMBER_3_7_1 GET_VERSION_NUMBER(3, 7, 1)
#define VERSION_NUMBER_3_8_0 GET_VERSION_NUMBER(3, 8, 0)
#define VERSION_NUMBER_3_9_1 GET_VERSION_NUMBER(3, 9, 1)
#endif // FRONTEND_CLANG

#endif // __version_h__
