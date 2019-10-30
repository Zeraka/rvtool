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

#ifndef PUMA_version_H
#define PUMA_version_H

/** \defgroup basics Basics 
 * Basic abstractions and utility classes. */

/** \defgroup common Common 
 * Common abstractions for source projects and parsing. */

/** \defgroup cpp Preprocessor 
 * Preprocessor implementation classes. */

/** \defgroup infos Semantic Information 
 * Semantic information classes. */

/** \defgroup types Type Information
 * Type information classes. 
 * \ingroup infos */

/** \defgroup manip Transformation 
 * Source transformation classes. */

/** \defgroup parser Parsing 
 * Basic parsing classes. */

/** \defgroup cparser C Parsing 
 * C parsing classes. 
 * \ingroup parser */

/** \defgroup ccparser C++ Parsing 
 * C++ parsing classes. 
 * \ingroup parser */

/** \defgroup acparser AspectC++ Parsing 
 * AspectC++ parsing classes. 
 * \ingroup parser */

/** \defgroup instantiation Template Instantiation 
 * C++ template instantiation classes. 
 * \ingroup ccparser */

/** \defgroup scanner Scanning 
 * Lexical analysis classes. */

/** \file
 *  Library version information.
 * \ingroup basics */

/** Library major version number. */
#define PUMA_MAJOR_VERSION        1
/** Library minor version number. */
#define PUMA_MINOR_VERSION        5

/** \cond internal */
#define __PUMA_STR(arg) #arg
#define __PUMA_VERSION(major,minor) __PUMA_STR(major) "." __PUMA_STR(minor)
/** \endcond */

/** Library version string. */
#define PUMA_VERSION __PUMA_VERSION(PUMA_MAJOR_VERSION,PUMA_MINOR_VERSION)

namespace Puma {

/** Get the library version string in the format "major.minor". 
 * \return The version string. */
static inline const char *puma_version() { return PUMA_VERSION; }

} // namespace Puma

/** \mainpage PUMA Library Reference Manual
 *
 * This is the class documentation of the PUMA library. PUMA is an extensible 
 * C/C++ source code analysis and transformation library written in C++. 
 *
 * See [puma.aspectc.org](http://puma.aspectc.org) for more information.
 *  
 * The main classes of the library are the following.
 *
 * ### Source directory, error, and configuration handling
 *
 * - Puma::ErrorStream - Error output stream
 * - Puma::Config - Library configuration
 * - Puma::Project - Source directory operations
 *
 * ### Lexical analysis
 *
 * - Puma::Lexer - Base class for language specific source code tokenizers
 * - Puma::Token - Language token
 * - Puma::Unit - Tokenized source file or code snippet
 *
 * ### Preprocessing
 *
 * - Puma::PreprocessorParser - The C preprocessor parser
 * - Puma::PreTree - Base class for C preprocessor syntax tree nodes
 * 
 * ### Syntactic analysis
 *
 * - Puma::Syntax - Base class for language specific syntax analyses
 * - Puma::Builder - Base class for language specific syntax tree builders
 * - Puma::CTree - Base class for syntax tree nodes for C, C++ and derived languages
 * 
 * ### Semantic analysis
 *
 * - Puma::Semantic - Base class for language specific semantic analyses
 * - Puma::CSemDatabase - Semantic information database
 * 
 * ### Parsing (chaining of lexical, syntactic, and semantic analysis)
 *
 * - Puma::Parser - Base class for language specific parsers
 * - Puma::CTranslationUnit - Translation unit for C, C++ and derived languages
 *  
 * ### Transformation (source code manipulation)
 *
 * - Puma::CVisitor - Puma::CTree syntax tree visitor
 * - Puma::ManipCommander - Token chain manipulation
 */

#endif /* PUMA_version_H */
