// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#include "Puma/InstantiationTokenSource.h"
#include "Puma/Token.h"
#include "Puma/CTree.h"
#include "Puma/DelayedParseTree.h"

namespace Puma {


InstantiationTokenSource::InstantiationTokenSource(CTree* root) {
  _root = root;
  _current = 0;
  _first = true;
  _detected_delayed_parse_node = false;
}


CTree *InstantiationTokenSource::findToken(CTree* node) {
  if (! node)
    return 0;
  const char *node_name = node->NodeName();
  if (node_name == CT_Token::NodeId())
    return node;
  else if (node_name == CT_DelayedParseParamInit::NodeId ()) {
    _detected_delayed_parse_node = true;
    return 0;
  }

  CTree* token = 0;
  unsigned sons = node->Sons();
  for (unsigned i = 0; i < sons && ! token; i++)
    token = findToken(node->Son(i));
  return token;
}


Token *InstantiationTokenSource::next() {
  if (_first) {
    _first = false;
    _current = _root ? _root->token_node() : 0;
  }
  else {
    CTree *node = _current, *last = _current, *son;
    int sons, idx;

    _current = 0;

    while (node) {
      sons = node->Sons();
      if (sons > 1) {
        idx = 0;
        if (node->IsList()) {
          idx = ((CT_List*)node)->Index(last) + 1;
        } else {
          for (int i = 0; i < sons; i++) {
            son = node->Son(i);
            if (son == last && (i+1) < sons) {
              idx = i + 1;
              break;
            }
          }
        }
        for (int i = idx; i > 0 && i < sons && !_current; i++)
          _current = findToken(node->Son(i));
      }
      if (_current || node == _root)
        break;
      last = node;
      node = node->Parent();
    }
  }
  return (_current ? _current->token () : 0);
}

} // namespace Puma
