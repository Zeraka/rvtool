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

#ifndef PUMA_PathInfo_H
#define PUMA_PathInfo_H

/** \file
 * Path information implementation. */

namespace Puma {

/** \class PathInfo PathInfo.h Puma/PathInfo.h
 * A path information object consists of a source and optionally
 * its destination path. For comparison purposes the canonical version
 * of the source path can be stored.
 * \ingroup common */
class PathInfo {
  const char *m_srcPath;
  const char *m_srcPathCanonical;
  const char *m_destPath;

public:
  /** Contruct a new empty path information object. */
  PathInfo()
      : m_srcPath(0), m_srcPathCanonical(0), m_destPath(0) {
  }
  /** Copy-constructor.
   * \param other The other path information object to copy. */
  PathInfo(const PathInfo &other);
  /** Destroy this object and free the stored paths. */
  ~PathInfo();

  /** Assign another path information object to this one.
   * \param other The other path information object to copy.
   * \return A reference to this object. */
  PathInfo &operator =(const PathInfo &other);

  /** Set the source path.*/
  void src(const char *path);
  /** Set the canonical variant of the source path.*/
  void srcCanonical(const char *path);
  /** Set the destination path.*/
  void dest(const char *path);

  /** Get the source path.
   * \return The source path. */
  const char *src() const {
    return m_srcPath;
  }
  /** Get the canonical variant of the source path.
   * \return The canonical source path. */
  const char *srcCanonical() const {
    return m_srcPathCanonical;
  }
  /** Get the destination path.
   * \return The destination path. */
  const char *dest() const {
    return m_destPath;
  }
};

} // namespace Puma

#endif /* PUMA_PathInfo_H */
