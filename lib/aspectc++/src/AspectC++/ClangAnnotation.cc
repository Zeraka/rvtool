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

#include "ClangAnnotation.h"

std::string Annotation::get_name() const {
  return attrNames[attrNames.size() - 1];
}

std::string Annotation::get_qualified_name() const {
  std::string result = attrNames[0];
  for (unsigned i = 1; i < attrNames.size(); i++)
    result += std::string("::") + attrNames[i];
  return result;
}

std::string Annotation::to_string() const {
  std::string result = "[[";
  result += get_qualified_name();
  if (!params.empty()) {
    result += "(";
    result += params;
    result += ")";
  }
  result += "]]";
  return result;
}
