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
#include "FileRepo.h"
#include "RepoXMLNode.h"
#include "Puma/StrCol.h"
using namespace Puma;

int FileRepo::insert (const string &name, int lines, int tunit) {
  Map::iterator entry = _map.find (RepoNameKey (name));
  if (entry != _map.end ()) {
    Data &data = (*entry).second;
    if (tunit == -1)
      tunit = data._id;
    data.bind_tunit (tunit);
    data._ref = true;
    return data._id;
  }

  set<int> tunits;
  int id = new_id ();
  // tunit == -1 means that the primary unit itself is inserted, which has no
  // id yet. Therefore, we use the new id.
  tunits.insert (tunit == -1 ? id : tunit);
    
  _map.insert (MapPair (RepoNameKey (name), Data (id, lines, tunits)));

  return id;
}

void FileRepo::noref () {
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    data._ref = false;
  }
}

void FileRepo::cleanup (int primary) {
  std::list<RepoNameKey> to_delete;
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    if (!data._ref) {
      data._tunits.erase (primary);
      if (data._tunits.size () == 0) {
	to_delete.push_back((*curr).first);
      }
    }
  }
  list<RepoNameKey>::iterator c;
  for (c = to_delete.begin(); c != to_delete.end(); ++c) {
    _map.erase(*c);
  }
}

void FileRepo::dependent (int primary, set<int> &deps) {
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    if (data._tunits.count (primary) == 1)
      deps.insert (data._id);
  }
}

void FileRepo::make_xml (RepoXMLNode parent) {
  RepoXMLNode files = parent.make_child ("file-list");
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    RepoXMLNode fn = files.make_child ("file");
    const RepoNameKey &key   = (*curr).first;
    key.make_xml (fn);

    const Data &data = (*curr).second;
    string tunits;
    for (set<int>::const_iterator curr = data._tunits.begin ();
         curr != data._tunits.end (); ++curr) {
      if (curr != data._tunits.begin ())
        tunits += ",";
      tunits += idstr (*curr);
    }

    fn.set_int_prop ("id", data._id);
    fn.set_str_prop ("tunits", tunits.c_str ());
    fn.set_int_prop ("lines", data._lines);
  }
}

void FileRepo::get_xml (RepoXMLNode parent) {
  reset_id ();
  for (RepoXMLNode::iter curr = parent.first_child ();
       curr != parent.end_child (); ++curr) {
    RepoXMLNode fn = *curr;
    if (fn.has_name ("file")) {
      int id    = fn.get_int_prop ("id");
      int lines = fn.get_int_prop ("lines");
      string tunit_str = fn.get_str_prop ("tunits");
      set<int> tunits;
      get_id_set (tunit_str, tunits);

      _map.insert (MapPair (RepoNameKey (fn), Data (id, lines, tunits)));
      update (id);
    }
  }    
}
