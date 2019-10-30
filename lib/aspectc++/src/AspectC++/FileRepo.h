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

#ifndef __FileRepo_h__
#define __FileRepo_h__

#include <libxml/tree.h>

#include <map>
#include <set>
#include <string>
using namespace std;

#include "RepoNameKey.h"
#include "RepoIdMgr.h"
#include "RepoXMLNode.h"

class FileRepo : public RepoIdMgr {

  struct Data {
    int _id;
    int _lines;
    set<int> _tunits;
    bool _ref;
    Data (int id, int lines, set<int> &tunits) : _id (id), _lines (lines),
    _tunits (tunits), _ref (true) {}
    void bind_tunit (int u) { _tunits.insert (u); }
  };
  typedef map<RepoNameKey, Data> Map;
  typedef Map::value_type MapPair;
  Map _map;

public:

  int insert (const string &name, int lines, int tunit);

  void noref ();

  void dependent (int primary, set<int> &deps);

  void cleanup (int primary);

  void make_xml (RepoXMLNode parent);

  void get_xml (RepoXMLNode parent);
};

#endif // __FileRepo_h__
