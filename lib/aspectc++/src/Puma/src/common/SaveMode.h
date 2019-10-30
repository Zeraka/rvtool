// This file is part of PUMA.
// Copyright (C) 1999-2016  The PUMA developer team.
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

#ifndef PUMA_SaveMode_H
#define PUMA_SaveMode_H

/** \file
 * File save modes. */

namespace Puma {

/** \class SaveMode SaveMode.h Puma/SaveMode.h
 * Project file save modes.
 * \ingroup common */
struct SaveMode {
  /** Save modes. */
  enum Mode {
    /** Overwrite existing file. */
    OVERWRITE = 1,
    /** Rename existing file. */
    RENAME_OLD,
    /** Save file with a new suffix. */
    NEW_SUFFIX
  };
};

} // namespace Puma

#endif /* PUMA_SaveMode_H */
