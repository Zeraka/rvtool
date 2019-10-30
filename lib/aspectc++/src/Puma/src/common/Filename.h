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

#ifndef PUMA_Filename_H
#define PUMA_Filename_H

/** \file
 * Filename abstraction. */

#include "Puma/SmartPtr.h"
#include "Puma/Printable.h"
#include "Puma/FilenameInfo.h"

namespace Puma {

/** \class Filename Filename.h Puma/Filename.h
 * Abstraction of a filename using a smart pointer for automatic
 * memory allocation / deallocation. It encapsulates often needed
 * operations like separating the path portion of the filename or
 * the check whether the filename is absolute.
 * \ingroup common */
class Filename : public Printable {
  SmartPtr<FilenameInfo> m_info;

public:
  /** Constructor. */
  Filename() {
  }
  /** Constructor.
   * \param filename The filename. */
  Filename(const char *filename) {
    name(filename);
  }
  /** Assign a new filename.
   * \param filename The filename.
   * \return A reference to this filename. */
  Filename &operator=(const char *filename) {
    name(filename);
    return *this;
  }
  /** Check if the filename is not NULL.
   * \return True if the filename is valid. */
  bool operator~() const {
    return m_info.valid();
  }
  /** Check if the filename is not NULL.
   * \return True if the filename is valid. */
  bool is_defined() const {
    return m_info.valid();
  }
  /** Set a new filename.
   * \param filename The new filename. */
  inline void name(const char *filename);
  /** Get the filename.
   * \return The filename. */
  inline const char *name() const;
  /** Get the path portion of the filename.
   * \return The path to the file. */
  inline const char *path();
  /** Get the root of an absolute filename, i.e. everything
   * up to the first path delimiter. This is for instance the
   * drive portion of the filename on Windows (c:\). On Unix systems
   * this is simply a slash (/).
   * \return The root or NULL if filename is not absolute. */
  inline const char *root();
  /** Check whether the filename is absolute, i.e. starts at the
   * root of the file system.
   * \return True if the filename is absolute. */
  inline bool is_absolute() const;
  /** Check if this filename equals the given.
   * \param filename The other filename.
   * \return True if both filenames are equal. */
  inline bool operator==(const Filename &filename) const;
  /** Check if this filename not equals the given.
   * \param filename The other filename.
   * \return True if both filenames are not equal. */
  bool operator!=(const Filename &filename) const {
    return !(*this == filename);
  }
  /** Print this filename on the given output stream.
   * \param out The output stream. */
  virtual void print(std::ostream &out) const {
    if (m_info)
      out << name();
  }
};

inline void Filename::name(const char *n) {
  m_info = new FilenameInfo;
  m_info->name(n);
}

inline const char *Filename::name() const {
  return m_info->name();
}

inline const char *Filename::path() {
  return m_info->path();
}

inline const char *Filename::root() {
  return m_info->root();
}

inline bool Filename::is_absolute() const {
  return m_info->is_absolute();
}

inline bool Filename::operator==(const Filename &f) const {
  if (m_info == f.m_info)
    return true;
  if (!m_info || !f.m_info)
    return false;
  return **m_info == **f.m_info;
}

} // namespace Puma

#endif /* PUMA_Filename_H */
