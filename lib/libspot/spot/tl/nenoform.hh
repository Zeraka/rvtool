// -*- coding: utf-8 -*-
// Copyright (C) 2011, 2012, 2013, 2015 Laboratoire de Recherche et
// Développement de l'Epita (LRDE).
// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
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

#pragma once

#include <spot/tl/formula.hh>

namespace spot
{
  /// \ingroup tl_rewriting
  /// \brief Build the negative normal form of \a f.
  ///
  /// All negations of the formula are pushed in front of the
  /// atomic propositions.
  ///
  /// \param f The formula to normalize.
  /// \param negated If \c true, return the negative normal form of
  ///        \c !f
  ///
  /// Note that this will not remove abbreviated operators.  If you
  /// want to remove abbreviations, call spot::unabbreviate
  /// first.  (Calling this function after
  /// spot::negative_normal_form would likely produce a formula
  /// which is not in negative normal form.)
  SPOT_API formula
  negative_normal_form(formula f, bool negated = false);
}
