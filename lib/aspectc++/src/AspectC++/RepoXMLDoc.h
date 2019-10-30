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

#ifndef __RepoXMLDoc_h__
#define __RepoXMLDoc_h__

#include "RepoXMLNode.h"

#include <fcntl.h>
#include <unistd.h>

#include <libxml/xmlsave.h>

class RepoXMLDoc {
  xmlDocPtr _doc;
  int fd;
public:
  RepoXMLDoc () : _doc (0) {}
  ~RepoXMLDoc () {
    if (!_doc) {
      // delete the DOM
      xmlFreeDoc (_doc);
    }
  }
  void create (const char *root_name) {
    _doc = xmlNewDoc((const xmlChar*)"1.0");
    _doc->children = xmlNewDocNode(_doc, NULL,
				   (const xmlChar*)root_name, NULL);
  }
  RepoXMLNode root () const { return _doc->children; }
  bool load_fd (int fd, const char *name) {
    _doc = xmlReadFd (fd, name, 0,
                      XML_PARSE_NODICT | XML_PARSE_NOERROR |
                      XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS);
    return (_doc != 0);
  }
  bool load (const char *name) {
    _doc = xmlReadFile (name, 0,
                         XML_PARSE_NODICT | XML_PARSE_NOERROR |
                         XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS);
    return (_doc != 0);
  }
  bool save_fd (int fd) {
    xmlSaveCtxtPtr ctx = xmlSaveToFd (fd, 0, XML_SAVE_FORMAT);
    if (xmlSaveDoc (ctx, _doc) == -1)
      return false;
    return (xmlSaveClose (ctx) != -1);
  }
  bool save (const char *name) {
    return (xmlSaveFormatFile (name, _doc, 1) != -1);
  }
};

#endif // __RepoXMLDoc_h__
