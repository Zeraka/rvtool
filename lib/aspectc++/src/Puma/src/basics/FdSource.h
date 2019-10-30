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

#ifndef PUMA_FdSource_H
#define PUMA_FdSource_H

/** \file
 * File descriptor input source implementation. */

#include "Puma/Source.h"
#include "Puma/SysCall.h"

namespace Puma {

/** \class FdSource FdSource.h Puma/FdSource.h
 * File descriptor wrapper implementing the Source interface. 
 * \ingroup basics */
class FdSource : public Source {
  int m_fd;

public:
  /** Construct a new file descriptor wrapper with invalid file descriptor. */
  FdSource()
      : m_fd(-1) {
  }
  /** Construct a new wrapper for the given file descriptor.
   * \param fdesc The file descriptor. */
  FdSource(int fdesc)
      : m_fd(fdesc) {
  }
  /** Destroy the wrapper. Does not close the file descriptor. */
  virtual ~FdSource() {
  }
  /** Read n bytes from the file into the given string.
   * \param dest The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  virtual int read(char *dest, int n) {
    return SysCall::read(m_fd, dest, n);
  }
  /** Set the wrapped file descriptor.
   * \param fdesc The file descriptor. */
  void fd(int fdesc) {
    m_fd = fdesc;
  }
  /** Get the wrapped file descriptor.
   * \return The file descriptor, or -1 if no file descriptor set. */
  int fd() {
    return m_fd;
  }
  /** Get the size of the file.
   * \return The size of the file, or -1 if size could not be determined. */
  int size() {
    FileInfo fileinfo;
    return (SysCall::fstat(m_fd, fileinfo)) ? fileinfo.size() : -1;
  }
};

} // namespace Puma

#endif /* PUMA_FdSource_H */
