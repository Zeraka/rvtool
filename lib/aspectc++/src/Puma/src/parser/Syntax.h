// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __Syntax_h__
#define __Syntax_h__

/** \file
 *  Basic syntactic analysis component. */

/** \page syntax Syntactic Analysis
 */

#include "Puma/Unit.h"
#include "Puma/Token.h"
#include "Puma/Builder.h"
#include "Puma/ErrorSink.h"
#include "Puma/TokenProvider.h"

#include <bitset>

namespace Puma {

class Semantic;
class Config;
class CTree;


/** \class Syntax Syntax.h Puma/Syntax.h
 *  Syntactic analysis base class. 
 *
 *  Implements the top-down parsing algorithm (recursive descend parser). 
 *  To be derived to implement parsers for specific grammars. Provides 
 *  infinite look-ahead. 
 *
 *  This class uses a tree builder object (see Builder) to create the 
 *  syntax tree, and a semantic analysis object (see Semantic) to perform
 *  required semantic analyses of the parsed code. 
 *
 *  The parse process is started by calling Syntax::run() with a token
 *  provider as argument. Using the token provider this method reads the 
 *  first core language token from the input source code and tries to parse 
 *  it by applying the top grammar rule. 
 *
 *  \code return parse(&Puma::Syntax::trans_unit) ? builder().Top() : (Puma::CTree*)0; \endcode
 *
 *  The top grammar rule has to be provided by reimplementing method 
 *  Syntax::trans_unit(). It may call sub-rules according to the implemented 
 *  language-specific grammar. Example:
 *
 *  \code 
 * Puma::CTree *MySyntax::trans_unit() {
 *   return parse(&MySyntax::block_seq) ? builder().block_seq() : (Puma::CTree*)0;
 * }
 *  \endcode
 *
 *  For context-sensitive grammars it may be necessary in the rules of 
 *  the grammar to perform first semantic analyses of the parsed code
 *  (to differentiate ambigous syntactic constructs, resolve names, 
 *  detect errors, and so one). Example:
 *
 *  \code 
 * Puma::CTree *MySyntax::block() {
 *   // '{' instruction instruction ... '}'
 *   if (parse(TOK_OPEN_CURLY)) {             // parse '{'
 *     semantic().enter_block();              // enter block scope
 *     seq(&MySyntax::instruction);           // parse sequence of instructions
 *     semantic().leave_block();              // leave block scope
 *     if (parse(TOK_CLOSE_CURLY)) {          // parse '}'
 *       return builder().block();            // build syntax tree for the block
 *     }
 *   }
 *   return (CTree*)0;                        // rule failed
 * }
 *  \endcode
 *
 *  If a rule could be parsed successfully the tree builder is used to 
 *  create a CTree based syntax tree (fragment) for the parsed rule. Failing
 *  grammar rules shall return NULL. The result of the top grammar rule is 
 *  the root node of the abstract syntax tree for the whole input source code. */
class Syntax {
  Token *_problem_token;
  bool _have_error;
  bool _verbose_errors;

  Builder &_builder;
  Semantic &_semantic;

public:
  /** Token provider for getting the tokens to parse. */
  TokenProvider *token_provider;

  /** Interface for aspects that affect the syntax and parsing process */
  pointcut parse_fct()  = "bool Puma::%::%::parse(%)";
  pointcut check_fct()  = "bool Puma::%::%::check(%)";
  pointcut in_syntax()  = within(member(derived("Puma::Syntax")));
  pointcut rule_exec()  = execution(parse_fct()) && in_syntax();
  pointcut rule_call()  = call(parse_fct()) && in_syntax() && !within("Puma::%::...::%");
  pointcut rule_check() = execution(check_fct()) && in_syntax();

public: 
  /** \class State Syntax.h Puma/Syntax.h
   *  Parser state, the current position in the token stream. */
  struct State : public TokenProvider::State {
    /** Constructor. */
    State () {}
    /** Constructor. */
    State (int) {}
    /** Copy constructor.
     *  \param s The parser state to copy. */
    State (const TokenProvider::State &s) : TokenProvider::State (s) {}
  };

protected:
  /** Constructor.
   *  \param b The syntax tree builder.
   *  \param s The semantic analysis object. */ 
  Syntax (Builder &b, Semantic &s);
  /** Destructor. */
  virtual ~Syntax () {}

public:
  /** Start the parse process.
   *  \param tp The token provider from where to get the tokens to parse. 
   *  \return The resulting syntax tree. */
  CTree *run (TokenProvider &tp);
  /** Start the parse process at a specific grammar rule.
   *  \param tp The token provider from where to get the tokens to parse. 
   *  \param rule The grammar rule where to start. 
   *  \return The resulting syntax tree. */
  template <class T> CTree *run (TokenProvider &tp, bool (T::*rule)());
  /** Configure the syntactic analysis object. 
   *  \param c The configuration object. */
  virtual void configure (Config &c);
  /** Get the token provider from which the parsed tokens are read. */
  TokenProvider *provider () const { return token_provider; }
  /** Get the last token that could not be parsed. */
  Token *problem () const;
  /** Check if errors occured during the parse process. */
  bool error () const;
  /** Look-ahead n core language tokens and check if 
   *  the n-th token has the given type. 
   *  \param token_type The type of the n-th token. 
   *  \param n The number of tokens to look-ahead. 
   *  \return True if the n-th token has the given type. */
  bool look_ahead (int token_type, unsigned n = 1);
  /** Look-ahead n core language tokens and check if 
   *  the n-th token has one of the given types. 
   *  \param token_types The possible types of the n-th token. 
   *  \param n The number of tokens to look-ahead. 
   *  \return True if the n-th token has one of the given types. */
  bool look_ahead (int* token_types, unsigned n = 1);
  /** Look-ahead one core language token.
   *  \param n The number of tokens to look-ahead.
   *  \return The type of the next core language token. */
  int look_ahead (unsigned n = 1);
  /** Consume all tokens until the next core language token. */
  bool consume ();

  typedef std::bitset<TOK_NO> tokenset;

  // check if the current token is in FIRST set 'ts'
  inline bool predict_1 (const tokenset &ts) {
    return ts[look_ahead ()];
  }

  /** Parse the given grammar rule. Saves the current state of 
   *  the builder, semantic, and token provider objects.
   *  \param rule The rule to parse. 
   *  \return True if parsed successfully. */
  template <class T> bool parse (CTree *(T::*rule)());
  /** Parse a sequence of the given grammar rule. 
   *  \param rule The rule to parse at least once. 
   *  \return True if parsed successfully. */
  template <class T> bool seq (CTree *(T::*rule)());
  /** Parse a sequence of the given grammar rule. 
   *  \param rule The rule to parse at least once. 
   *  \return True if parsed successfully. */
  template <class T> bool seq (bool (T::*rule)());
  /** Parse a sequence of the given grammar rule by calling RULE::check() in a loop.
   *  \param s A pointer to the syntax object on which the rule should be executed
   *  \tparam SYNTAX The type of syntax
   *  \tparam RULE The class that represents the grammar rule
   *  \return True if parsed successfully. */
  template <typename SYNTAX, typename RULE> static bool seq (SYNTAX &s);
  /** Parse a sequence of rule-separator pairs. 
   *  \param rule The rule to parse at least once. 
   *  \param separator The separator token.
   *  \param trailing_separator True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <class T> bool list (CTree *(T::*rule)(), int separator, bool trailing_separator = false);
  /** Parse a sequence of rule-separator pairs. 
   *  \param rule The rule to parse at least once. 
   *  \param separators The separator tokens.
   *  \param trailing_separator True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <class T> bool list (CTree *(T::*rule)(), int* separators, bool trailing_separator = false);
  /** Parse a sequence of rule-separator pairs. 
   *  \param rule The rule to parse at least once. 
   *  \param separator The separator token.
   *  \param trailing_separator True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <class T> bool list (bool (T::*rule)(), int separator, bool trailing_separator = false);
  /** Parse a sequence of rule-separator pairs. 
   *  \param rule The rule to parse at least once. 
   *  \param separators The separator tokens.
   *  \param trailing_separator True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <class T> bool list (bool (T::*rule)(), int* separators, bool trailing_separator = false);
  /** Parse a sequence of rule-separator pairs by calling RULE::check() in a loop.
   *  \param s A pointer to the syntax object on which the rule should be executed
   *  \tparam SYNTAX The type of syntax
   *  \tparam RULE The class that represents the grammar rule
   *  \param sep The separator token
   *  \param trailing_sep True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <typename SYNTAX, typename RULE>
  static bool list (SYNTAX &s, int sep, bool trailing_sep = false);
  /** Parse a sequence of rule-separator pairs by calling RULE::check() in a loop.
   *  \param s A pointer to the syntax object on which the rule should be executed
   *  \tparam SYNTAX The type of syntax
   *  \tparam RULE The class that represents the grammar rule
   *  \param separators The separator tokens
   *  \param trailing_sep True if a trailing separator token is allowed.
   *  \return True if parsed successfully. */
  template <typename SYNTAX, typename RULE>
  static bool list (SYNTAX &s, int *separators, bool trailing_sep = false);
  /** Parse a grammar rule automatically catching parse errors. 
   *  \param rule The rule to parse. 
   *  \param msg The error message to show if the rule fails.
   *  \param finish_tokens Set of token types that abort parsing the rule.
   *  \param skip_tokens If the rule fails skip all tokens until a token is read
   *                     that has one of the types given here. 
   *  \return False if at EOF or a finish_token is read, true otherwise. */
  template <class T> bool catch_error (bool (T::*rule)(), const char* msg, int* finish_tokens, int* skip_tokens);
  /** Parse a grammar rule automatically catching parse errors. 
   *  \tparam SYNTAX The type of syntax
   *  \tparam RULE The class that represents the grammar rule
   *  \param s A pointer to the syntax object on which the rule should be executed
   *  \param msg The error message to show if the rule fails.
   *  \param finish_tokens Set of token types that abort parsing the rule.
   *  \param skip_tokens If the rule fails skip all tokens until a token is read
   *                     that has one of the types given here. 
   *  \return False if at EOF or a finish_token is read, true otherwise. */
  template <class SYNTAX, class RULE> 
  static bool catch_error (SYNTAX &s, const char *msg, int *finish_tokens, int *skip_tokens);
  /** First parse rule1 and if that rule fails discard all errors and parse the rule2.
   *  \tparam RULE1 The class that represents the first grammar rule
   *  \tparam RULE2 The class that represents the second grammar rule
   *  \tparam SYNTAX The type of syntax
   *  \param s The syntax object on which the rules should be executed */
  template <class RULE1, class RULE2, class SYNTAX>
  static bool ambiguous (SYNTAX &s);
  /** Parse a token with the given type.
   *  \param token_type The token type. 
   *  \return True a corresponding token was parsed. */
  bool parse (int token_type);
  /** Parse a token with one of the given types.
   *  \param token_types The token types. 
   *  \return True a corresponding token was parsed. */
  bool parse (int *token_types);
  /** Parse a token with the given type.
   *  \param token_type The token type. 
   *  \return True a corresponding token was parsed. */
  bool parse_token (int token_type);
  /** Optional rule parsing. Always succeeds regardless
   *  of the argument.
   *  \param dummy Dummy parameter, is not evaluated.
   *  \return True. */
  bool opt (bool dummy) const;

  /** Get the syntax tree builder. */
  Builder &builder () const;
  /** Get the semantic analysis object. */
  Semantic &semantic () const;

  /** Top parse rule to be reimplemented for a specific grammar. 
   *  \return The root node of the syntax tree, or NULL. */
  virtual bool trans_unit ();

  /** Handle a compiler directive token. The default handling is
   *  to skip the compiler directive. */
  virtual void handle_directive ();

  /** Save the current parser state. Calls save_state() on the 
   *  builder, semantic, and token provider objects.
   *  \return The current parser state. */
  State save_state ();
  /** Forget the saved parser state. */
  void forget_state ();
  /** Restore the saved parser state. Triggers restoring the 
   *  syntax and semantic trees to the saved state. */
  void restore_state ();
  /** Restore the saved parser state to the given state. 
   *  Triggers restoring the syntax and semantic trees.
   *  \param state The state to which to restore. */
  void restore_state (State state);
  /** Overwrite the parser state with the given state. 
   *  \param state The new parser state. */
  void set_state (State state);

  /** Accept the given syntax tree node. If the node is NULL
   *  then the parser state is restored to the given state.
   *  Otherwise all saved states are discarded. 
   *  \param tree Tree to accept. 
   *  \param state The saved state. */
  bool accept (CTree *tree, State state);
  /** Accept the given syntax tree node. Returns the given node.
   *  \param tree Tree to accept. */
  CTree* accept (CTree *tree);

  /** Skip all non-core language tokens until the next 
   *  core-language token is read. 
   *  \return The next core-language token. */
  Token *locate_token ();
  /** Skip the current token. */
  void skip ();
  /** Skip all tokens between start and end, including
   *  start and end token.
   *  \param start The start token type.
   *  \param end The end token type.
   *  \param inclusive If true, the stop token is skipped too. */
  void skip_block (int start, int end, bool inclusive = true);
  /** Skip all tokens between '{' and '}', including
   *  '{' and '}'. */
  void skip_curly_block ();
  /** Skip all tokens between '(' and ')', including
   *  '(' and ')'. */
  void skip_round_block ();
  /** Parse all tokens between start and end, including
   *  start and end token.
   *  \param start The start token type.
   *  \param end The end token type.
   *  \return False if the stop token is not found, true otherwise. */
  bool parse_block (int start, int end);
  /** Parse all tokens between '{' and '}', including
   *  '{' and '}'.
   *  \return False if the stop token '}' is not found, true otherwise. */
  bool parse_curly_block ();
  /** Parse all tokens between '(' and ')', including
   *  '(' and ')'.
   *  \return False if the stop token ')' is not found, true otherwise. */
  bool parse_round_block ();
  /** Skip all tokens until a token with the given type is read.
   *  \param stop_token The type of the token to stop.
   *  \param inclusive If true, the stop token is skipped too. 
   *  \return False if the stop token is not found, true otherwise. */
  bool skip (int stop_token, bool inclusive = true);
  /** Skip all tokens until a token with one of the given types is read.
   *  \param stop_tokens The types of the token to stop.
   *  \param inclusive If true, the stop token is skipped too. 
   *  \return False if the stop token is not found, true otherwise. */
  bool skip (int *stop_tokens, bool inclusive = true);
  /** Check if the given token type is in the set of given token types.
   *  \param token_type The token type to check.
   *  \param token_types The set of token types. */
  bool is_in (int token_type, int *token_types) const;
};

inline Syntax::Syntax (Builder &b, Semantic &s) : 
  _problem_token ((Token*)0), 
  _have_error (false),
  _verbose_errors (false),
  _builder (b),
  _semantic (s) {
}

template <class T> 
CTree *Syntax::run (TokenProvider &tp, bool (T::*rule)()) {
  TokenProvider *tp_save = token_provider;
  token_provider = &tp;
  token_provider->init();
  _have_error = false;
  _problem_token = (Token*)0;
  locate_token ();
  CTree *result = (((T*)this)->*rule)() ? builder ().Top () : (CTree*)0;
  if (result) builder ().Pop ();
  token_provider = tp_save;
  return result;
}


inline int Syntax::look_ahead (unsigned n) {
  Token *token = token_provider->current ();
  if (n > 1) {
    State s = token_provider->get_state ();
    for (unsigned i = 1; i < n; i++) {
      token_provider->next ();
      locate_token ();
    }
    token = token_provider->current ();
    token_provider->set_state (s);
  }
  return token ? token->type () : 0;
}

inline bool Syntax::consume () {
  Token *t = token_provider->current ();
  _problem_token = (Token*)0;
  token_provider->next ();
  locate_token ();
  builder ().Push (builder ().token (t));
  return true;
}
 

inline bool Syntax::parse (int token_type) { return parse_token (token_type); }
inline bool Syntax::opt (bool) const { return true; }
inline bool Syntax::error () const { return _have_error; }
inline Token *Syntax::problem () const { return _problem_token; }
inline Builder &Syntax::builder () const { return _builder; }
inline Semantic &Syntax::semantic () const { return _semantic; }

inline bool Syntax::trans_unit () { return false; }

inline void Syntax::handle_directive () {
  while (token_provider->current () &&
         token_provider->current ()->is_directive ()) 
    token_provider->next ();
}

template <class T> 
inline bool Syntax::parse (CTree *(T::*rule)()) {
  State s = save_state ();
  return accept ((((T*)this)->*rule) (), s);
}

template <class T> 
inline bool Syntax::seq (CTree *(T::*rule)()) {
  if (! parse (rule))
    return false;
  while (parse (rule));
  return true;
}

template <class T> 
inline bool Syntax::seq ( bool (T::*rule)()) {
  if (!(((T*)this)->*rule) ())
    return false;
  while ((((T*)this)->*rule) ());
  return true;
}

template <typename SYNTAX, typename RULE> bool Syntax::seq (SYNTAX &s)  {
  if (!RULE::check (s))
    return false;
  while (RULE::check (s));
  return true;
}

template <class T> 
inline bool Syntax::list (CTree *(T::*rule)(), int token_type, bool end_sep) {
  if (! parse (rule))
    return false;
  while (parse (token_type))
    if (! parse (rule))
      return end_sep ? true : false;
  return true;
}

template <class T> 
inline bool Syntax::list (bool (T::*rule)(), int token_type, bool end_sep) {
  if (! (((T*)this)->*rule) ())
    return false;
  while (parse (token_type))
    if (! (((T*)this)->*rule) ())
      return end_sep ? true : false;
  return true;
}

template <typename SYNTAX, typename RULE>
inline bool Syntax::list (SYNTAX &s, int separator, bool trailing_sep) {
  if (!RULE::check (s))
    return false;
  while (s.parse (separator))
    if (!RULE::check (s))
      return trailing_sep ? true : false;
  return true;
}

template <typename SYNTAX, typename RULE>
inline bool Syntax::list (SYNTAX &s, int *separators, bool trailing_sep) {
  if (!RULE::check (s))
    return false;
  while (s.parse (separators))
    if (!RULE::check (s))
      return trailing_sep ? true : false;
  return true;
}

template <class T> 
inline bool Syntax::list (CTree *(T::*rule)(), int *token_type_set, bool end_sep) {
  if (! parse (rule))
    return false;
  while (parse (token_type_set))
    if (! parse (rule))
      return end_sep ? true : false;
  return true;
}

template <class T> 
inline bool Syntax::list (bool (T::*rule)(), int *token_type_set, bool end_sep) {
  if (! (((T*)this)->*rule) ())
    return false;
  while (parse (token_type_set))
    if (! (((T*)this)->*rule) ())
      return end_sep ? true : false;
  return true;
}

template <class T> 
bool Syntax::catch_error (bool (T::*rule)(), const char *msg,
                          int *finish_tokens, int *skip_tokens) {
  Token *current_pos, *token;
  int index;

  current_pos = token_provider->current ();
  if (! current_pos || is_in (current_pos->type (), finish_tokens))
    return false;

  index = ((ErrorCollector&)builder ().err ()).index ();
  if ((((T*)this)->*rule)())
    return true;

  _have_error = true;
  token = problem () ? problem () : current_pos;
  builder ().Push (builder ().error ());

  // if there is no detailed error message generate a standard message
  if (index == ((ErrorCollector&)builder ().err ()).index ()) {
    builder ().err () << sev_error << token->location () << msg
      << " near token `" << token->text () << "'" << endMessage;
    // if the error is located in not a file unit, print the whole unit
    if (_verbose_errors && token->unit () && !token->unit ()->isFile ()) {
      builder ().err () << token->location () << "located in the following non-file unit:\n"
        << *token->unit() << endMessage;
    }
  }

  skip (skip_tokens, false);
  return true;
}

template <class SYNTAX, class RULE> 
bool Syntax::catch_error (SYNTAX &s, const char *msg,
  int *finish_tokens, int *skip_tokens) {

  Token *current_pos, *token;
  int index;
  Builder &builder = ((Syntax&)s).builder ();

  current_pos = s.token_provider->current ();
  if (! current_pos || s.is_in (current_pos->type (), finish_tokens))
    return false;

  index = ((ErrorCollector&)builder.err ()).index ();
  if (RULE::check (s))
    return true;

  s._have_error = true;
  token = s.problem () ? s.problem () : current_pos;
  builder.Push (builder.error ());

  // if there is no detailed error message generate a standard message
  if (index == ((ErrorCollector&)builder.err ()).index ()) {
    builder.err () << sev_error << token->location () << msg
      << " near token `" << token->text () << "'" << endMessage;
    // if the error is located in not a file unit, print the whole unit
    if (s._verbose_errors && token->unit () && !token->unit ()->isFile ()) {
      builder.err () << token->location () << "located in the following non-file unit:\n"
        << *token->unit() << endMessage;
    }
  }

  s.skip (skip_tokens, false);
  return true;
}

template <class RULE1, class RULE2, class SYNTAX>
inline bool Syntax::ambiguous (SYNTAX &s) {
  Builder& builder = ((Syntax&)s).builder ();
  ErrorCollector& ec = (ErrorCollector&) builder.err ();
  int index = ec.index ();
  if (RULE1::check (s))
    return true;
  ec.index (index); // restore error state for ambiguous rule
  return RULE2::check (s);;
}

} // namespace Puma

#endif /* __Syntax_h__ */
