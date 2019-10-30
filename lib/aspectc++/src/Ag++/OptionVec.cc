// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2004  The 'ac++' developers (see aspectc.org)
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

#include "OptionVec.h"

//stdc++ includes
#include <iostream>

void
OptionVec::pushback(string name, string arg, OptionItem::flag_t flag)
{
  OptionItem newItem(name, arg, flag);
  this->push_back(newItem);
}

void
OptionVec::pushback(string name, OptionItem::flag_t flag)
{
  OptionItem newItem(name, flag);
  this->push_back(newItem);
}

const string&
OptionVec::getString()
{
  string* str = new string;
  for (vec_t::iterator optitem = this->begin(); optitem != this->end();
      ++optitem)
  {
    str->append(" " + optitem->name());
    str->append(optitem->arg());
  }
  return *str;
}

const string&
OptionVec::getString(OptionItem::flag_t flag)
{
  string* str = new string;
  for (vec_t::iterator optitem = this->begin(); optitem != this->end();
      ++optitem)
  {
    if ((flag & optitem->flag()) == flag)
    {
      str->append(" " + optitem->name());
      str->append(optitem->arg());
    }
  }
  return *str;
}

const string&
OptionVec::getString(OptionItem::flag_t flag, OptionItem::flag_t exclude_flag)
{
  string* str = new string;
  for (vec_t::iterator optitem = this->begin(); optitem != this->end();
      ++optitem)
  {
    if (((flag & optitem->flag()) == flag)
        && ((exclude_flag & optitem->flag()) == 0))
    {
      str->append(" " + optitem->name());
      str->append(optitem->arg());
    }
  }
  return *str;
}
