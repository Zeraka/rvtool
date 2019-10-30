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

#include "Puma/PreprocessorParser.h"
#include "Puma/PreFileIncluder.h"
#include "Puma/PreParserState.h"
#include "Puma/UnitManager.h"
#include "Puma/TokenStream.h"
#include "Puma/PipedCmd.h"
#include "Puma/ImportHandler.h"
#include <sstream>
#include <string.h>

namespace Puma {


ImportHandler::ImportHandler (PreprocessorParser &cpp, const char *args) : 
 parser (cpp), header (0) {
  const char *p;
  unsigned len, l;
  
  // can't do an import without import handler
  if (! parser.importHandler () || ! args) {
    *parser.parserState ()->err << sev_error 
      << "using #import without import handler given" << endMessage;
    return;
  }
  
  // construct a handler call of the form:
  // <handler-cmd> [-I"<path>"]* "#import <args>" "<imported-file>"
  
  std::ostringstream cmd, name;
  
  // <handler-cmd>
  cmd << parser.importHandler () << " ";
  
  // [-I"<path>"]*
  const std::list<std::string> &paths = parser.fileIncluder ()->getIncludePaths ();
  std::list<std::string>::const_iterator pathIdx = paths.begin();
  for (; pathIdx != paths.end(); ++pathIdx)
    cmd << "-I\"" << *pathIdx << "\" ";

  p = args;
  len = strlen (p);

  // "#import args"
  cmd << "\"#import ";
  // copy arguments
  for (l = len; l > 0; l--) {
    if (*p == '\"')
      cmd << "\\";
    cmd << *p;
    p++;
  }
  cmd << "\" ";
  p = args;
  
  // "<imported-file>"
  name << "%";
  cmd << "\"";
  // skip white spaces
  while (isspace (*p)) 
    p++;
  // skip leading '<' resp. '"'
  if (*p == '<' || *p == '\"') 
    p++;
  // copy imported file name
  for (l = len; l > 0; l--) {
    if (*p == '>' || *p == '\"')
      break;
    cmd << *p;
    name << *p;
    p++;
  }
  // command line finished
  cmd << "\" ";
  
  callHandler (cmd.str ().c_str (), name.str ().c_str ());
}


Unit *ImportHandler::IncludeFile () const { 
  return header; 
}


void ImportHandler::callHandler (const char *cmd, const char *name) {
  // already imported
  header = parser.unitManager ()->get (name);
  if (header)
    return;

  // import header
  PipedCmd pcmd;
  if (pcmd.exec (cmd) != 0) {
    *parser.parserState ()->err << sev_error 
      << "#import failed, (using this information:" << endMessage;
    *parser.parserState ()->err << cmd << ")" << endMessage;
    return;
  }
    
  // scan generated header file
  header = new Unit;
  header->name (name);
#ifdef VISUAL_STUDIO  
  header->state ().onlyOnce (true);
#endif /* VISUAL_STUDIO */
  parser.cscanner ().fill_unit (pcmd.getOutput ().c_str (), *header);
    
  // push header on token stack (like an ordinary include file)
  parser.unitManager ()->add (header);
  ((TokenStream*)(parser.scanner ()))->push (header);
}


} // namespace Puma
