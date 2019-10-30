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

#ifndef __AspectIncludeCluster_h__
#define __AspectIncludeCluster_h__

#include <set>

#include "ACFileID.h"

// This is a simple container for storing a reference to an aspect header file
// and the set of aspect headers that are also needed for the respective aspect
// header, because they affect files that are included by it.

struct AspectIncludeCluster {
  ACFileID _aspect;
  std::set<ACFileID> _cluster; // including '_aspect'
  AspectIncludeCluster(ACFileID aspect_fid) : _aspect(aspect_fid) {}
  void set_cluster(const std::set<ACFileID> &cluster) { _cluster = cluster; }
};

#endif // __AspectIncludeCluster_h__
