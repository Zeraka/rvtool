#ifndef ANNOTATION_H
#define ANNOTATION_H

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

#include <string>
#include <vector>
#include <map>
#include "clang/Basic/SourceLocation.h"

struct Annotation {
  clang::SourceLocation tokBegin, tokEnd, seqBegin, seqEnd;
  std::vector<std::string> attrNames;
  std::string params;
  bool is_user_defined;
  std::string get_name() const;
  std::string get_qualified_name() const;
  std::string to_string() const;
};

typedef std::map<clang::SourceLocation, Annotation> AnnotationMap;

#endif // ANNOTATION_H
