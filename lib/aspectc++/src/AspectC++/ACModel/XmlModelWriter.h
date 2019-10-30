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

#ifndef __XmlModelWriter_h__
#define __XmlModelWriter_h__

#include "Elements.h"
#include <libxml/xmlsave.h>

class XmlModelWriter {
public:
  // save project model as an XML file
  bool write (ProjectModel &model, const char *filename, int fd = -1) {
    SaveContext context;
    xmlDocPtr doc = model.xml_doc (); 
    if (!doc) {
      doc = xmlNewDoc((const xmlChar*)"1.0");
      doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"ac-model", NULL);
      model.xml_doc (doc);
      model.set_xml_node (doc->children);
    }
    model.to_xml (context);
    model.set_xml_dirty (false);
    model.set_property ("ids", context.next_id ()); // also save the number of used IDs
    if (fd >= 0) {
      xmlSaveCtxtPtr ctx = xmlSaveToFd (fd, 0, XML_SAVE_FORMAT);
      if (xmlSaveDoc (ctx, doc) == -1)
        return false;
      return (xmlSaveClose (ctx) != -1);
    }
    else {
      return (xmlSaveFormatFile (filename, doc, 1) != -1);
    }
  }
};

#endif // __XmlModelWriter_h__
