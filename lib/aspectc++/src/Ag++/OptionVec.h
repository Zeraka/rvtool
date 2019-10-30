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

#ifndef __OptionVec_h__
#define __OptionVec_h__

//stdc++ includes
#include <string>
#include <vector>
using namespace std;

class OptionItem
{
public:
  enum
  {
    OPT_ACC = 0x1,
    OPT_GCC = 0x2,
    OPT_FILE = 0x4,
    OPT_CONFIG = 0x8,
    OPT_DEP = 0x10
  };
  typedef unsigned int flag_t;

private:
  string _name;
  string _argument;
  flag_t _flag;

public:
  OptionItem(string name, string arg, flag_t flag) :
      _name(name), _argument(arg), _flag(flag)
  {
  }
  OptionItem(string name, flag_t flag) :
      _name(name), _flag(flag)
  {
  }
  OptionItem(const OptionItem& opt) :
      _name(opt._name), _argument(opt._argument), _flag(opt._flag)
  {
  }

  const string&
  name()
  {
    return _name;
  }
  void
  name(string newname)
  {
    _name = newname;
  }

  const string&
  arg()
  {
    return _argument;
  }
  void
  arg(string newarg)
  {
    _argument = newarg;
  }

  const flag_t
  flag()
  {
    return _flag;
  }
  void
  flag(flag_t flag)
  {
    _flag = flag;
  }
};

class OptionVec : public vector<OptionItem>
{

public:
  typedef vector<OptionItem> vec_t;

private:
  //vec_t _options;

public:
  //OptionVec(){}
  //OptionVec(const OptionVec& optvec):_options(optvec._options){}

  // Add a new Option
  void
  pushback(string name, string arg, OptionItem::flag_t flag);

  // Add a new Option
  void
  pushback(string name, OptionItem::flag_t flag);

  const string&
  getString();

  // get a string of options which have at least the flags passed with parameter
  const string&
  getString(OptionItem::flag_t);

  // get a string of options which have at least the flags passed with parameter
  // and not the flags pass as second argument
  const string&
  getString(OptionItem::flag_t, OptionItem::flag_t);
};

#endif //__OptionVec_h__
