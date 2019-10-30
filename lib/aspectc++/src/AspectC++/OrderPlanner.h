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

#ifndef __OrderPlanner_h__
#define __OrderPlanner_h__

#include <set>
#include <map>
#include <vector>

using namespace std;

template <typename Item, typename Compare = less<Item*> >
class OrderPlanner {

  struct Links;

  typedef map<Item*,Links,Compare> NodeMap;
  typedef typename NodeMap::value_type Node;

  struct Links {
    set<Node*> _in;
    set<Node*> _out;
  };

  NodeMap _node_map;
  vector<Item*> _total_order;

public:
  // consider an order relation of two nodes
  void precedence (Item &first, Item &later);

  // calculate the total order => false if there is a cycle
  bool plan ();

  // get the result
  int items () const { return _total_order.size (); }
  Item &item (int i) const { return *_total_order[i]; }

};


template <typename Item, typename Compare>
bool OrderPlanner<Item, Compare>::plan () {

  // loop until all node have been moved to the total order vector
  while (_node_map.size () > 0) {

    // loop over all remaining nodes and count number of shifted nodes
    int moved = 0;
    typename NodeMap::iterator curr = _node_map.begin ();
    while (curr != _node_map.end ()) {
      // shift the current node if it has no incoming edge
      // (no left node has a higher precedence)
      if ((*curr).second._in.size () == 0) {
        moved++;

        // add this node to the total order
        _total_order.push_back ((*curr).first);

        // remove the edges of this node to other nodes with lower precedence
        for (typename set<Node*>::iterator edge = (*curr).second._out.begin ();
            edge != (*curr).second._out.end (); ++edge)
          (*edge)->second._in.erase (&*curr);

        // remove the node
        typename NodeMap::iterator rem = curr;
        ++curr;
        _node_map.erase (rem);
      }
      else {
        ++curr;
      }
    }
    // if no node was moved there must be a cycle in the graph!
    if (moved == 0)
      return false;
  }
  return true;
}

template <typename Item, typename Compare>
void OrderPlanner<Item, Compare>::precedence (Item &first, Item &later) {

  // find/insert the 'first' node
  typename NodeMap::iterator f = _node_map.find (&first);
  if (f == _node_map.end ()) {
    _node_map.insert (Node (&first, Links ()));
    f = _node_map.find (&first);
  }

  // find/insert the 'later' node
  typename NodeMap::iterator l = _node_map.find (&later);
  if (l == _node_map.end ()) {
    _node_map.insert (Node (&later, Links ()));
    l = _node_map.find (&later);
  }

  // insert the incoming and outgoing edges
  (*f).second._out.insert (&*l);
  (*l).second._in.insert (&*f);
}

#endif // __OrderPlanner_h__
