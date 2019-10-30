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

#include <iostream>
#include <list>
#include "AdviceRepo.h"
#include "RepoXMLNode.h"

int AdviceRepo::insert (int file_id, int line, const string &type, int asp,
			int lines) {
  Map::iterator entry = _map.find (RepoPosAspectKey (file_id, line, asp));
  if (entry != _map.end ()) {
    Data &data = (*entry).second;
    data._ref = true;
    return data._id;
  }
  int id = new_id ();
  _map.insert (MapPair (RepoPosAspectKey (file_id, line, asp), 
			Data (id, type, lines)));
  return id;
}

void AdviceRepo::noref () {
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    data._ref = false;
  }
}

void AdviceRepo::cleanup (set<int> files) {
  std::list<RepoPosAspectKey> to_delete;
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    const RepoPosAspectKey &key = (*curr).first;
    if (!data._ref && files.count (key.file_id ()) == 1) {
      to_delete.push_back(key);
    }
  }
  list<RepoPosAspectKey>::iterator c;
  for (c = to_delete.begin(); c != to_delete.end(); ++c) {
    _map.erase(*c);
  }
}

void AdviceRepo::make_xml (RepoXMLNode parent) {

  RepoXMLNode aspects = parent.make_child ("advice-list");

  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    RepoXMLNode an = aspects.make_child ("advice");
    const RepoPosAspectKey &key = (*curr).first;
    key.make_xml (an);

    const Data &data = (*curr).second;
    an.set_int_prop ("id", data._id);
    an.set_str_prop ("type", data._type.c_str ());
    an.set_int_prop ("lines", data._lines);
  }
}

void AdviceRepo::get_xml (RepoXMLNode parent) {
  reset_id ();
  for (RepoXMLNode::iter curr = parent.first_child ();
       curr != parent.end_child (); ++curr) {
    RepoXMLNode an = *curr;
    if (an.has_name ("advice")) {
      int id      = an.get_int_prop ("id");
      string type = an.get_str_prop ("type");
      int lines   = an.get_int_prop ("lines");
      
      _map.insert (MapPair (RepoPosAspectKey (an),
			    Data (id, type.c_str (), lines)));
      update (id);
    }
  }    
}
