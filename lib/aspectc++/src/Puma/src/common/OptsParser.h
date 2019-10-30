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

#ifndef PUMA_OptsParser_H
#define PUMA_OptsParser_H

/** \file
 * Command line options parser. */

#include "Puma/SimpleParser.h"
#include <vector>
#include <string>

namespace Puma {

/** \class OptsParser OptsParser.h Puma/OptsParser.h
 * The options parser searches command line parameters for options and
 * handles their option arguments, if applicable.
 * \ingroup common */
class OptsParser : public SimpleParser {
  /** Vector holding the command line parameters.
   * This vector is filled in the constructor */
  std::vector<std::string> m_argv;

  /** Position of current parameter analyzed by the
   * options parser. */
  unsigned int m_curarg;

  /** The increment added to m_curarg used to get to the next
   * entry of m_argv which shall be analyzed. */
  unsigned int m_tonext;

  /** %Option number of currently processed element of m_argv. */
  int m_number;

  /** Name of current option. */
  std::string m_opt;

  /** Value of current option argument. */
  std::string m_arg;

public:
  /** Return codes for OptsParser::getOption() and OptsParser::getArgument(). */
  enum {
    NOARG = -4, /**< %Option has no argument. */
    ERR = -3, /**< Error when parsing option. */
    NOOPTION = -2, /**< Command line parameter is not an option. */
    UNKNOWN = -1, /**< Parameter is an unknown option. */
    FINISH = 0 /**< All options are handled. */
  };

  /** Type of argument associated to an option. */
  enum ArgType {
    AT_NONE = 0, /**< %Option receives no argument. */
    AT_MANDATORY = 1, /**< %Option requires an argument. */
    AT_OPTIONAL = 2 /**< %Option has optionally an argument. */
  };

  /** Type for specifying an array of options recognized by the %Option %Parser.
   * An array of this type has to be passed to the constructor OptsParser::OptsParser() */
  struct Option {
    int number; /**< Number associated with this option. */
    const char *shortName; /**< Short name of this option. Should ideally not contain more than one character */
    const char *longName; /**< Long name of this option. */
    const char *desc; /**< Description of this option. */
    ArgType argType; /**< Type of argument for this option. */
  };

private:
  /** Pointer to array of available options. This value is set to the value of the 
   * parameter passed to the constructor OptsParser::OptsParser() */
  const Option *m_opts;

public:
  /** Constructor.
   * \param argc Number of parameters to be analyzed.
   * \param argv Parameters to be analyzed.
   * \param opts Pointer to array of options which shall be recognized by the options parser. */
  OptsParser(int argc, char **argv, const Option *opts);

  /** Constructor.
   * \param line Reference to string with parameters to analyze.
   *             The parameters are extracted from the string by splitting it up
   *             into tokens at:
   *               - space character
   *               - tabs 
   *               - newline
   *               - carriage return
   * \param opts Pointer to array of options which shall be
   *             recognized by the options parser */
  OptsParser(const std::string &line, const Option *opts);

  /** Get next parameter without modification. 
   * No option parsing is done.
   * \return OptsParser::NOOPTION or OptsParser::FINISH. */
  int getRawOption();

  /** Get next option.
   * This function processes the next parameter passed to the constructor 
   * OptsParser::OptsParser(). 
   * That is, search for options and eventually process
   * option and option argument.
   * \return Number of options recognized by the options parser or
   *         OptsParser::NOARG, OptsParser::ERR, OptsParser::NOOPTION,
   *         OptsParser::UNKNOWN or OptsParser::FINISH. */
  int getOption();

  /** Get result of last processed parameter.
   * The number returned by this function is identical to the number
   * returned by the last call of OptsParser::getOption.
   * \return Number of options recognized by the options parser or
   *         OptsParser::NOARG, OptsParser::ERR, OptsParser::NOOPTION,
   *         OptsParser::UNKNOWN or OptsParser::FINISH. */
  int getResult() const;

  /** Get option argument of last processed parameter.
   * This function returns the argument of the option processed during
   * OptsParser::getOption. If the option does not provide any argument
   * the string will be empty.
   * If OptsParser::getOption or respectively OptsParser::getResult returns with
   * OptsParser::NOOPTION this function (OptsParser::getArgument) returns the last processed parameter,
   * which is no option, of course.
   * \return Argument of last processed parameter */
  const std::string &getArgument();

  /** Get option argument of last processed parameter without stripping.
   * This function returns the argument of the option processed during
   * OptsParser::getOption. If the option does not provide any argument
   * the string will be empty.
   * If OptsParser::getOption or respectively OptsParser::getResult returns with
   * OptsParser::NOOPTION this function (OptsParser::getArgument) returns the last processed parameter,
   * which is no option, of course.
   * \return Argument of last processed parameter */
  const std::string &getUnstrippedArgument() const;

  /** Get option name of last processed parameter.
   * If the last processed parameter did not contain an option the string
   * returned by this function is empty.
   * \return Name of last processed parameter */
  const std::string &getOptionName() const;

  /** Get position of current argument.
   * \return Position of currently processed parameter previously passed to OptsParser::OptsParser() */
  int getCurrentArgNum() const;

  /** Get position of next argument.
   * \return Position of next parameter which will be processed by the options parser */
  int getNextArgNum() const;

private:
  /** Search for an option in m_argv at position m_curarg.
   * \param arg Currently processed parameter */
  const Option* findOption(const std::string &arg, bool isLong);

  /** Process an option found by OptsParser::findOption(). */
  bool processOption(const std::string &arg, const Option* curOpt, bool isLong);
};

} // namespace Puma

#endif /* PUMA_OptsParser_H */
