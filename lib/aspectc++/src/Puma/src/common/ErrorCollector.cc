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

#include "Puma/ErrorCollector.h"
#include <string.h>

namespace Puma {

ErrorCollector::Message::Message(const ErrorSeverity &s, Location l, const char *m) :
    m_severity(s), m_location(l), m_haveLocation(true) {
  m_message = new char[strlen(m) + 1];
  strcpy(m_message, m);
}

ErrorCollector::Message::Message(const ErrorSeverity &s, const char *m) :
    m_severity(s), m_haveLocation(false) {
  m_message = new char[strlen(m) + 1];
  strcpy(m_message, m);
}

ErrorCollector::Message::~Message() {
  delete[] m_message;
}

bool ErrorCollector::Message::operator ==(const Message& msg) const {
  if (m_haveLocation != msg.m_haveLocation)
    return false;
  if (m_haveLocation)
    if (m_location != msg.m_location)
      return false;
  if (m_severity != msg.m_severity)
    return false;
  if (strcmp(m_message, msg.m_message) != 0)
    return false;
  return true;
}

void ErrorCollector::Message::print(ErrorSink &e) {
  if (m_haveLocation)
    e << m_location;
  e << m_severity << m_message << Puma::endMessage;
}

ErrorCollector& ErrorCollector::operator <<(const char *v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(long v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(unsigned v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(int v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(short v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(char v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(double v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(const ErrorSeverity& sev) {
  if (m_severity < sev)
    m_severity = sev;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(Location v) {
  m_location = v;
  m_haveLocation = true;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(const Printable& v) {
  m_msg << v;
  return *this;
}

ErrorCollector& ErrorCollector::operator <<(void (*f)(ErrorSink&)) {
  (*f)(*(ErrorSink*) this);
  return *this;
}

void ErrorCollector::endMessage() {
  Message *msg;
  if (m_haveLocation)
    msg = new Message(m_severity, m_location, m_msg.str().c_str());
  else
    msg = new Message(m_severity, m_msg.str().c_str());

  bool found = false;
  for (int m = 0; m < m_messages.length(); m++)
    if (*m_messages.lookup(m) == *msg)
      found = true;

  if (!found)
    m_messages.append(msg);
  else
    delete msg;

  clear();
}

void ErrorCollector::clear() {
  m_severity = sev_none;
  m_haveLocation = false;
  m_msg.str("");
  m_msg.clear();
}

int ErrorCollector::index() {
  return (int) m_messages.length();
}

void ErrorCollector::index(int n) {
  while (n >= 0 && m_messages.length() > n) {
    delete m_messages.lookup(m_messages.length() - 1);
    m_messages.remove(m_messages.length() - 1);
  }
}

void ErrorCollector::shift(ErrorSink &e) {
  for (int m = 0; m < m_messages.length(); m++)
    m_messages.lookup(m)->print(e);
  index(0);
}

const ErrorSeverity &ErrorCollector::severity() const {
  const ErrorSeverity *max_sev = &sev_none;
  for (int m = 0; m < m_messages.length(); m++)
    if (*max_sev < m_messages.lookup(m)->severity())
      max_sev = &m_messages.lookup(m)->severity();
  return *max_sev;
}

} // namespace Puma
