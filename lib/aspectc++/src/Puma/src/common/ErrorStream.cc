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

#include "Puma/ErrorStream.h"
#include "Puma/Location.h"
#include <iostream>

namespace Puma {

ErrorStream &ErrorStream::operator <<(const char *v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(long v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(unsigned v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(int v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(short v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(char v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(double v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(const ErrorSeverity &sev) {
  m_currSeverity = sev;
  if (m_currSeverity > m_maxSeverity)
    m_maxSeverity = m_currSeverity;
  return *this;
}

ErrorStream &ErrorStream::operator <<(Location v) {
  if (!m_silently) {
    m_location = v;
    m_haveLocation = (v.filename().is_defined() && v.filename().name());
  }
  return *this;
}

ErrorStream &ErrorStream::operator <<(const Printable &v) {
  if (!m_silently)
    *m_msg << v;
  return *this;
}

ErrorStream &ErrorStream::operator <<(void (*f)(ErrorSink &)) {
  (*f)(*(ErrorSink*) this);
  return *this;
}

void ErrorStream::endMessage() {
  output();
  clear();
}

void ErrorStream::reset() {
  m_maxSeverity = sev_none;
  clear();
}

bool ErrorStream::silent() {
  bool was_silent = m_silently;
  m_silently = true;
  return was_silent;
}

bool ErrorStream::loud() {
  bool was_loud = !m_silently;
  m_silently = false;
  return was_loud;
}

void ErrorStream::clear() {
  m_currSeverity = sev_none;
  m_haveLocation = false;
  if (m_msg)
    delete m_msg;
  m_msg = new std::ostringstream;
}

void ErrorStream::output() {
  if (m_silently)
    return;

  if (m_haveLocation)
    *m_out << m_location << ": ";

  if (m_currSeverity != sev_none)
    *m_out << m_currSeverity.text() << ": ";

  *m_out << m_msg->str().c_str() << std::endl;
}

} // namespace Puma
