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

/* $Id: PointCut.h,v 1.7 2004/09/07 10:57:16 olaf.spinczyk Exp $ */

#ifndef __PointCut_h__
#define __PointCut_h__

#include <iostream>
using namespace std;

#include "Puma/List.h"
#include "JoinPoint.h"

class PointCut : private Puma::List
 {
   public:
//      enum operation { PC_NOT, PC_AND, PC_OR };
      enum pctype { PCT_CODE, PCT_CLASS, PCT_UNKNOWN };

   private:
      pctype _type;
//      pccontents _contents; // shall replace _type someday
      list<PointCut*> _cflow_triggers;

   public:
      PointCut();
      ~PointCut ();
      PointCut& operator = (const PointCut& pc);

      // set and get the pointcut type
      void type (pctype t) { _type = t; }
      pctype type () const { return _type; }

      // set and get for cflow trigger pointcut
      void cflow_triggers (PointCut& pc);
      const list<PointCut*> &cflow_triggers();

      friend ostream& operator<< (ostream&, PointCut&);

      class iterator {
        const Puma::ListElement *_curr;
        Puma::List *_list;
      public:
        iterator (const Puma::ListElement *e, Puma::List *l) : _curr (e), _list (l) {}
        void operator ++ () { if (_curr) _curr = _list->next (_curr); }
        int operator == (const iterator &iter) const {
          return _curr == iter._curr && _list == iter._list; 
        }
        int operator != (const iterator &iter) const {
          return !(*this == iter);
        }
        const JoinPoint &operator * () { return (const JoinPoint&)*_curr; }
      };

      iterator begin () { return iterator (Puma::List::first (), this); }
      iterator end () { return iterator (0, this); }
      iterator find (ACM_Any *jpl);

      void append (Puma::ListElement& element) { Puma::List::append (element); }
      void clear () { Puma::List::clear (); }
 };

#endif
