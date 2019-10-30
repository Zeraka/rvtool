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

#ifndef __C_COMMENT_TOKENS_H__
#define __C_COMMENT_TOKENS_H__

/** \file
 *  White space and comment token types. */

namespace Puma {


/** White space and comment token types. These types
 *  can be compared to Puma::Token::type(). */
enum {
  /** Any white space block. */
  TOK_WSPACE = 400,
  /** C++ style single line comment. */
  TOK_CCSINGLE = 350,
  /** C style multi-line comment start token. */
  TOK_CCMULTIBEGIN,
  /** C style multi-line comment end token. */
  TOK_CCMULTIEND,
  /** Comment block. */
  TOK_CCOMMENT
};


} // namespace Puma

#endif /* __C_COMMENT_TOKENS_H__ */
