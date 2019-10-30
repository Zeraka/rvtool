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

#ifndef PUMA_UnitState_H
#define PUMA_UnitState_H

/** \file
 * Unit modification state. */

namespace Puma {

/** \class UnitState UnitState.h Puma/UnitState.h
 * Unit state information, like modification state
 * and time of last modification.
 * \ingroup common */
class UnitState {
  bool m_modified;
  long m_lastModified;

public:
  /** Constructor. */
  UnitState();
  /** Check if the unit was marked as modified.
   * \return True if unit was modified. */
  bool isModified() const;
  /** Get the last modification time of the unit.
   * \return The last modification time or 0 if not set. */
  long lastModified() const;
  /** Mark the unit as being modified. */
  void modified();
  /** Set a last modification time for the unit.
   * \param time The last modification time as returned by stat(). */
  void lastModified(long time);
  /** Mark the unit as not being modified. */
  void unmodified();
};

inline UnitState::UnitState() {
  m_modified = false;
  m_lastModified = 0;
}

inline bool UnitState::isModified() const {
  return m_modified;
}
inline long UnitState::lastModified() const {
  return m_lastModified;
}

inline void UnitState::modified() {
  m_modified = true;
  m_lastModified = 0;
}
inline void UnitState::unmodified() {
  m_modified = false;
}
inline void UnitState::lastModified(long when) {
  m_lastModified = when;
}

} // namespace Puma

#endif /* PUMA_UnitState_H */
