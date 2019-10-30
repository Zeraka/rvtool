// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#include "PointCutExprParser.h"
#include "PointCutExpr.h"
#include "PointCutSearcher.h"
#include "TransformInfo.h"

#include <exception>
using namespace std;

// TODO: exceptions cause memory leak; objects on the heap are not freed
struct PCEParserException : exception {
  string _what;
  PCEParserException(const string &s) : _what(s) {}
  ~PCEParserException() throw() {}
  const char* what() const throw() { return _what.c_str (); }
};

enum PctTokType {
  PCTTOK_EXEC = 1,
  PCTTOK_CALL,
  PCTTOK_BUILTIN,
  PCTTOK_GET,
  PCTTOK_SET,
  PCTTOK_REF,
  PCTTOK_ALIAS,
  PCTTOK_CONSTRUCTION,
  PCTTOK_DESTRUCTION,
  PCTTOK_CLASSES,
  PCTTOK_WITHIN,
  PCTTOK_MEMBER,
  PCTTOK_BASE,
  PCTTOK_DERIVED,
  PCTTOK_THAT,
  PCTTOK_TARGET,
  PCTTOK_ARGS,
  PCTTOK_RESULT,
  PCTTOK_CFLOW,
  PCTTOK_MATCHEXPR,
  PCTTOK_OPEN,
  PCTTOK_CLOSE,
  PCTTOK_OR,
  PCTTOK_AND,
  PCTTOK_NOT,
  PCTTOK_COMMA,
  PCTTOK_COLON_COLON,
  PCTTOK_ID
};

PointCutExprParser::PointCutExprParser( ACConfig &config ) : config_(config) {
  lexertl::rules rules;
  rules.push ("execution", PCTTOK_EXEC);
  rules.push ("call", PCTTOK_CALL);
  if( config.data_joinpoints() ) {
    rules.push( "get", PCTTOK_GET );
    rules.push( "set", PCTTOK_SET );
    rules.push( "ref", PCTTOK_REF );
    rules.push( "alias", PCTTOK_ALIAS );
  }
  if( config.builtin_operators() ) {
    rules.push( "builtin", PCTTOK_BUILTIN );
  }
  rules.push ("construction", PCTTOK_CONSTRUCTION);
  rules.push ("destruction", PCTTOK_DESTRUCTION);
  rules.push ("classes", PCTTOK_CLASSES);
  rules.push ("within", PCTTOK_WITHIN);
  rules.push( "member", PCTTOK_MEMBER );
  rules.push ("base", PCTTOK_BASE);
  rules.push ("derived", PCTTOK_DERIVED);
  rules.push ("that", PCTTOK_THAT);
  rules.push ("target", PCTTOK_TARGET);
  rules.push ("args", PCTTOK_ARGS);
  rules.push ("result", PCTTOK_RESULT);
  rules.push ("cflow", PCTTOK_CFLOW);
  rules.push ("\\\"([^\\\\\\\"]|\\\\.)*\\\"", PCTTOK_MATCHEXPR);
  rules.push ("\\(", PCTTOK_OPEN);
  rules.push ("\\)", PCTTOK_CLOSE);
  rules.push ("\\|\\|", PCTTOK_OR);
  rules.push ("&&", PCTTOK_AND);
  rules.push ("!", PCTTOK_NOT);
  rules.push (",", PCTTOK_COMMA);
  rules.push ("::", PCTTOK_COLON_COLON);
  rules.insert_macro("Hex", "[0-9a-fA-F]");
  rules.insert_macro ("UniversalChar", "\\\\(u{Hex}{4}|U{Hex}{8})");
  rules.insert_macro ("Alpha", "[a-zA-Z_\\$]|{UniversalChar}");
  rules.push ("{Alpha}({Alpha}|\\d)*", PCTTOK_ID);

  lexertl::generator::build (rules, state_machine_);
}

PointCutExpr *PointCutExprParser::parse (const string &input,
    PointCutSearcher &searcher) {
  string::const_iterator iter = input.begin ();
  string::const_iterator end = input.end ();
  lexertl::smatch results (iter, end);

  lexertl::lookup (state_machine_, results);
  PointCutExpr *result = parse_or_expr (results, searcher);
  if (!result)
    throw PCEParserException("Pointcut expression parser failed");
  if (results.id != 0)
    throw PCEParserException("Input after end of pointcut expression");
  return result;
}

PointCutExpr *PointCutExprParser::parse_or_expr (lexertl::smatch &results,
    PointCutSearcher &searcher) {

  PointCutExpr *result = parse_and_expr (results, searcher);
  while (results.id == PCTTOK_OR) {
    lexertl::lookup (state_machine_, results);
    PointCutExpr *r = parse_and_expr (results, searcher);
    if (!r) return 0;
    result = new PCE_Or(result, r);
  }
  return result;
}

PointCutExpr *PointCutExprParser::parse_and_expr (lexertl::smatch &results,
    PointCutSearcher &searcher) {

  PointCutExpr *result = parse_unary_expr (results, searcher);
  while (results.id == PCTTOK_AND) {
    lexertl::lookup (state_machine_, results);
    PointCutExpr *r = parse_unary_expr (results, searcher);
    if (!r) return 0;
    result = new PCE_And(result, r);
  }
  return result;
}

PointCutExpr *PointCutExprParser::parse_unary_expr (lexertl::smatch &results,
    PointCutSearcher &searcher) {

  PointCutExpr *result = parse_primary_expr (results, searcher);
  if (result) return result;

  PointCutExpr *r = 0;
  switch (results.id) {
  case PCTTOK_NOT:
    lexertl::lookup (state_machine_, results);
    r = parse_unary_expr (results, searcher);
    if (!r) return 0;
    result = new PCE_Not (r);
    break;
  case PCTTOK_EXEC:
  case PCTTOK_CALL:
  case PCTTOK_BUILTIN:
  case PCTTOK_GET:
  case PCTTOK_SET:
  case PCTTOK_REF:
  case PCTTOK_ALIAS:
  case PCTTOK_CONSTRUCTION:
  case PCTTOK_DESTRUCTION:
  case PCTTOK_BASE:
  case PCTTOK_DERIVED:
  case PCTTOK_THAT:
  case PCTTOK_TARGET:
  case PCTTOK_CLASSES:
  case PCTTOK_WITHIN:
  case PCTTOK_MEMBER:
  case PCTTOK_ARGS:
  case PCTTOK_RESULT:
  case PCTTOK_CFLOW:
    unsigned long tt = results.id;
    lexertl::lookup (state_machine_, results);
    if (results.id != PCTTOK_OPEN)
      return 0;
    lexertl::lookup (state_machine_, results);
    vector<PointCutExpr*> arguments;
    while (true) {
      PointCutExpr *r = parse_or_expr (results, searcher);
      if (!r) break;
      arguments.push_back (r);
      if (results.id != PCTTOK_COMMA)
        break;
      lexertl::lookup (state_machine_, results);
    }
    if (tt != PCTTOK_ARGS && arguments.size () != 1) return 0;
    if (tt == PCTTOK_CALL)
      result = new PCE_Call (arguments[0]);
    else if (tt == PCTTOK_BUILTIN)
      result = new PCE_Builtin(arguments[0]);
    else if( tt == PCTTOK_GET )
      result = new PCE_Get( arguments[0] );
    else if( tt == PCTTOK_SET )
      result = new PCE_Set( arguments[0] );
    else if( tt == PCTTOK_REF )
      result = new PCE_Ref( arguments[0] );
    else if( tt == PCTTOK_ALIAS )
      result = new PCE_Alias( arguments[0] );
    else if (tt == PCTTOK_EXEC)
      result = new PCE_Execution (arguments[0]);
    else if (tt == PCTTOK_CONSTRUCTION)
      result = new PCE_Construction (arguments[0]);
    else if (tt == PCTTOK_DESTRUCTION)
      result = new PCE_Destruction (arguments[0]);
    else if (tt == PCTTOK_BASE)
      result = new PCE_Base (arguments[0]);
    else if (tt == PCTTOK_DERIVED)
      result = new PCE_Derived (arguments[0]);
    else if (tt == PCTTOK_THAT)
      result = new PCE_That (arguments[0]);
    else if (tt == PCTTOK_TARGET)
      result = new PCE_Target (arguments[0]);
    else if (tt == PCTTOK_CLASSES)
      result = new PCE_Classes (arguments[0]);
    else if (tt == PCTTOK_WITHIN)
      result = new PCE_Within (arguments[0]);
    else if( tt == PCTTOK_MEMBER )
      result = new PCE_Member( arguments[0] );
    else if (tt == PCTTOK_RESULT)
      result = new PCE_Result (arguments[0]);
    else if (tt == PCTTOK_CFLOW)
      result = new PCE_CFlow (arguments[0]);
    else if (tt == PCTTOK_ARGS) {
      PCE_Args *pce_args = new PCE_Args;
      for (vector<PointCutExpr*>::iterator i = arguments.begin ();
          i != arguments.end(); ++i)
        pce_args->add_arg(*i);
      result = pce_args;
    }

    if (results.id != PCTTOK_CLOSE)
      throw PCEParserException("Closing bracket missing");
    lexertl::lookup (state_machine_, results);
    break;
  }
  return result;
}

PointCutExpr *PointCutExprParser::parse_primary_expr (lexertl::smatch &results,
    PointCutSearcher &searcher) {

  PointCutExpr *result = 0;
  string match_expr;
  switch (results.id) {
  case PCTTOK_MATCHEXPR:
    do {
      match_expr += string (results.start+1, results.end-1);
      lexertl::lookup (state_machine_, results);
    } while (results.id == PCTTOK_MATCHEXPR);
    result = new PCE_Match(match_expr);
    if (!((PCE_Match*)result)->parse ())
      throw PCEParserException("Invalid match expression");
    break;
  case PCTTOK_OPEN:
    lexertl::lookup (state_machine_, results);
    result = parse_or_expr (results, searcher);
    if (!result) return 0;
    if (results.id != PCTTOK_CLOSE)
      throw PCEParserException("Closing bracket missing");
    lexertl::lookup (state_machine_, results);
    break;
  case PCTTOK_ID:
  case PCTTOK_COLON_COLON:
    vector<string> qual_name;
    bool root_qualified = false;
    if (results.id == PCTTOK_COLON_COLON) {
      root_qualified = true;
      lexertl::lookup (state_machine_, results);
    }
    if (results.id != PCTTOK_ID)
      return 0;
    string id (results.start, results.end);
    qual_name.push_back (id);
    lexertl::lookup (state_machine_, results);
    while (results.id == PCTTOK_COLON_COLON) {
      lexertl::lookup (state_machine_, results);
      if (results.id != PCTTOK_ID)
        return 0;
      id = string(results.start, results.end);
      qual_name.push_back (id);
      lexertl::lookup (state_machine_, results);
    }
    if (results.id == PCTTOK_OPEN) {
      lexertl::lookup (state_machine_, results);
      ACM_Pointcut *pct_func = searcher.lookup_pct_func(root_qualified, qual_name);
      ACM_Attribute *pct_attr = searcher.lookup_pct_attr(root_qualified, qual_name);

      if(pct_func && pct_attr && config_.attributes()) {
        string msg = "Declarations of named pointcut and attribute'";
        msg += id;
        msg += "' are ambigous! ";

        throw PCEParserException(msg);
      }
      else if (!pct_func && (!pct_attr || !config_.attributes())) {
        string msg = "Named pointcut or attribute '";
        msg += id;
        msg += "' not found";
#ifdef FRONTEND_CLANG
        if(id == "builtin" || id == "get" || id == "set" || id == "ref" || id == "alias") {
          msg += ". If you meant the ";
          msg += id;
          msg += " pointcut function, then enable it by adding the '";
          if(id == "builtin") {
            msg += "--builtin_operators";
          }
          else {
            msg += "--data_joinpoints";
          }
          msg += "' command-line argument";
        }
#endif // FRONTEND_CLANG
        throw PCEParserException(msg);
      }
#ifdef FRONTEND_CLANG
      else if(config_.warn_deprecated() &&
          (id == "builtin" || id == "get" || id == "set" || id == "ref" || id == "alias")
      ) {
        config_.project().err() << Puma::sev_warning << "named pointcuts with one of the "
            "names 'alias', 'builtin', 'get', 'ref' or 'set' are deprecated as they "
            "name new predefined pointcut functions. These new poincut functions are "
            "currently disabled by default but this will change with one of the next "
            "releases." << Puma::endMessage;
      }
#endif // FRONTEND_CLANG

      if(pct_attr && config_.attributes()) {
        result = new PCE_CXX11Attr(pct_attr);
        lexertl::lookup (state_machine_, results); // consume ')'
      }
      else {
        PCE_Named *call = new PCE_Named(pct_func); // just needed for the right type
        bool first = true;
        while (results.id != 0 && results.id != PCTTOK_CLOSE) {
          if (!first) {
            if (results.id != PCTTOK_COMMA)
              throw PCEParserException("Invalid argument list, comma expected");
                lexertl::lookup (state_machine_, results);
          }
          if (results.id != PCTTOK_ID)
            throw PCEParserException("Invalid argument list, argument name expected");
          string varname (results.start, results.end);
          call->params().push_back(varname);
          first = false;
          lexertl::lookup (state_machine_, results);
        }
        if (results.id == 0)
          return 0;
        lexertl::lookup (state_machine_, results); // consume ')'
        result = call;
      }
    }
    else {
      result = new PCE_ContextVar(id);
    }
    break;
  }
  return result;
}
