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

#ifndef PUMA_ErrorSink_H
#define PUMA_ErrorSink_H

/** \file
 * Error sink interface. */

#include "Puma/ErrorSeverity.h"
#include "Puma/Printable.h"
#include "Puma/Location.h"

namespace Puma {

/** \class ErrorSink ErrorSink.h Puma/ErrorSink.h
 * This is an interface for classes that may be used to consume error
 * messages during a process where many errors can occur like parsing
 * a program.
 *
 * The error information is handed over to the ErrorSink in stream-like
 * style. Different severities of errors are predefined and can be used
 * to classify error information that follows. Also a location can be
 * handed over to the ErrorSink to specify where the error was found
 * (filename, line, etc.). The error information itself can be of any
 * standard data type or anything that implements the Printable interface.
 * An error message is finished with the Puma::endMessage stream manipulator.
 *
 * Specializations of this class could either build and print error
 * messages or collect them for a later processing.
 * \ingroup common */
class ErrorSink {
public:
  /** Destructor. */
  virtual ~ErrorSink() {
  }

  /** Add given string to current error message.
   * \param str The string to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(const char *str) = 0;
  /** Add given long integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(long value) = 0;
  /** Add given unsigned integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(unsigned value) = 0;
  /** Add given signed integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(int value) = 0;
  /** Add given short integer value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(short value) = 0;
  /** Add given character to current error message.
   * \param c The character to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(char c) = 0;
  /** Add given floating point value to current error message.
   * \param value The value to add.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(double value) = 0;
  /** Change the severity of the current error message.
   * \param severity The new severity.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(const ErrorSeverity &severity) = 0;
  /** Set the location the error occurred.
   * \param location The error location.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(Location location) = 0;
  /** Add the serialized value of the given object to the current error message.
   * \param obj The object to serialize.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(const Printable &obj) = 0;
  /** Apply the given stream manipulator.
   * \param fct The stream manipulator function.
   * \return A reference to this error stream. */
  virtual ErrorSink &operator <<(void (*fct)(ErrorSink &)) = 0;

  /** Finish the current error message. */
  virtual void endMessage() = 0;
};

/** Finish the current error message on the given error sink.
 * \param err The error sink. */
void endMessage(ErrorSink &err);

} // namespace Puma

#endif /* PUMA_ErrorSink_H */
