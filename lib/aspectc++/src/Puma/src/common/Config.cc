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

#include "Puma/Config.h"
#include "Puma/SysCall.h"
#include "Puma/StrCol.h"
#include "Puma/OptsParser.h"
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace Puma {

// print error messages

#define ERROR_CONFIG_FILE \
   m_err << sev_error << "in config file `" << file << "': "

#define ERROR_COMMAND_LINE \
   m_err << sev_error << "at command line: "

#define ERROR_MSG(__mesg) \
   m_err << __mesg << endMessage

#define ERROR__missing(__name) \
   ERROR_COMMAND_LINE; \
   ERROR_MSG ("missing `" << __name << "' for `-A'")

#define ERROR__empty(__what,__name) \
   ERROR_COMMAND_LINE; \
   ERROR_MSG ("no " << __what << " given for `" << __name << "'")

unsigned int Config::OPT_I_HASH = StrCol::hash("-I");
unsigned int Config::OPT_ISYSTEM_HASH = StrCol::hash("--isystem");

OptsParser::Option pumaOpts[] = {
   {Config::PRE_ASSERT, "A", NULL, "Assert a preprocessor predicate", OptsParser::AT_MANDATORY},
   {Config::PRE_DEFINE, "D", NULL, "Define a preprocessor macro", OptsParser::AT_MANDATORY},
   {Config::PRE_UNDEFINE, "U", NULL, "Undefine a preprocessor macro", OptsParser::AT_MANDATORY},
   {Config::PRE_LOCK_MACRO, NULL, "lock-macro", "Define an immutable preprocessor macro", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "inhibit-macro", "Add the name of a macro whose definition will be ignored", OptsParser::AT_MANDATORY},
   {Config::PRE_INCLUDE, "I", NULL, "Add a non-system include directory", OptsParser::AT_MANDATORY},
   {Config::PROJ_PATH, "p", "path", "Add a project source directory", OptsParser::AT_MANDATORY},
   {Config::PROJ_DESTINATION, "d", "dest", "Add a destination folder for modified sources", OptsParser::AT_MANDATORY},
   {Config::SET_SUFFIX, "s", "suffix", "Set the suffix for output files", OptsParser::AT_MANDATORY},
   {Config::SET_EXTENSION, "e", "extension", "Set the extension for source files", OptsParser::AT_MANDATORY},
   {Config::SET_WRITE_PROTECTED, "w", "write-protected", "Add a write protected project directory", OptsParser::AT_MANDATORY},
   {Config::CFG_FILE, NULL, "config", "Load configuration options from a file", OptsParser::AT_MANDATORY},
   {Config::SYS_CFG_FILE, NULL, "system-config", "Load low precedence configuration options from a file", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION, NULL, "save-overwrite", "Overwrite existing files when saving", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "save-rename-old", "Rename existing files when saving", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "save-new-suffix", "Append new suffix on existing files when saving", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "lang-c", "Set input language to C", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "lang-ec++", "Set input language to EC++", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "lang-c++", "Set input language to C++", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "vc", "Enable Visual C++ extensions", OptsParser::AT_NONE},
   {Config::GNU, NULL, "gnu", "Enable GNU extensions", OptsParser::AT_OPTIONAL},
   {Config::SET_OPTION, NULL, "gnu-nested-fct", "Enable GNU nested functions extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-condition-scope", "Enable GNU condition scope extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-param-decl", "Enable GNU parameter declaration extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-fct-decl", "Enable GNU function declaration extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-param-scope", "Enable GNU parameter scope extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-default-args", "Enable GNU default arguments extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-extended-asm", "Enable GNU extended asm declaration extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-extended-expr", "Enable GNU extended expression extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-long-long", "Enable GNU type long long extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-int128", "Enable GNU type __int128 extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-name-scope", "Enable GNU name scope extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-implicit-int", "Enable GNU implicit int extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-fct-attribute", "Enable GNU function attributes extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-if-then-expr", "Enable GNU if-then expression extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-std-hack", "Enable GNU std namespace extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-friend-injection", "Enable GNU friend injection extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "gnu-pic24", "Enable GNU PIC24 and dsPIC extension", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "builtin-type-traits", "Enable built-in type traits", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "c++1x", "Enable C++1X extensions", OptsParser::AT_OPTIONAL},
   {Config::SET_OPTION, NULL, "c++1x-static-assert", "Enable C++1X static assertions", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "skip-bodies-all", "Skip parsing function bodies", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "skip-bodies-tpl", "Skip parsing function bodies of templates", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "skip-bodies-non-prj", "Skip parsing non-project function bodies", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "skip-bodies-non-prim", "Skip parsing function bodies in non-primary files", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "real-instances", "Enable template instantiation", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "pseudo-instances", "Simplified template instantiation scheme (deprecated)", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "inst-fct-bodies", "Enable function template body instantiation", OptsParser::AT_NONE},
   {Config::SET_OPTION_ARG, NULL, "template-depth", "Set the maximum instantiation depth for templates", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION, NULL, "verbose-errors", "Enable verbose error messages", OptsParser::AT_NONE},
   {Config::SET_OPTION, NULL, "match-expr", "Enable parsing match expressions", OptsParser::AT_NONE},
   {Config::SET_OPTION_ARG, NULL, "import-handler", "Set handler for Visual C++ #import directives", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "include", "Add file to be included at the beginning of every source file", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "size-type", "Set type for size_t", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "ptrdiff-type", "Set type for ptrdiff_t", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "target", "Set target triple, which determines sizes and alignments for built-in types", OptsParser::AT_MANDATORY},
   {Config::SET_OPTION_ARG, NULL, "isystem", "Add a system include directory", OptsParser::AT_MANDATORY},
   {0, 0, 0, 0, OptsParser::AT_NONE}
};

// print the options
void Config::PrintOptions(std::ostream& os) const {
  // first print short options with description
  for (int i = 0; pumaOpts[i].number > 0; i++) {
    if (pumaOpts[i].shortName && pumaOpts[i].desc && strlen(pumaOpts[i].desc)) {
      PrintOption(pumaOpts[i], os);
    }
  }
  // then print long options with description
  for (int i = 0; pumaOpts[i].number > 0; i++) {
    if (!pumaOpts[i].shortName && pumaOpts[i].desc && strlen(pumaOpts[i].desc)) {
      PrintOption(pumaOpts[i], os);
    }
  }
  // then print options without description
  for (int i = 0; pumaOpts[i].number > 0; i++) {
    if (!pumaOpts[i].desc || strlen(pumaOpts[i].desc) == 0) {
      PrintOption(pumaOpts[i], os);
    }
  }
}

// print a single option
void Config::PrintOption(OptsParser::Option& o, std::ostream& os) const {
  if (o.shortName) {
    os << "  -" << o.shortName;
  }
  if (o.longName) {
    if (o.shortName) {
      os << "|";
    } else {
      os << "  ";
    }
    os << "--" << o.longName;
  }
  if (o.argType == OptsParser::AT_MANDATORY) {
    os << " <ARG>";
  }
  if (o.argType == OptsParser::AT_OPTIONAL) {
    os << " [ARG]";
  }
  if (o.desc) {
    os << "\033[35G" << o.desc;
  }
  os << std::endl;
}

Config::~Config() {
  OptionList::iterator idx = m_sysopts.begin();
  for (; idx != m_sysopts.end(); ++idx) {
    delete *idx;
  }
  for (idx = m_useropts.begin(); idx != m_useropts.end(); ++idx) {
    delete *idx;
  }
  for (idx = m_sysincls.begin(); idx != m_sysincls.end(); ++idx) {
    delete *idx;
  }
}

void Config::Add(const ConfOption *o) {
  m_optnamemap[o->Hash()] = o;
  if (o->isSystem()) {
    if (o->Hash() == OPT_I_HASH || o->Hash() == OPT_ISYSTEM_HASH)
      m_sysincls.push_back(o);
    else
      m_sysopts.push_back(o);
  } else {
    m_useropts.push_back(o);
  }
}

unsigned Config::Options() const {
  return m_sysopts.size() + m_useropts.size() + m_sysincls.size();
}

const ConfOption *Config::Option(unsigned i) const {
  OptionList::size_type s = m_sysopts.size();
  if (i < s)
    return m_sysopts[i];
  i -= s;
  s = m_useropts.size();
  if (i < s)
    return m_useropts[i];
  i -= s;
  s = m_sysincls.size();
  if (i < s)
    return m_sysincls[i];
  return 0;
}

void Config::Read(const char *file) {
  // file not given, read from $PUMA_CONFIG or /etc/puma.config
  bool default_loc = false;
  if (!file)
    file = getenv("PUMA_CONFIG");
  if (!file) {
    file = "/etc/puma.config"; // default location
    default_loc = true;
  }

  std::ifstream in(file);
  if (!in) {
    if (!default_loc) {
      ERROR_MSG("cannot open config file `" << file << "'");
    }
    return;
  }

  std::ostringstream optstr;
  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    // skip comments and empty lines
    if (!line.empty() && line[0] != '#') {
      optstr << " " << StrCol::replaceEnvironmentVariables(line);
    }
  }
  in.close();

  OptsParser optp(optstr.str(), pumaOpts);

  // process the given options
  while (optp.getOption() != OptsParser::FINISH) {
    Process(optp, true);
  }
}

// read options from command line
void Config::Read(int &argc, char **&argv) {
  OptsParser optp(argc, argv, pumaOpts);
  char** newargv = new char*[argc];
  int newargc = 1;

  // process the given options
  while (optp.getOption() != OptsParser::FINISH) {
    if (Process(optp) == false) {
      // this is not processed by PUMA - copy
      newargv[newargc++] = argv[optp.getCurrentArgNum()];
    }
  }

  // make argv point to new argv and set argc to value of new argc
  argc = newargc;
  for (int i = 1; i < argc; i++)
    argv[i] = newargv[i];
  delete[] newargv;
}

// check if a config file is given on command line
bool Config::CustomConfigFile(int argc, char **argv) {
  OptsParser optp(argc, argv, pumaOpts);
  while (optp.getOption() != OptsParser::FINISH)
    if (optp.getResult() == CFG_FILE)
      return true;
  return false;
}

// check if a system config file is given on command line
// for lower priority options
bool Config::CustomSystemConfigFile(int argc, char **argv) {
  OptsParser optp(argc, argv, pumaOpts);
  while (optp.getOption() != OptsParser::FINISH)
    if (optp.getResult() == SYS_CFG_FILE)
      return true;
  return false;
}

bool Config::Process(OptsParser &parser, bool unescape) {
  bool res = true;
  std::string arg = parser.getArgument();
  if (!arg.empty() && unescape)
    arg = Unescape(arg);

  switch (parser.getResult()) {
    case PRE_ASSERT:
      ProcessAssert(arg);
      break;
    case PRE_DEFINE:
      ProcessDefine(arg);
      break;
    case PRE_LOCK_MACRO:
      ProcessLockMacro(arg);
      break;
    case PRE_UNDEFINE:
      ProcessUndefine(arg);
      break;
    case PRE_INCLUDE:
      ProcessPathArgument("-I", arg);
      break;
    case PROJ_PATH:
      ProcessPathArgument("-p", arg);
      break;
    case PROJ_DESTINATION:
      ProcessPathArgument("-d", arg);
      break;
    case CFG_FILE:
      ProcessConfigFile("--config", arg);
      break;
    case SYS_CFG_FILE:
      ProcessSystemConfigFile("--system-config", arg);
      break;
    case SET_SUFFIX:
      ProcessArgument("-s", arg);
      break;
    case SET_EXTENSION:
      ProcessArgument("-e", arg);
      break;
    case SET_WRITE_PROTECTED:
      ProcessArgument("-w", arg);
      break;
    case GNU:
      ProcessOptionalArgument("--gnu", arg);
      break;
    case SET_OPTION:
      Add(parser.getOptionName().c_str(), m_system);
      break;
    case SET_OPTION_ARG:
      Add(parser.getOptionName().c_str(), arg.c_str(), m_system);
      break;
    case OptsParser::UNKNOWN:
    case OptsParser::NOOPTION:
      res = false;
      break;
    default:
    case OptsParser::ERR:
      break;
  }
  return res;
}

bool Config::ProcessConfigFile(const std::string &opt, const std::string &arg) {
  bool res = false;

  if (arg.empty() == false) {
    Read(arg.c_str());
    res = true;
  } else {
    ERROR__empty("file name", opt.c_str());
  }

  return res;
}

bool Config::ProcessSystemConfigFile(const std::string &opt, const std::string &arg) {
  bool res = false;
  bool system_old = m_system;

  m_system = true;

  if (arg.empty() == false) {
    Read(arg.c_str());
    res = true;
  } else {
    ERROR__empty("file name", opt.c_str());
  }

  m_system = system_old;

  return res;
}

bool Config::ProcessArgument(const std::string &opt, const std::string &arg) {
  bool res = false;

  if (arg.empty() == false) {
    Add(opt.c_str(), arg.c_str(), m_system);
    res = true;
  } else {
    ERROR__empty("argument", opt.c_str());
  }
  return res;
}

bool Config::ProcessOptionalArgument(const std::string &opt, const std::string &arg) {
  if (arg.empty()) {
    Add(opt.c_str(), m_system);
  } else {
    Add(opt.c_str(), arg.c_str(), m_system);
  }
  return true;
}

bool Config::ProcessPathArgument(const std::string &opt, const std::string &arg) {
  bool res = false;

  if (arg.empty() == false) {
#if defined (WIN32)
    char *tmp = StrCol::dup(arg.c_str());
    if (tmp != 0) {
      SysCall::MakeUnixPath(tmp);
      Add(opt.c_str(), tmp, m_system);
      delete[] tmp;
    }
#else
    Add(opt.c_str(), arg.c_str(), m_system);
    res = true;
#endif
  } else {
    ERROR__empty("argument", opt.c_str());
  }

  return res;
}

bool Config::ProcessUndefine(const std::string &arg) {
  bool res = false;

  if (Valid(arg.c_str())) {
    Add("-U", arg.c_str(), m_system);
    res = true;
  } else {
    ERROR_MSG("invalid name for -U");
  }

  return res;
}

bool Config::ProcessDefine(const std::string &arg) {
  bool res = false;
  std::string::size_type pos;
  std::string name, body;

  if ((pos = arg.find_first_of('=', 0)) != std::string::npos) {
    body = arg.substr(pos + 1, arg.length() - pos);
    name = arg.substr(0, pos);
  } else {
    body = "1";
    name = arg;
  }

  if (Valid(name.c_str())) {
    Add("-D", name.c_str(), body.c_str(), m_system);
    res = true;
  } else {
    ERROR_MSG("invalid name for -D");
  }

  return res;
}

bool Config::ProcessLockMacro(const std::string &arg) {
  bool res = false;
  std::string::size_type pos;
  std::string name, body;

  if ((pos = arg.find_first_of('=', 0)) != std::string::npos) {
    body = arg.substr(pos + 1, arg.length() - pos);
    name = arg.substr(0, pos);
  } else {
    body = "1";
    name = arg;
  }

  if (Valid(name.c_str())) {
    Add("--lock-macro", name.c_str(), body.c_str(), m_system);
    res = true;
  } else {
    ERROR_MSG("invalid name for --lock-macro");
  }

  return res;
}

bool Config::ProcessAssert(const std::string &arg) {
  bool res = false;
  std::string::size_type pos, end;
  std::string name, body;

  if ((pos = arg.find_first_of('(', 0)) != std::string::npos) {
    if ((end = arg.find_first_of(')', pos)) != std::string::npos) {
      name = arg.substr(0, pos);
      body = arg.substr(pos + 1, end - pos - 1);
      if (StrCol::onlySpaces(body.c_str()) == false) {
        if (Valid(name.c_str())) {
          Add("-A", name.c_str(), body.c_str(), m_system);
          res = true;
        } else {
          ERROR_MSG("invalid name for -A");
        }
      } else {
        ERROR__empty("answer text", "-A");
      }
    } else {
      ERROR__missing(")");
    }
  } else {
    ERROR__missing("(");
  }

  return res;
}

bool Config::Valid(const char *name, const char *file) const {
  if (name) {
    const char *reject = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$0123456789";
    if (strlen(name) && !strcspn(name, reject) && !(*name >= '0' && *name <= '9'))
      return true;
  }
  if (file)
    ERROR_CONFIG_FILE;
  else
    ERROR_COMMAND_LINE;
  ERROR_MSG("`" << name << "' is not a valid C identifier");
  return false;
}

void Config::Join(const Config &c) {
  unsigned num = c.Options();
  for (unsigned i = 0; i < num; i++)
    Add(c.Option(i));
}

const ConfOption *Config::Option(const char *name) const {
  const ConfOption* co = 0;
  if (name) {
    OptionMap::const_iterator idx = m_optnamemap.find(StrCol::hash(name));
    if (idx != m_optnamemap.end()) {
      co = idx->second;
    }
  }
  return co;
}

std::string Config::Unescape(const std::string& arg) {
  // Replacement rules:
  // 1. '\' '"' => '"'
  // 2. '\' '\' => '\'
  // 3. '"'     => ''
  std::ostringstream res;
  bool escaped = false;
  for (std::string::size_type i = 0; i < arg.length(); i++) {
    char c = arg[i];
    if (c == '"') {
      if (escaped) {
        escaped = false;
        res << c;
      }
    } else if (c == '\\') {
      if (escaped) {
        escaped = false;
        res << c;
      } else {
        escaped = true;
      }
    } else {
      if (escaped) {
        escaped = false;
        res << "\\";
      }
      res << c;
    }
  }
  return res.str();
}

void Config::Remove(const ConfOption *o) {
  if (o->isSystem()) {
    if (o->Hash() == OPT_I_HASH || o->Hash() == OPT_ISYSTEM_HASH)
      Remove(o, m_sysincls);
    else
      Remove(o, m_sysopts);
  } else {
    Remove(o, m_useropts);
  }

  OptionMap::iterator idx = m_optnamemap.find(o->Hash());
  if (idx != m_optnamemap.end() && idx->second == o) {
    m_optnamemap.erase(idx);
  }

  delete o;
}

void Config::Remove(const ConfOption *o, OptionList& options) {
  OptionList::iterator idx = std::find(options.begin(), options.end(), o);
  if (idx != options.end()) {
    options.erase(idx);
  }
}

} // namespace Puma
