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

#ifndef __RepoPosKey_h__
#define __RepoPosKey_h__

#include <libxml/tree.h>
#include "RepoXMLNode.h"

class RepoPosKey {
  int _file_id;
  int _line;
public:
  RepoPosKey (int file_id, int line) : _file_id (file_id), _line (line) {}
  RepoPosKey (RepoXMLNode node) { get_xml (node); }
  int operator < (const RepoPosKey& key) const {
    return _file_id == key._file_id ?
      (_line < key._line) : (_file_id < key._file_id);
  }
  int file_id () const { return _file_id; }
  int line () const { return _line; }
  void get_xml (RepoXMLNode node);
  void make_xml (RepoXMLNode node) const;
};

#endif // __RepoPosKey_h__
