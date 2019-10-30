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

#ifndef __RepoPosAspectKey_h__
#define __RepoPosAspectKey_h__

#include "RepoPosKey.h"

class RepoPosAspectKey : public RepoPosKey {
  int _aspect_id;
public:
  RepoPosAspectKey (int file_id, int line, int aspect_id) :
    RepoPosKey (file_id, line), _aspect_id (aspect_id) {}
  RepoPosAspectKey (RepoXMLNode node) : RepoPosKey (node) { get_xml (node); }
  int operator < (const RepoPosAspectKey& key) const {
    return _aspect_id == key._aspect_id ?
      (*(const RepoPosKey *)this < key) : (_aspect_id < key._aspect_id);
  }
  int aspect_id () const { return _aspect_id; }
  void get_xml (RepoXMLNode node);
  void make_xml (RepoXMLNode node) const;
};

#endif // __RepoPosAspectKey_h__
