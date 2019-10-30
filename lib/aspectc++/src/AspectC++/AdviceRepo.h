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

#ifndef __AdviceRepo_h__
#define __AdviceRepo_h__

#include <map>
#include <string>
#include <set>
using namespace std;

#include <libxml/tree.h>
#include "RepoPosAspectKey.h"
#include "RepoIdMgr.h"
#include "RepoXMLNode.h"

class AdviceRepo : public RepoIdMgr {

  struct Data {
    int _id;
    string _type;
    int _lines;
    bool _ref;
    Data (int id, const string &type, int lines) :
      _id (id), _type (type), _lines (lines), _ref (true) {}
  };
  typedef map<RepoPosAspectKey, Data> Map;
  typedef Map::value_type MapPair;
  Map _map;

public:

  int insert (int file_id, int line, const string &type, int asp, int lines);

  void noref ();

  void cleanup (set<int> files);

  void make_xml (RepoXMLNode parent);

  void get_xml (RepoXMLNode parent);
};

#endif // __AdviceRepo_h__
