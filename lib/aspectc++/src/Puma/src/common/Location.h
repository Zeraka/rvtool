// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_Location_H
#define PUMA_Location_H

/** \file
 * Token location implementation. */

#include "Puma/Filename.h"

namespace Puma {

/** \class Location Location.h Puma/Location.h
 * Location of a token in a source file.
 * \ingroup common */
class Location {
  Filename m_filename;
  int m_line;
  int m_column;

public:
  /** Construct an empty location. */
  Location()
      : m_filename(), m_line(0), m_column(0) {
  }
  /** Construct a token location.
   * \param filename The name of the file the token is located in.
   * \param line The line number.
   * \param column The optional column number. Defaults to 0. */
  Location(Filename filename, int line, int column = 0) {
    setup(filename, line, column);
  }
  /** Initialize a token location.
   * \param filename The name of the file the token is located in.
   * \param line The line number.
   * \param column The optional column number. Defaults to 0. */
  void setup(Filename filename, int line, int column = 0) {
    m_filename = filename;
    m_line = line;
    m_column = column;
  }
  /** Get the name of the file the token is located in.
   * \return A reference to the file name. */
  const Filename &filename() const {
    return m_filename;
  }
  /** Get the number of the line in the file.
   * \return The line number. */
  int line() const {
    return m_line;
  }
  /** Get the number of the column in the file.
   * \return The column number. */
  int column() const {
    return m_column;
  }
  /** Compare with another location.
   * \param location The other location.
   * \return True if both locations are equal. */
  bool operator ==(const Location &location) const {
    return filename() == location.filename() && line() == location.line() && column() == location.column();
  }
  /** Compare with another location.
   * \param location The other location.
   * \return True if both locations differ. */
  bool operator !=(const Location &location) const {
    return !(*this == location);
  }
  /** Compare with another location in the same file.
   * \param location The other location.
   * \return True if this location is before the given location in the same file. */
  bool operator <(const Location &location) const {
    if (filename() != location.filename())
      return false;
    return line() < location.line() && column() < location.column();
  }
};

/** Output stream operator to print the given location on the given output stream.
 * \param out The output stream.
 * \param location The location to print.
 * \return A reference to the output stream. */
inline std::ostream &operator <<(std::ostream &out, const Location &location) {
#ifdef VISUAL_STUDIO
  out << location.filename() << "(" << location.line() << ")";
#else
  out << location.filename() << ":" << location.line();
#endif
  return out;
}

} // namespace Puma

#endif /* PUMA_Location_H */
