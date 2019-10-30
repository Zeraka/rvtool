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

#ifndef __RepoIdMgr_h__
#define __RepoIdMgr_h__

#include <stdio.h>
#include <sstream>

class RepoIdMgr {
  int _last;
  char _idstr[11];

public:
  RepoIdMgr () { reset_id (); }
  int new_id () { return _last++; }
  void reset_id () { _last = 0; }
  void update (int id ) {
    if (id >= _last)
      _last = id + 1;
  }
  const char *idstr (int id) {
    sprintf (_idstr, "%d", id);
    return _idstr;
  }
  static void get_id_set (const string &id_list_string, set<int> &ids) {
    std::stringstream ss(id_list_string);
    int i;

    while (ss >> i) {
      ids.insert (i);
      if (ss.peek() == ',')
        ss.ignore();
    }
  }
};

#endif // __RepoIdMgr_h__
