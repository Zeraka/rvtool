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

#ifndef PUMA_Config_H
#define PUMA_Config_H

/** \file
 *  Configuration information from file or command line. */

/** \page configuration Configuration Options
 *
 * Several aspects of the functionality of PUMA can be conﬁgured using command line 
 * options or a configuration file. The following configuration options are understood 
 * by the library.
 * 
 * ### File Handling Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | -p, --path PATH | Add a project source directory
 * | -d, --dest PATH | Add a destination folder for modified sources
 * | -w, --write-protected PATH | Add a write protected project directory
 * | -e, --extension STRING | Set the extension for source files
 * | -s, --suffix STRING | Set the suffix for output files
 * | --config FILE | Load configuration options from a file
 * | --system-config FILE | Load low precedence configuration options from a file
 * | --save-new-suffix | Append new suffix on existing files when saving
 * | --save-overwrite | Overwrite existing files when saving
 * | --save-rename-old | Rename existing files when saving
 * 
 * ### Preprocessor Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | -A PREDICATE(ANSWER) | Assert a preprocessor predicate, e.g. -Asystem(linux)
 * | -D NAME[=BODY] | Define a preprocessor macro, e.g. -DSYSTEM=linux or -D DEBUG
 * | -U NAME | Undefine a preprocessor macro, e.g. -U SYSTEM
 * | -I PATH | Add a non-system include directory
 * | --isystem PATH | Add a system include directory
 * | --include FILE | Include the given file at the beginning of each source file
 * | --lock-macro NAME[=BODY] | Define an immutable preprocessor macro
 * | --inhibit-macro NAME | Add the name of a macro whose definition will be ignored
 * 
 * ### General %Parser Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | --lang-c | Set input language to C
 * | --lang-ec++ | Set input language to Embedded C++
 * | --lang-c++ | Set input language to C++
 * | --size-type TYPE | Set the internal type for size_t
 * | --ptrdiff-type TYPE | Set the internal type for ptrdiff_t
 * | --target TRIPLE | Set target triple which determines sizes and alignments for built-in types
 * | --builtin-type-traits | Enable built-in type traits
 * | --verbose-errors | Enable verbose error messages
 * | --match-expr | Enable match expression language extensions
 * 
 * ### Extended %Parser Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | --skip-bodies-all | Don't parse function bodies
 * | --skip-bodies-tpl | Don't parse function bodies of templates
 * | --skip-bodies-non-prj | Don't parse function bodies in non-project files
 * | --skip-bodies-non-prim | Don't parse function bodies in non-primary files
 * | --pseudo-instances | Simplified template instantiation scheme (deprecated)
 * | --inst-fct-bodies | Enable function template body instantiation
 * | --template-depth DEPTH | Set the maximum instantiation depth for templates
 * 
 * ### VisualC++ Extension Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | --vc | Enable VisualC++ language extensions
 * | --import-handler FILE | Set a handler for resolving \#import directives
 * 
 * ### GNU C/C++ Extension Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | --gnu [VERSION] | Enable all GNU C/C++ language extensions (optionally version dependent)
 * | --gnu-nested-fct | Enable GNU C/C++ nested functions
 * | --gnu-condition-scope | Enable GNU C/C++ condition scope
 * | --gnu-param-decl | Enable GNU C/C++ parameter declarator
 * | --gnu-fct-decl | Enable GNU C/C++ function declarator
 * | --gnu-param-scope | Enable GNU C/C++ function parameter scope
 * | --gnu-default-args | Enable GNU C/C++ function default arguments
 * | --gnu-extended-asm | Enable GNU C/C++ extended asm syntax
 * | --gnu-extended-expr | Enable GNU C/C++ extended expressions
 * | --gnu-long-long | Enable GNU C/C++ long long type
 * | --gnu-int128 | Enable GNU type __int128 extension
 * | --gnu-name-scope | Enable GNU C/C++ name scope
 * | --gnu-implicit-int | Enable GNU implicit int extension
 * | --gnu-fct-attribute | Enable GNU C/C++ function attributes
 * | --gnu-if-then-expr | Enable GNU C/C++ if-then expression syntax
 * | --gnu-std-hack | Enable GNU C/C++ implicit namespace std hack
 * | --gnu-friend-injection | Enable GNU friend injection extension
 * | --gnu-pic24 | Enable GNU PIC24 and dsPIC extension
 * 
 * ### C++1x Extension Options
 * 
 * | Option | Description
 * |----------------------|-----------------------
 * | --c++1x | Enable all C++1x language extensions
 * | --c++1x-static-assert | Enable C++1x static assertions
 * 
 * ### Configuration Files
 * 
 * All command line options can also be specified in a configuration file. 
 * Each option in the configuration file has to start on a new line.
 * 
 * \code{.py}
 * -D i386
 * -D linux
 * -I /usr/include
 * \endcode
 * 
 * Lines beginning with # are interpreted as comments and will be ignored.
 * 
 * \code{.py}
 * ### defines
 * -D i386
 * -D linux
 * ### includes
 * -I /usr/include
 * -I /usr/local/include
 * \endcode
 *
 * Arguments of options containing spaces have to be enclosed in double-quotes. 
 * Double-quotes in the argument have to be escaped.
 * 
 * \code{.py}
 * -D __PTRDIFF_TYPE__=int
 * -D "__SIZE_TYPE__=unsigned int"
 * -D "__VERSION__=\"4.1.0 (Linux)\""
 * \endcode
 * 
 * All occurrences of ${Name} in the configuration file are interpreted as environment 
 * variables and replaced by their values, or by nothing if the variable is not defined. 
 * To avoid variable replacement, $ has to be escaped.
 * 
 * \code{.py}
 * -I ${LIBDIR}/include
 * -D OS_STR=\"${OSTYPE}\"
 * ### same as: #define OS_STR "linux"
 * -D OS_VAR=\"\${OSTYPE}\"
 * ### same as: #define OS_VAR "${OSTYPE}"
 * \endcode
 *
 * The default PUMA configuration file location is /etc/puma.config. It can be overwritten
 * by setting the environment variable PUMA_CONFIG like this:
 *
 * \code{.py}
 * export PUMA_CONFIG=/usr/local/lib/Puma/puma.config
 * \endcode
 *
 * Use class Puma::Config to load the configuration in the correct order from the
 * command line or a configuration file.
 */

#include "Puma/ConfOption.h"
#include "Puma/ErrorStream.h"
#include "Puma/OptsParser.h"
#include "Puma/StrCol.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace Puma {

/** \class Config Config.h Puma/Config.h
 * Load and hold PUMA configuration options in the correct order
 * from the command line or a configuration file.
 * \ingroup common */
class Config {
  typedef std::vector<const ConfOption *> OptionList;
  typedef std::map<unsigned int, const ConfOption*> OptionMap;

  ErrorStream &m_err;
  OptionList m_sysopts;
  OptionList m_useropts;
  OptionList m_sysincls;
  OptionMap m_optnamemap;
  bool m_system;

  static unsigned int OPT_I_HASH;
  static unsigned int OPT_ISYSTEM_HASH;

public:
  /** Configuration option types. */
  enum OPTIONS {
    /** \cond internal */
    PRE_ASSERT = 1,
    PRE_DEFINE,
    PRE_UNDEFINE,
    PRE_LOCK_MACRO,
    PRE_INCLUDE,
    CFG_FILE,
    SYS_CFG_FILE,
    PROJ_PATH,
    PROJ_DESTINATION,
    SET_SUFFIX,
    SET_EXTENSION,
    SET_WRITE_PROTECTED,
    GNU,
    /** \endcond */
    /** Option without argument. */
    SET_OPTION,
    /** Option with argument. */
    SET_OPTION_ARG
  };

public:
  /** Constructor.
   * \param err The error stream to use. */
  Config(ErrorStream &err);
  /** Constructor. Read the given configuration file. See Read()
   * for details.
   * \param err The error stream to use.
   * \param path Path to the configuration file, or NULL. */
  Config(ErrorStream &err, const char *path);
  /** Constructor. Read configuration options from the command
   * line. See Read() for details.
   * \param err The error stream to use.
   * \param argc The number of command line arguments.
   * \param argv The command line arguments. */
  Config(ErrorStream &err, int &argc, char **&argv);
  /** Constructor. Read the configuration from a configuration
   * file and the command line. See Read() for details.
   * \param err The error stream to use.
   * \param path Path to the configuration file, or NULL.
   * \param argc The number of command line arguments.
   * \param argv The command line arguments. */
  Config(ErrorStream &err, const char *path, int &argc, char **&argv);
  /** Copy-constructor.
   * \param copy The configuration to copy. */
  Config(const Config &copy);
  /** Destroy this configuration object. */
  ~Config();

  /** Join this configuration with another.
   * \param other The configuration to join with. */
  void Join(const Config &other);

  /** Read the given configuration file, or the file given in
   * PUMA_CONFIG environment variable if path is NULL, or
   * /etc/puma.config otherwise.
   * \param path The path to the configuration file, or NULL. */
  void Read(const char *path = (const char *) 0);
  /** Read configuration options from the command line and remove
   * all recognized options from the command line.
   * \param argc The number of command line arguments.
   * \param argv The command line arguments. */
  void Read(int &argc, char **&argv);

  /** Add a configuration option to the configuration.
   * \param option The option to add. */
  void Add(const ConfOption *option);
  /** Add a configuration option to the configuration.
   * \param option The option to add.
   * \param system True if this option shall be added as a system priority option. */
  void Add(const char *option, bool system = false);
  /** Add a configuration option to the configuration.
   * \param option The option to add.
   * \param arg The argument or the option.
   * \param system True if this option shall be added as a system priority option. */
  void Add(const char *option, const char *arg, bool system = false);
  /** Add a configuration option to the configuration.
   * \param option The option to add.
   * \param arg1 The first argument or the option.
   * \param arg2 The second argument or the option.
   * \param system True if this option shall be added as a system priority option. */
  void Add(const char *option, const char *arg1, const char *arg2, bool system = false);

  /** Remove an option from the configuration.
   * \param option The option to remove. */
  void Remove(const ConfOption *option);

  /** Get the number options collected.
   * \return The number of options in this configuration. */
  unsigned Options() const;
  /** Get the n-th option in this configuration.
   * \param n The index of the option to get.
   * \return A pointer to the option. */
  const ConfOption *Option(unsigned n) const;
  /** Get a specific option in this configuration.
   * \param option The name of the option to get.
   * \return A pointer to the option. */
  const ConfOption *Option(const char *option) const;

  /** Check if a configuration file is given on the command line.
   * \param argc The number of command line arguments.
   * \param argv The command line arguments.
   * \return True if a custom configuration file is given on the command line. */
  bool CustomConfigFile(int argc, char **argv);

  /** Check if a system configuration file (lower priority options) is given
   * on the command line.
   * \param argc The number of command line arguments.
   * \param argv The command line arguments.
   * \return True if a system custom configuration file is given on the command line. */
  bool CustomSystemConfigFile(int argc, char **argv);

  /** Print the given option on an output stream.
   * \param option The option to print.
   * \param os The output stream. */
  void PrintOption(OptsParser::Option& option, std::ostream& os) const;
  /** Print the collected options on the given output stream.
   * \param os The output stream. */
  void PrintOptions(std::ostream& os) const;

private:
  bool Valid(const char *, const char * = (const char *) 0) const;

  bool Process(OptsParser&, bool unescape = false);
  bool ProcessAssert(const std::string&);
  bool ProcessDefine(const std::string&);
  bool ProcessLockMacro(const std::string&);
  bool ProcessUndefine(const std::string&);
  bool ProcessConfigFile(const std::string&, const std::string&);
  bool ProcessSystemConfigFile(const std::string&, const std::string&);
  bool ProcessArgument(const std::string&, const std::string&);
  bool ProcessOptionalArgument(const std::string&, const std::string&);
  bool ProcessPathArgument(const std::string&, const std::string&);

  std::string Unescape(const std::string&);

  void Remove(const ConfOption *, OptionList& options);
};

inline Config::Config(ErrorStream &e) :
    m_err(e), m_system(false) {
}
inline Config::Config(const Config &c) :
    m_err(c.m_err), m_system(false) {
  Join(c);
}

inline Config::Config(ErrorStream &e, const char *file) :
    m_err(e), m_system(false) {
  Read(file);
}
inline Config::Config(ErrorStream &e, int &argc, char **&argv) :
    m_err(e), m_system(false) {
  Read(argc, argv);
}
inline Config::Config(ErrorStream &e, const char *file, int &argc, char **&argv) :
    m_err(e), m_system(false) {
  Read(file);
  Read(argc, argv);
}

inline void Config::Add(const char *n, bool system) {
  Add(new ConfOption(n, system));
}
inline void Config::Add(const char *n, const char *a1, bool system) {
  Add(new ConfOption(n, a1, system));
}
inline void Config::Add(const char *n, const char *a1, const char *a2, bool system) {
  Add(new ConfOption(n, a1, a2, system));
}

} // namespace Puma

#endif /* PUMA_Config_H */
