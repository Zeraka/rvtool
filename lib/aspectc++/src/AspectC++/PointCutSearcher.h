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

#ifndef __point_cut_searcher_h__
#define __point_cut_searcher_h__

#include <vector>
#include <string>
class ACM_Pointcut;
class ACM_Attribute;

class PointCutSearcher {
public:
  virtual ACM_Pointcut *lookup_pct_func (bool root_qualified, std::vector<std::string> &qual_name) = 0;
  virtual ACM_Attribute *lookup_pct_attr (bool root_qualified, std::vector<std::string> &qual_name) = 0;
//  virtual bool lookup_context_var(const std::string &name) = 0;
  virtual ~PointCutSearcher() {}
};

#endif // __point_cut_searcher_h__
