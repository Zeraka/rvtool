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

#ifndef PUMA_ErrorCollector_H
#define PUMA_ErrorCollector_H

/** \file
 * Error stream implementation. */

#include "Puma/ErrorSink.h"
#include "Puma/Array.h"
#include "Puma/Location.h"
#include "Puma/ErrorSeverity.h"
#include <sstream> 

namespace Puma {

/** \class ErrorCollector ErrorCollector.h Puma/ErrorCollector.h
 * Error stream implementation that collects all messages in an
 * internal message stack. Messages are finished with the Puma::endMessage
 * stream manipulator. Collected messages can be shifted to another
 * ErrorSink.
 * \ingroup common */
class ErrorCollector : public ErrorSink {
  class Message {
    ErrorSeverity m_severity;
    Location m_location;
    bool m_haveLocation;
    char *m_message;

  public:
    Message(const ErrorSeverity &s, Location l, const char *m);
    Message(const ErrorSeverity &s, const char *m);
    ~Message();
    bool operator ==(const Message&) const;
    void print(ErrorSink &e);
    const ErrorSeverity &severity() const {
      return m_severity;
    }
  };

  Array<Message*> m_messages;
  Location m_location;
  bool m_haveLocation;
  ErrorSeverity m_severity;
  std::ostringstream m_msg;

  void clear();

public:
  /** Constructor. */
  ErrorCollector() {
    clear();
  }
  /** Destructor. */
  virtual ~ErrorCollector() {
  }

  /** Add given string to current error message.
   * \param str The string to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(const char *str);
  /** Add given long integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(long value);
  /** Add given unsigned integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(unsigned value);
  /** Add given signed integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(int value);
  /** Add given short integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(short value);
  /** Add given character to current error message.
   * \param c The character to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(char c);
  /** Add given floating point value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(double value);
  /** Change the severity of the current error message.
   * \param severity The new severity.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(const ErrorSeverity& severity);
  /** Set the location the error occurred.
   * \param location The error location.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(Location location);
  /** Add the serialized value of the given object to the current error message.
   * \param obj The object to serialize.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(const Printable& obj);
  /** Apply the given stream manipulator.
   * \param fct The stream manipulator function.
   * \return A reference to this error stream. */
  ErrorCollector& operator <<(void (*fct)(ErrorSink&));

  /** Finish the current error message. */
  void endMessage();

  /** Get the index of the current error message on the
   * message stack.
   * \return The index of the current error message. */
  int index();
  /** Set the index of the current error message on the
   * message stack. If the new index is less than the current
   * number of messages on the stack, all messages with an
   * index greater than the given index are dropped. If the new
   * index is 0 then all previously collected messages are
   * dropped.
   * \param idx The new index. */
  void index(int idx);

  /** Shift all collected messages to the destination error sink.
   * \param dest The destination error sink. */
  void shift(ErrorSink &dest);

  /** Get the current error severity of the stream.
   * \return A reference to the current error severity. */
  const ErrorSeverity &severity() const;
};

} // namespace Puma

#endif /* PUMA_ErrorCollector_H */
