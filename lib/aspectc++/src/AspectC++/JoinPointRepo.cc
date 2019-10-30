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
#include "JoinPointRepo.h"
#include "RepoXMLNode.h"
#include "Puma/StrCol.h"
// Reinhard
// #include "MatchExpr.h"
// End Reinhard
using namespace Puma;

int JoinPointRepo::insert (int file_id, int line, const string &signature, 
			   const string &type, int adv, int lines) {
  Map::iterator entry = _map.find (RepoPosKey (file_id, line));
  if (entry != _map.end ()) {
    Data &data = (*entry).second;
    if (!data._ref) {
      data._ref = true;
      data._advice.clear ();
    }
    data._advice.insert (adv);
    return data._id;
  }

  set<int> advice_set;
  advice_set.insert (adv);
  int id = new_id ();
  _map.insert (MapPair (RepoPosKey (file_id, line),
			Data (id, signature, type, advice_set, lines)));
  return id;
}

void JoinPointRepo::noref () {
  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    Data &data = (*curr).second;
    data._ref = false;
  }
}

void JoinPointRepo::cleanup (set<int> files) {
  std::list<RepoPosKey> to_delete;
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

void JoinPointRepo::make_xml (RepoXMLNode parent) {

  RepoXMLNode joinpoints = parent.make_child ("joinpoint-list");

  for (Map::iterator curr = _map.begin (); curr != _map.end (); ++curr) {
    RepoXMLNode jpn = joinpoints.make_child ("joinpoint");

    const RepoPosKey &key   = (*curr).first;
    key.make_xml (jpn);

    const Data &data = (*curr).second;
    string adv;
    for (set<int>::const_iterator curr = data._advice.begin ();
	 curr != data._advice.end (); ++curr) {
      if (curr != data._advice.begin ())
	adv += ",";
      adv += idstr (*curr);
    }

    jpn.set_int_prop ("id", data._id);
    jpn.set_str_prop ("type", data._type.c_str ());
    jpn.set_str_prop ("signature", data._signature.c_str ());
    jpn.set_str_prop ("advice", adv.c_str ());
    jpn.set_int_prop ("lines", data._lines);
  }
}

void JoinPointRepo::get_xml (RepoXMLNode parent) {
  reset_id ();
  for (RepoXMLNode::iter curr = parent.first_child ();
       curr != parent.end_child (); ++curr) {
    RepoXMLNode jpn = *curr;
    if (jpn.has_name ("joinpoint")) {
      int id           = jpn.get_int_prop ("id");
      string signature = jpn.get_str_prop ("signature");
      string type      = jpn.get_str_prop ("type");
      string adv_str   = jpn.get_str_prop ("advice");
      set<int> adv;
      get_id_set (adv_str, adv);
      int lines = jpn.get_int_prop ("lines");

      _map.insert (MapPair (RepoPosKey (jpn),
			    Data (id, signature.c_str (), type.c_str (),
				  adv, lines)));
      update (id);
      
// Reinhard
//      if (type == "exec") {
//        ErrorStream err; // beides kann erstmal ignoriert werden
//        Location loc;
//        MatchExpr expr (err, loc, "% ...::member%(...)");
//        
//        cout << "SIGNATURE: " << signature.c_str () << endl;
//        MatchSignature sig (err, loc, signature.c_str ());
//        cout << "- parsed" << endl;
//        if (expr.error ())
//          cout << "error in match" << endl;
//        else if (sig.error ())
//          cout << "error in sig" << endl;
//        else if (expr.matches(sig))
//          cout << "MATCHED signature " << signature.c_str () << " -> ID: " << id << endl;
//      }
// End Reinhard
    }
  }    
}
