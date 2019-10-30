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

#ifndef PUMA_VerboseMgr_H
#define PUMA_VerboseMgr_H

/** \file
 * Manager for verbose manager. */

#include <sstream>
#include <iostream>

namespace Puma {

/** \class VerboseMgr VerboseMgr.h Puma/VerboseMgr.h
 * Indented output stream with a adjustable verbosity level.
 * If the level reaches the verbosity level, all messages
 * printed on the stream are ignored. Use the stream manipulator
 * Puma::endvm to finish a message on the stream.
 * \ingroup basics */
class VerboseMgr : public std::ostringstream {
  int m_level;
  int m_verbose;
  std::ostream &m_out;

public:
  /** Constructor. Default verbosity level is 10.
   * \param out The destination output stream. */
  VerboseMgr(std::ostream &out)
      : m_level(0), m_verbose(10), m_out(out) {
  }
  /** Construct a new verbosity manager.
   * \param out The destination output stream.
   * \param verbose The default verbosity level. */
  VerboseMgr(std::ostream &out, int verbose)
      : m_level(0), m_verbose(verbose), m_out(out) {
  }
  /** Set the verbosity level.
   * \param verbose The verbosity level. */
  void verbose(int verbose) {
    m_verbose = verbose;
  }
  /** Get the verbosity level.
   * \return The verbosity level. */
  int verbose() {
    return m_verbose;
  }
  /** Increase the current level starting at 0. */
  void operator ++(int) {
    m_level++;
  }
  /** Decrease the current level starting at 0. */
  void operator --(int) {
    m_level--;
  }
  /** Finish the message and write it on the destination
   * output stream if the verbosity level is not yet reached. */
  void endl();
};

/** Finish a VerboseMgr message and write it on the destination
 * output stream if the verbosity level is not yet reached.
 * \param outs The VerboseMgr stream.
 * \return A reference to the input VerboseMgr stream. */
inline std::ostream &endvm(std::ostream &outs) {
  ((VerboseMgr&) outs).endl();
  return outs;
}

} // namespace Puma

#endif /* PUMA_VerboseMgr_H */
