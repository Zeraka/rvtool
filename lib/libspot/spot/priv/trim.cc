// -*- coding: utf-8 -*-
// Copyright (C) 2015, 2018 Laboratoire de Recherche et Developpement
// de l'Epita (LRDE).
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"
#include <spot/priv/trim.hh>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace spot
{
  void
  trim(std::string& str)
  {
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           std::not1(std::ptr_fun<int, int>
                                     (std::isspace))).base(),
              str.end());
    str.erase(str.begin(),
              std::find_if(str.begin(), str.end(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))));
  }
}
