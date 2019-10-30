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

#include "RepoXMLNode.h"

bool RepoXMLNode::has_prop (const char* prop) {
  return xmlHasProp (_node, (xmlChar*)prop) != NULL;
}

void RepoXMLNode::set_str_prop (const char* prop, const char* val) {
  xmlSetProp (_node, (xmlChar*)prop, (xmlChar*)val);
}

void RepoXMLNode::set_int_prop (const char* prop, int val) {
  char buffer[12];
  sprintf (buffer, "%d", val);
  xmlSetProp (_node, (xmlChar*)prop, (xmlChar*)buffer);
}

string RepoXMLNode::get_str_prop (const char* prop) {
  char *str = (char*)xmlGetProp (_node, (xmlChar*)prop);
  string result (str);
  free (str);
  return result;
}

int RepoXMLNode::get_int_prop (const char* prop) {
  char *str = (char*)xmlGetProp (_node, (xmlChar*)prop);
  int val = atoi (str);
  free (str);
  return val;
}

RepoXMLNode RepoXMLNode::make_child (const char *name) {
  return xmlNewChild(_node, NULL, (xmlChar*)name, NULL);
}


