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

#include "Puma/OptsParser.h"
#include "Puma/ParserKey.h"
#include <string>
#include <string.h>

namespace Puma {

OptsParser::OptsParser(int argc, char **argv, const Option *opts)
    : m_opts(opts) {
  for (int idx = 0; idx < argc; idx++) {
    m_argv.push_back(argv[idx]);
  }
  m_curarg = 1;
  m_tonext = 0;
  m_number = 0;
}

OptsParser::OptsParser(const std::string &line, const Option *opts)
    : m_opts(opts) {
  tokenize(line, m_argv, " \t\n\r");
  m_curarg = m_tonext = m_number = 0;
}

int OptsParser::getOption() {
  m_number = FINISH;
  m_curarg += m_tonext;

  if (m_curarg < m_argv.size()) {
    bool isLong = false;
    const std::string &arg(m_argv[m_curarg]);
    const Option* curOpt;

    m_tonext = 1;
    m_opt = m_arg = "";

    if (ParserKey::isLongKey(arg) && (arg.length() > ParserKey::getLongKeyLength())) {
      isLong = true;
    } else if ((ParserKey::isKey(arg)) && (arg.length() > ParserKey::getKeyLength())) {
      isLong = false;
    } else {
      m_number = NOOPTION;
      m_arg = arg;
    }

    if (m_number != NOOPTION) {
      // search options in argument string
      curOpt = findOption(arg, isLong);

      // if we found an option go on with processing
      if ((curOpt != NULL) && (curOpt->number != 0)) {
        if (processOption(arg, curOpt, isLong) == false) {
          m_number = NOARG;
        } else {
          m_number = curOpt->number;
        }
      } else {
        //cout << "no option: " << arg << endl;
        m_arg = arg;
        m_number = UNKNOWN;
      }
    }
  }
  return m_number;
}

int OptsParser::getRawOption() {
  m_number = FINISH;
  m_curarg += m_tonext;
  if (m_curarg < m_argv.size()) {
    m_tonext = 1;
    m_opt = m_arg = "";
    m_arg = m_argv[m_curarg];
    m_number = NOOPTION;
  }
  return m_number;
}

const Puma::OptsParser::Option* OptsParser::findOption(const std::string &arg, bool isLong) {
  unsigned int keyLen;
  std::string curName;
  const Option *curOpt = m_opts;
  const Option *selOpt = NULL;
  unsigned int selOptLength = 0;

  // search for long option name in arg
  while (curOpt->number != 0) {
    if ((isLong == true) && (curOpt->longName != NULL)) {
      keyLen = ParserKey::getLongKeyLength();
      curName = curOpt->longName;
    } else if (curOpt->shortName != NULL) {
      keyLen = ParserKey::getKeyLength();
      curName = curOpt->shortName;
    } else {
      curName = "";
      keyLen = 0;
    }
    /* Select option if:
     *    - name is not null AND
     *    - selected option is either null OR shorter than
     *      the currently analyzed option  AND
     *    - the current option receives a parameter OR is exactly as long
     *      as passed argument AND
     *    - the string passed as argument starts at the position after
     *      the option key
     */
    if ((!curName.empty()) && ((selOpt == NULL) || (selOptLength <= curName.length()))
        && ((curOpt->argType != AT_NONE) || ((arg.length() - keyLen) == curName.length())) && (arg.find(curName) == keyLen)) {
      selOpt = curOpt;
      selOptLength = curName.length();
    }
    curOpt++;
  }
  return selOpt;
}

// process option and option argument.
// there can AT_NONE, a AT_MANDATORY or an AT_OPTIONAL option argument.
bool OptsParser::processOption(const std::string &arg, const Option* curOpt, bool isLong) {
  unsigned int len;

  if (isLong == true) {
    len = (ParserKey::getLongKeyLength() + strlen(curOpt->longName));
  } else {
    len = (ParserKey::getKeyLength() + strlen(curOpt->shortName));
  }

  // set current option
  m_opt = arg.substr(0, len);

  // if there should be no argument provided do not proceed
  if (curOpt->argType == AT_NONE) {
    return true;
  }

  // if there are characters behind the end of the option name,
  // treat them as option parameter
  if (arg.length() > len) {
    m_arg = arg.substr(len, arg.length() - len);
  }
  // if ARGV contains at least one more element
  // check if it is an option argument
  else if ((m_curarg + 1) < m_argv.size()) {
    // if the next element of ARGV is NOT an other option treat it as option argument
    // otherwise m_arg will continue containing an empty string
    const std::string &next_argv(m_argv[m_curarg + 1]);
    if (!((ParserKey::isLongKey(next_argv) && next_argv.length() > ParserKey::getLongKeyLength())
        || (ParserKey::isKey(next_argv) && next_argv.length() > ParserKey::getKeyLength()))) {
      m_arg = next_argv;
      ++m_tonext;
    }
  }

  // return false if there should be an argument
  if (m_arg.empty() && curOpt->argType == AT_MANDATORY) {
    return false;
  }

  return true;
}

int OptsParser::getResult() const {
  return m_number;
}

const std::string &OptsParser::getArgument() {
  m_arg = strip(m_arg);
  return m_arg;
}

const std::string &OptsParser::getUnstrippedArgument() const {
  return m_arg;
}

const std::string &OptsParser::getOptionName() const {
  return m_opt;
}

int OptsParser::getCurrentArgNum() const {
  return m_curarg;
}

int OptsParser::getNextArgNum() const {
  return m_curarg + m_tonext;
}

} // namespace Puma
