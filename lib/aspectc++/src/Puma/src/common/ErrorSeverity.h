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

#ifndef PUMA_ErrorSeverity_H
#define PUMA_ErrorSeverity_H

/** \file
 * Severity of errors. */

namespace Puma {

/** \class ErrorSeverity ErrorSeverity.h Puma/ErrorSeverity.h
 * Severity of errors represented by a short integer and a textual
 * representation. Following error severities are defined (in
 * ascending order):
 * - Puma::sev_none
 * - Puma::sev_message
 * - Puma::sev_warning
 * - Puma::sev_error
 * - Puma::sev_fatal
 *
 * \ingroup common */
class ErrorSeverity {
  unsigned short m_severity;
  const char *m_text;

public:
  /** Constructor.
   * \param severity The severity value.
   * \param text The textual representation. */
  ErrorSeverity(unsigned short severity = 0, const char* text = (const char*) 0) :
      m_severity(severity), m_text(text) {
  }
  /** Copy-constructor.
   * \param copy The error severity to copy. */
  ErrorSeverity(const ErrorSeverity &copy) :
      m_severity(copy.m_severity), m_text(copy.m_text) {
  }
  /** Assign another severity.
   * \param severity The severity to assign.
   * \return A reference to this. */
  ErrorSeverity &operator =(const ErrorSeverity &severity) {
    m_severity = severity.m_severity;
    m_text = severity.m_text;
    return *this;
  }

  /** Check if this severity is lower than the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator <(const ErrorSeverity &other) const {
    return m_severity < other.m_severity;
  }
  /** Check if this severity is higher than the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator >(const ErrorSeverity &other) const {
    return m_severity > other.m_severity;
  }
  /** Check if this severity is lower than or the same as the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator <=(const ErrorSeverity &other) const {
    return m_severity < other.m_severity || m_severity == other.m_severity;
  }
  /** Check if this severity is higher than or the same as the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator >=(const ErrorSeverity &other) const {
    return m_severity > other.m_severity || m_severity == other.m_severity;
  }
  /** Check if this severity is the same as the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator ==(const ErrorSeverity &other) const {
    return m_severity == other.m_severity;
  }
  /** Check if this severity is not the same as the given.
   * \param other The severity to compare with.
   * \return 1 if true, 0 otherwise. */
  int operator !=(const ErrorSeverity &other) const {
    return m_severity != other.m_severity;
  }

  /** Get the textual representation of this severity.
   * \return The textual representation. */
  const char *text() const {
    return m_text;
  }
};

/** Error has no severity. */
extern const ErrorSeverity sev_none;
/** Informal message. */
extern const ErrorSeverity sev_message;
/** A warning. */
extern const ErrorSeverity sev_warning;
/** An error. */
extern const ErrorSeverity sev_error;
/** A fatal error. */
extern const ErrorSeverity sev_fatal;

} // namespace Puma

#endif /* PUMA_ErrorSeverity_H */
