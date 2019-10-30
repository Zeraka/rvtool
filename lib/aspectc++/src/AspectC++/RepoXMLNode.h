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

#ifndef __RepoXMLNode_h__
#define __RepoXMLNode_h__

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <string>
#include <string.h>

using namespace std;

class RepoXMLNode {
  xmlNodePtr _node;
public:
  class Iter {
    xmlNodePtr _node;
  public:
    Iter (xmlNodePtr n) : _node (n) {}
    RepoXMLNode operator * () { return RepoXMLNode (_node); }
    void operator ++ () { _node = _node->next; }
    int operator == (const Iter &iter) const { return _node == iter._node; } 
    int operator != (const Iter &iter) const { return _node != iter._node; } 
  };
  typedef Iter iter;
    
  RepoXMLNode () : _node (0) {}
  RepoXMLNode (xmlNodePtr n) : _node (n) {}
  xmlNodePtr node () const { return _node; }
  int operator ! () const { return (_node == 0); }
  bool has_prop (const char* prop);
  void set_str_prop (const char* prop, const char* val);
  void set_int_prop (const char* prop, int val);
  string get_str_prop (const char* prop);
  int get_int_prop (const char* prop);
  RepoXMLNode make_child (const char *name);
  iter first_child () const { return Iter (_node->children); }
  iter end_child () const { return Iter ((xmlNodePtr)0); }
  bool has_name (const char *name) const { 
    return (strcmp (name, (const char*)_node->name) == 0); 
  }
};

#endif // __RepoXML_h__
