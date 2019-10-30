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

#ifndef PUMA_ConfOption_H
#define PUMA_ConfOption_H

/** \file
 * Configuration option abstraction. */

#include "Puma/Array.h"
#include "Puma/StrCol.h"
#include <stdlib.h>

namespace Puma {

/** \class ConfOption ConfOption.h Puma/ConfOption.h
 * Configuration option abstraction. A configuration option has
 * a name and an optional list of arguments. System priority
 * configuration options have a lower priority than non-system
 * options.
 * \ingroup common */
class ConfOption {
  const char *m_name;
  unsigned int m_hash;
  Array<const char *> m_args;
  bool m_system;

public:
  /** Constructor.
   * \param option The full name of the option.
   * \param system Has system priority or not. */
  ConfOption(const char *option, bool system = false);
  /** Constructor.
   * \param option The full name of the option.
   * \param arg The argument value of the option.
   * \param system Has system priority or not. */
  ConfOption(const char *option, const char *arg, bool system = false);
  /** Constructor.
   * \param option The full name of the option.
   * \param arg1 The first argument value of the option.
   * \param arg2 The second argument value of the option.
   * \param system Has system priority or not. */
  ConfOption(const char *option, const char *arg1, const char *arg2, bool system = false);
  /** Destroy this option. */
  ~ConfOption();

  /** Check if the option is an system priority option.
   * \return True if a system priority option. */
  bool isSystem() const;

  /** Get the name of the option.
   * \return The full name of the option. */
  const char *Name() const;
  /** Get the hash value of the option's name.
   * Allows faster comparison when searching for this option.
   * \return The hash value. */
  unsigned int Hash() const;

  /** Get the number of option arguments.
   * \return The number of arguments. */
  unsigned Arguments() const;
  /** Get the n-th option argument.
   * \param n The index of the argument to get.
   * \return The n-th argument. */
  const char *Argument(unsigned n) const;
  /** Add a further option argument.
   * \param arg The argument value. */
  void addArgument(const char *arg);
};

inline ConfOption::ConfOption(const char *n, bool system) :
    m_args(1, 1), m_system(system) {
  m_name = StrCol::dup(n);
  m_hash = StrCol::hash(n);
}
inline ConfOption::ConfOption(const char *n, const char *a1, bool system) :
    m_args(1, 1), m_system(system) {
  m_name = StrCol::dup(n);
  m_hash = StrCol::hash(n);
  addArgument(a1);
}
inline ConfOption::ConfOption(const char *n, const char *a1, const char *a2, bool system) :
    m_args(2, 1), m_system(system) {
  m_name = StrCol::dup(n);
  m_hash = StrCol::hash(n);
  addArgument(a1);
  addArgument(a2);
}

inline ConfOption::~ConfOption() {
  delete[] m_name;
  for (unsigned i = 0; i < Arguments(); i++)
    delete[] Argument(i);
}

inline bool ConfOption::isSystem() const {
  return m_system;
}
inline const char *ConfOption::Name() const {
  return m_name;
}
inline unsigned int ConfOption::Hash() const {
  return m_hash;
}
inline unsigned ConfOption::Arguments() const {
  return m_args.length();
}
inline const char *ConfOption::Argument(unsigned i) const {
  return m_args.lookup(i);
}

inline void ConfOption::addArgument(const char *arg) {
  if (arg) {
    m_args.append(StrCol::dup(arg));
  }
}

} // namespace Puma

#endif /* PUMA_ConfOption_H */
