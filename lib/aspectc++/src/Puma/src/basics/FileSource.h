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

#ifndef PUMA_FileSource_H
#define PUMA_FileSource_H

/** \file
 * File source wrapper. */

#include "Puma/ErrorSink.h"
#include "Puma/FdSource.h"
#include "Puma/SysCall.h"
#include <fcntl.h>

namespace Puma {

/** \class FileSource FileSource.h Puma/FileSource.h
 * File wrapper implementing the Source interface. 
 * \ingroup basics */
class FileSource : public FdSource {
public:
  /** Construct a new file wrapper.
   * Opens the given file in read-only mode.
   * \param path The path to the file.
   * \param err Optional error stream. */
  FileSource(const char *path, ErrorSink *err = (ErrorSink*) 0) {
    fd(SysCall::open(path, O_RDONLY, err));
  }
  /** Destroy the file wrapper. Closes the file opened in the constructor. */
  ~FileSource() {
    SysCall::close(fd());
  }
  /** Read n bytes from the file into the given string.
   * \param dest The string used as buffer for the bytes read.
   * \param n The number of bytes to read.
   * \return The number of bytes read, or -1 in case of errors. */
  virtual int read(char *dest, int n) {
    return FdSource::read(dest, n);
  }
};

} // namespace Puma

#endif /* PUMA_FileSource_H */
