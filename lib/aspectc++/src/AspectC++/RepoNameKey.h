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

#ifndef __RepoNameKey_h__
#define __RepoNameKey_h__

#include <string>
using namespace std;

#include <libxml/tree.h>
#include "RepoXMLNode.h"

class RepoNameKey {
  string _name;
public:
  RepoNameKey (const string &name) : _name (name) {}
  RepoNameKey (RepoXMLNode node) { get_xml (node); }
  int operator < (const RepoNameKey& key) const { return _name < key._name; }
  void get_xml (RepoXMLNode node) {
    _name = node.get_str_prop ("name"); 
  }
  void make_xml (RepoXMLNode node) const {
    node.set_str_prop ("name", _name.c_str ());
  }
};

#endif // __RepoNameKey_h__
