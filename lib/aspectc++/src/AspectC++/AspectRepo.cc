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

#include <list>
#include "AspectRepo.h"
#include "RepoXMLNode.h"

int AspectRepo::insert (int file_id, int line, const string &name) {
  Map::iterator entry = _map.find (RepoPosKey (file_id, line));
  if (entry != _map.end ()) {
    Data &data = (*entry).second;
    data._ref = true;
    return data._id;
  }

  int id = new_id ();
  _map.insert (MapPair (RepoPosKey (file_id, line), Data (id, name)));
  return id;
}

void AspectRepo::noref () {
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    data._ref = false;
  }
}

void AspectRepo::cleanup (set<int> files) {
  list<RepoPosKey> to_delete;
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    const RepoPosKey &key = (*curr).first;
    if (!data._ref && files.count (key.file_id ()) == 1) {
      to_delete.push_back(key);
    }
  }
  list<RepoPosKey>::iterator c;
  for (c = to_delete.begin(); c != to_delete.end(); ++c) {
    _map.erase(*c);
  }
}

void AspectRepo::make_xml (RepoXMLNode parent) {

  RepoXMLNode aspects = parent.make_child ("aspect-list");

  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    RepoXMLNode an = aspects.make_child ("aspect");
    const RepoPosKey &key   = (*curr).first;
    key.make_xml (an);

    const Data &data = (*curr).second;
    an.set_int_prop ("id", data._id);
    an.set_str_prop ("name", data._name.c_str ());
  }
}

void AspectRepo::get_xml (RepoXMLNode parent) {
  reset_id ();
  for (RepoXMLNode::iter curr = parent.first_child ();
       curr != parent.end_child (); ++curr) {
    RepoXMLNode an = *curr;
    if (an.has_name ("aspect")) {
      int id      = an.get_int_prop ("id");
      string name = an.get_str_prop ("name");

      _map.insert (MapPair (RepoPosKey (an), Data (id, name.c_str ())));
      update (id);
    }
  }    
}
