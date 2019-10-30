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

#ifndef PUMA_ErrorStream_H
#define PUMA_ErrorStream_H

/** \file
 * Error stream implementation. */

#include <sstream>
#include "Puma/ErrorSink.h"
#include "Puma/ErrorSeverity.h"
#include "Puma/Location.h"

namespace Puma {

/** \class ErrorStream ErrorStream.h Puma/ErrorStream.h
 * An error stream is a special ErrorSink that prints out an error message
 * as soon as it is complete. It also stores the maximum error severity.
 * The error stream can be reset to its initial state.
 * \ingroup common */
class ErrorStream : public ErrorSink {
  std::ostream *m_out;
  ErrorSeverity m_maxSeverity;
  ErrorSeverity m_currSeverity;
  Location m_location;
  bool m_haveLocation;
  std::ostringstream *m_msg;
  bool m_silently;

  void output();
  void clear();

public:
  /** Constructor.
   * \param out The output stream to use. Defaults to std::cerr. */
  ErrorStream(std::ostream &out = std::cerr) :
      m_out(&out), m_msg(0) {
    reset();
    m_silently = false;
  }
  /** Destructor. */
  virtual ~ErrorStream() {
    delete m_msg;
  }

  /** Add given string to current error message.
   * \param str The string to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(const char *str);
  /** Add given long integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(long value);
  /** Add given unsigned integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(unsigned value);
  /** Add given signed integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(int value);
  /** Add given short integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(short value);
  /** Add given character to current error message.
   * \param c The character to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(char c);
  /** Add given floating point value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(double value);
  /** Change the severity of the current error message.
   * \param sev The new severity.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(const ErrorSeverity &sev);
  /** Set the location the error occurred.
   * \param location The error location.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(Location location);
  /** Add the serialized value of the given object to the current error message.
   * \param obj The object to serialize.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(const Printable &obj);
  /** Apply the given stream manipulator.
   * \param fct The stream manipulator function.
   * \return A reference to this error stream. */
  ErrorStream &operator <<(void (*fct)(ErrorSink &));

  /** Finish the current error message. */
  void endMessage();
  /** Reset the error stream to its initial state. */
  void reset();
  /** Turn to silent mode. In silent mode no message is printed
   * on the output stream.
   * \return True if the previous mode was silent already. */
  bool silent();
  /** Turn to loud mode. In loud mode messages are printed
   * on the output stream. This is the default mode.
   * \return True if the previous mode was loud already. */
  bool loud();

  /** Get the current error severity.
   * \return The error severity. */
  ErrorSeverity severity() const {
    return m_maxSeverity;
  }
  /** Set a new severity for all following messages.
   * \param sev The new severity. */
  void severity(ErrorSeverity& sev) {
    m_maxSeverity = sev;
  }
};

} // namespace Puma

#endif /* PUMA_ErrorStream_H */
