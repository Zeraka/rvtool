// A Bison parser, made by GNU Bison 3.3.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2019 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.


// Take the name prefix into account.
#define yylex   tlyylex



#include "parsetl.hh"


// Unqualified %code blocks.
#line 59 "parsetl.yy" // lalr1.cc:435

/* parsetl.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include <spot/parsetl/parsedecl.hh>
using namespace spot;

#define missing_right_op_msg(op, str)		\
  error_list.emplace_back(op,			\
    "missing right operand for \"" str "\"");

#define missing_right_op(res, op, str)		\
  do						\
    {						\
      missing_right_op_msg(op, str);		\
      res = fnode::ff();		\
    }						\
  while (0);

// right is missing, so complain and use left.
#define missing_right_binop(res, left, op, str)	\
  do						\
    {						\
      missing_right_op_msg(op, str);		\
      res = left;				\
    }						\
  while (0);

// right is missing, so complain and use false.
#define missing_right_binop_hard(res, left, op, str)	\
  do							\
    {							\
      left->destroy();					\
      missing_right_op(res, op, str);			\
    }							\
  while (0);

  static bool
  sere_ensure_bool(const fnode* f, const spot::location& loc,
                   const char* oper, spot::parse_error_list& error_list)
  {
    if (f->is_boolean())
      return true;
    std::string s;
    s.reserve(80);
    s = "not a Boolean expression: in a SERE ";
    s += oper;
    s += " can only be applied to a Boolean expression";
    error_list.emplace_back(loc, s);
    return false;
  }

  static const fnode*
  error_false_block(const spot::location& loc,
                    spot::parse_error_list& error_list)
  {
    error_list.emplace_back(loc, "treating this block as false");
    return fnode::ff();
  }

  static const fnode*
  parse_ap(const std::string& str,
           const spot::location& location,
           spot::environment& env,
           spot::parse_error_list& error_list)
  {
    auto res = env.require(str);
    if (!res)
      {
        std::string s;
        s.reserve(64);
        s = "unknown atomic proposition `";
        s += str;
        s += "' in ";
        s += env.name();
        error_list.emplace_back(location, s);
      }
    return res.to_node_();
  }

  enum parser_type { parser_ltl, parser_bool, parser_sere };

  static const fnode*
  try_recursive_parse(const std::string& str,
		      const spot::location& location,
		      spot::environment& env,
		      bool debug,
		      parser_type type,
		      spot::parse_error_list& error_list)
    {
      // We want to parse a U (b U c) as two until operators applied
      // to the atomic propositions a, b, and c.  We also want to
      // parse a U (b == c) as one until operator applied to the
      // atomic propositions "a" and "b == c".  The only problem is
      // that we do not know anything about "==" or in general about
      // the syntax of atomic proposition of our users.
      //
      // To support that, the lexer will return "b U c" and "b == c"
      // as PAR_BLOCK tokens.  We then try to parse such tokens
      // recursively.  If, as in the case of "b U c", the block is
      // successfully parsed as a formula, we return this formula.
      // Otherwise, we convert the string into an atomic proposition
      // (it's up to the environment to check the syntax of this
      // proposition, and maybe reject it).

      if (str.empty())
	{
	  error_list.emplace_back(location, "unexpected empty block");
	  return nullptr;
	}

      spot::parsed_formula pf;
      switch (type)
	{
	case parser_sere:
	  pf = spot::parse_infix_sere(str, env, debug, true);
	  break;
	case parser_bool:
	  pf = spot::parse_infix_boolean(str, env, debug, true);
	  break;
	case parser_ltl:
	  pf = spot::parse_infix_psl(str, env, debug, true);
	  break;
	}

      if (pf.errors.empty())
	return pf.f.to_node_();
      return parse_ap(str, location, env, error_list);
    }


#line 179 "parsetl.cc" // lalr1.cc:435


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if TLYYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !TLYYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !TLYYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace tlyy {
#line 274 "parsetl.cc" // lalr1.cc:510

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (spot::parse_error_list &error_list_yyarg, spot::environment &parse_environment_yyarg, spot::formula &result_yyarg)
    :
#if TLYYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      error_list (error_list_yyarg),
      parse_environment (parse_environment_yyarg),
      result (result_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/

  // basic_symbol.
#if 201103L <= YY_CPLUSPLUS
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (basic_symbol&& that)
    : Base (std::move (that))
    , value (std::move (that.value))
    , location (std::move (that.location))
  {}
#endif

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_type.
  parser::by_type::by_type ()
    : type (empty_symbol)
  {}

#if 201103L <= YY_CPLUSPLUS
  parser::by_type::by_type (by_type&& that)
    : type (that.type)
  {
    that.clear ();
  }
#endif

  parser::by_type::by_type (const by_type& that)
    : type (that.type)
  {}

  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  void
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  int
  parser::by_type::type_get () const YY_NOEXCEPT
  {
    return type;
  }


  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_number_type
  parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.type_get ())
    {
      case 9: // "(...) block"
#line 280 "parsetl.yy" // lalr1.cc:652
        { delete (yysym.value.str); }
#line 509 "parsetl.cc" // lalr1.cc:652
        break;

      case 10: // "{...} block"
#line 280 "parsetl.yy" // lalr1.cc:652
        { delete (yysym.value.str); }
#line 515 "parsetl.cc" // lalr1.cc:652
        break;

      case 11: // "{...}! block"
#line 280 "parsetl.yy" // lalr1.cc:652
        { delete (yysym.value.str); }
#line 521 "parsetl.cc" // lalr1.cc:652
        break;

      case 50: // "atomic proposition"
#line 280 "parsetl.yy" // lalr1.cc:652
        { delete (yysym.value.str); }
#line 527 "parsetl.cc" // lalr1.cc:652
        break;

      case 92: // atomprop
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 533 "parsetl.cc" // lalr1.cc:652
        break;

      case 93: // booleanatom
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 539 "parsetl.cc" // lalr1.cc:652
        break;

      case 94: // sere
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 545 "parsetl.cc" // lalr1.cc:652
        break;

      case 95: // bracedsere
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 551 "parsetl.cc" // lalr1.cc:652
        break;

      case 96: // parenthesedsubformula
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 557 "parsetl.cc" // lalr1.cc:652
        break;

      case 97: // boolformula
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 563 "parsetl.cc" // lalr1.cc:652
        break;

      case 98: // subformula
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 569 "parsetl.cc" // lalr1.cc:652
        break;

      case 99: // lbtformula
#line 281 "parsetl.yy" // lalr1.cc:652
        { (yysym.value.ltl)->destroy(); }
#line 575 "parsetl.cc" // lalr1.cc:652
        break;

      default:
        break;
    }
  }

#if TLYYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
      case 9: // "(...) block"
#line 283 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << *(yysym.value.str); }
#line 606 "parsetl.cc" // lalr1.cc:676
        break;

      case 10: // "{...} block"
#line 283 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << *(yysym.value.str); }
#line 612 "parsetl.cc" // lalr1.cc:676
        break;

      case 11: // "{...}! block"
#line 283 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << *(yysym.value.str); }
#line 618 "parsetl.cc" // lalr1.cc:676
        break;

      case 42: // "number for square bracket operator"
#line 286 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.num); }
#line 624 "parsetl.cc" // lalr1.cc:676
        break;

      case 50: // "atomic proposition"
#line 283 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << *(yysym.value.str); }
#line 630 "parsetl.cc" // lalr1.cc:676
        break;

      case 58: // "SVA delay operator"
#line 286 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.num); }
#line 636 "parsetl.cc" // lalr1.cc:676
        break;

      case 85: // sqbracketargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 642 "parsetl.cc" // lalr1.cc:676
        break;

      case 86: // gotoargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 648 "parsetl.cc" // lalr1.cc:676
        break;

      case 88: // starargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 654 "parsetl.cc" // lalr1.cc:676
        break;

      case 89: // fstarargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 660 "parsetl.cc" // lalr1.cc:676
        break;

      case 90: // equalargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 666 "parsetl.cc" // lalr1.cc:676
        break;

      case 91: // delayargs
#line 287 "parsetl.yy" // lalr1.cc:676
        { debug_stream() << (yysym.value.minmax).min << ".." << (yysym.value.minmax).max; }
#line 672 "parsetl.cc" // lalr1.cc:676
        break;

      case 92: // atomprop
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 678 "parsetl.cc" // lalr1.cc:676
        break;

      case 93: // booleanatom
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 684 "parsetl.cc" // lalr1.cc:676
        break;

      case 94: // sere
#line 285 "parsetl.yy" // lalr1.cc:676
        { print_sere(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 690 "parsetl.cc" // lalr1.cc:676
        break;

      case 95: // bracedsere
#line 285 "parsetl.yy" // lalr1.cc:676
        { print_sere(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 696 "parsetl.cc" // lalr1.cc:676
        break;

      case 96: // parenthesedsubformula
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 702 "parsetl.cc" // lalr1.cc:676
        break;

      case 97: // boolformula
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 708 "parsetl.cc" // lalr1.cc:676
        break;

      case 98: // subformula
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 714 "parsetl.cc" // lalr1.cc:676
        break;

      case 99: // lbtformula
#line 284 "parsetl.yy" // lalr1.cc:676
        { print_psl(debug_stream(), formula((yysym.value.ltl)->clone())); }
#line 720 "parsetl.cc" // lalr1.cc:676
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if TLYYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // TLYYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location, error_list));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 291 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 963 "parsetl.cc" // lalr1.cc:919
    break;

  case 3:
#line 296 "parsetl.yy" // lalr1.cc:919
    {
		result = nullptr;
		YYABORT;
	      }
#line 972 "parsetl.cc" // lalr1.cc:919
    break;

  case 4:
#line 301 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 981 "parsetl.cc" // lalr1.cc:919
    break;

  case 5:
#line 306 "parsetl.yy" // lalr1.cc:919
    { YYABORT; }
#line 987 "parsetl.cc" // lalr1.cc:919
    break;

  case 6:
#line 308 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 996 "parsetl.cc" // lalr1.cc:919
    break;

  case 7:
#line 313 "parsetl.yy" // lalr1.cc:919
    {
		result = nullptr;
		YYABORT;
	      }
#line 1005 "parsetl.cc" // lalr1.cc:919
    break;

  case 8:
#line 318 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 1014 "parsetl.cc" // lalr1.cc:919
    break;

  case 9:
#line 323 "parsetl.yy" // lalr1.cc:919
    { YYABORT; }
#line 1020 "parsetl.cc" // lalr1.cc:919
    break;

  case 10:
#line 325 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 1029 "parsetl.cc" // lalr1.cc:919
    break;

  case 11:
#line 330 "parsetl.yy" // lalr1.cc:919
    {
		result = nullptr;
		YYABORT;
	      }
#line 1038 "parsetl.cc" // lalr1.cc:919
    break;

  case 12:
#line 335 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 1047 "parsetl.cc" // lalr1.cc:919
    break;

  case 13:
#line 340 "parsetl.yy" // lalr1.cc:919
    { YYABORT; }
#line 1053 "parsetl.cc" // lalr1.cc:919
    break;

  case 14:
#line 342 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 1062 "parsetl.cc" // lalr1.cc:919
    break;

  case 15:
#line 347 "parsetl.yy" // lalr1.cc:919
    {
		result = nullptr;
		YYABORT;
	      }
#line 1071 "parsetl.cc" // lalr1.cc:919
    break;

  case 16:
#line 352 "parsetl.yy" // lalr1.cc:919
    {
		result = formula((yystack_[1].value.ltl));
		YYACCEPT;
	      }
#line 1080 "parsetl.cc" // lalr1.cc:919
    break;

  case 17:
#line 357 "parsetl.yy" // lalr1.cc:919
    { YYABORT; }
#line 1086 "parsetl.cc" // lalr1.cc:919
    break;

  case 18:
#line 360 "parsetl.yy" // lalr1.cc:919
    {
		error_list.emplace_back(yylhs.location, "empty input");
		result = nullptr;
	      }
#line 1095 "parsetl.cc" // lalr1.cc:919
    break;

  case 19:
#line 366 "parsetl.yy" // lalr1.cc:919
    {
		error_list.emplace_back(yystack_[1].location, "ignoring trailing garbage");
	      }
#line 1103 "parsetl.cc" // lalr1.cc:919
    break;

  case 26:
#line 379 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yystack_[3].value.num); (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1109 "parsetl.cc" // lalr1.cc:919
    break;

  case 27:
#line 381 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yystack_[2].value.num); (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1115 "parsetl.cc" // lalr1.cc:919
    break;

  case 28:
#line 383 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1121 "parsetl.cc" // lalr1.cc:919
    break;

  case 29:
#line 385 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1127 "parsetl.cc" // lalr1.cc:919
    break;

  case 30:
#line 387 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1133 "parsetl.cc" // lalr1.cc:919
    break;

  case 31:
#line 391 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yystack_[3].value.num); (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1139 "parsetl.cc" // lalr1.cc:919
    break;

  case 32:
#line 393 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yystack_[2].value.num); (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1145 "parsetl.cc" // lalr1.cc:919
    break;

  case 33:
#line 395 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 1U; (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1151 "parsetl.cc" // lalr1.cc:919
    break;

  case 34:
#line 397 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 1U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1157 "parsetl.cc" // lalr1.cc:919
    break;

  case 35:
#line 399 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yylhs.value.minmax).max = 1U; }
#line 1163 "parsetl.cc" // lalr1.cc:919
    break;

  case 36:
#line 401 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = (yylhs.value.minmax).max = (yystack_[1].value.num); }
#line 1169 "parsetl.cc" // lalr1.cc:919
    break;

  case 37:
#line 403 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "treating this goto block as [->]");
             (yylhs.value.minmax).min = (yylhs.value.minmax).max = 1U; }
#line 1176 "parsetl.cc" // lalr1.cc:919
    break;

  case 38:
#line 406 "parsetl.yy" // lalr1.cc:919
    { error_list.
	       emplace_back(yylhs.location, "missing closing bracket for goto operator");
	     (yylhs.value.minmax).min = (yylhs.value.minmax).max = 0U; }
#line 1184 "parsetl.cc" // lalr1.cc:919
    break;

  case 41:
#line 413 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1190 "parsetl.cc" // lalr1.cc:919
    break;

  case 42:
#line 415 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 1U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1196 "parsetl.cc" // lalr1.cc:919
    break;

  case 43:
#line 417 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax) = (yystack_[0].value.minmax); }
#line 1202 "parsetl.cc" // lalr1.cc:919
    break;

  case 44:
#line 419 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "treating this star block as [*]");
              (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1209 "parsetl.cc" // lalr1.cc:919
    break;

  case 45:
#line 422 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "missing closing bracket for star");
	      (yylhs.value.minmax).min = (yylhs.value.minmax).max = 0U; }
#line 1216 "parsetl.cc" // lalr1.cc:919
    break;

  case 46:
#line 426 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1222 "parsetl.cc" // lalr1.cc:919
    break;

  case 47:
#line 428 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 1U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1228 "parsetl.cc" // lalr1.cc:919
    break;

  case 48:
#line 430 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax) = (yystack_[0].value.minmax); }
#line 1234 "parsetl.cc" // lalr1.cc:919
    break;

  case 49:
#line 432 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back
		(yylhs.location, "treating this fusion-star block as [:*]");
              (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1242 "parsetl.cc" // lalr1.cc:919
    break;

  case 50:
#line 436 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back
		(yylhs.location, "missing closing bracket for fusion-star");
	      (yylhs.value.minmax).min = (yylhs.value.minmax).max = 0U; }
#line 1250 "parsetl.cc" // lalr1.cc:919
    break;

  case 51:
#line 441 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax) = (yystack_[0].value.minmax); }
#line 1256 "parsetl.cc" // lalr1.cc:919
    break;

  case 52:
#line 443 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "treating this equal block as [=]");
              (yylhs.value.minmax).min = 0U; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1263 "parsetl.cc" // lalr1.cc:919
    break;

  case 53:
#line 446 "parsetl.yy" // lalr1.cc:919
    { error_list.
		emplace_back(yylhs.location, "missing closing bracket for equal operator");
	      (yylhs.value.minmax).min = (yylhs.value.minmax).max = 0U; }
#line 1271 "parsetl.cc" // lalr1.cc:919
    break;

  case 54:
#line 451 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax) = (yystack_[0].value.minmax); }
#line 1277 "parsetl.cc" // lalr1.cc:919
    break;

  case 55:
#line 453 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "treating this delay block as ##1");
              (yylhs.value.minmax).min = (yylhs.value.minmax).max = 1U; }
#line 1284 "parsetl.cc" // lalr1.cc:919
    break;

  case 56:
#line 456 "parsetl.yy" // lalr1.cc:919
    { error_list.
		emplace_back(yylhs.location, "missing closing bracket for ##[");
	      (yylhs.value.minmax).min = (yylhs.value.minmax).max = 1U; }
#line 1292 "parsetl.cc" // lalr1.cc:919
    break;

  case 57:
#line 460 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 1; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1298 "parsetl.cc" // lalr1.cc:919
    break;

  case 58:
#line 462 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.minmax).min = 0; (yylhs.value.minmax).max = fnode::unbounded(); }
#line 1304 "parsetl.cc" // lalr1.cc:919
    break;

  case 59:
#line 465 "parsetl.yy" // lalr1.cc:919
    {
            (yylhs.value.ltl) = parse_ap(*(yystack_[0].value.str), yystack_[0].location, parse_environment, error_list);
            delete (yystack_[0].value.str);
            if (!(yylhs.value.ltl))
              YYERROR;
          }
#line 1315 "parsetl.cc" // lalr1.cc:919
    break;

  case 60:
#line 472 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 1321 "parsetl.cc" // lalr1.cc:919
    break;

  case 61:
#line 473 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[1].value.ltl); }
#line 1327 "parsetl.cc" // lalr1.cc:919
    break;

  case 62:
#line 475 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) = fnode::unop(op::Not, (yystack_[1].value.ltl));
	      }
#line 1335 "parsetl.cc" // lalr1.cc:919
    break;

  case 63:
#line 479 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::tt(); }
#line 1341 "parsetl.cc" // lalr1.cc:919
    break;

  case 64:
#line 481 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::ff(); }
#line 1347 "parsetl.cc" // lalr1.cc:919
    break;

  case 65:
#line 483 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 1353 "parsetl.cc" // lalr1.cc:919
    break;

  case 66:
#line 485 "parsetl.yy" // lalr1.cc:919
    {
		if (sere_ensure_bool((yystack_[0].value.ltl), yystack_[0].location, "`!'", error_list))
		  {
		    (yylhs.value.ltl) = fnode::unop(op::Not, (yystack_[0].value.ltl));
		  }
		else
		  {
		    (yystack_[0].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1369 "parsetl.cc" // lalr1.cc:919
    break;

  case 67:
#line 496 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 1375 "parsetl.cc" // lalr1.cc:919
    break;

  case 68:
#line 498 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) =
		  try_recursive_parse(*(yystack_[0].value.str), yystack_[0].location, parse_environment,
				      debug_level(), parser_sere, error_list);
		delete (yystack_[0].value.str);
		if (!(yylhs.value.ltl))
		  YYERROR;
	      }
#line 1388 "parsetl.cc" // lalr1.cc:919
    break;

  case 69:
#line 507 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[1].value.ltl); }
#line 1394 "parsetl.cc" // lalr1.cc:919
    break;

  case 70:
#line 509 "parsetl.yy" // lalr1.cc:919
    { error_list.
		  emplace_back(yylhs.location,
			       "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1404 "parsetl.cc" // lalr1.cc:919
    break;

  case 71:
#line 515 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[2].location + yystack_[1].location, "missing closing parenthesis");
		(yylhs.value.ltl) = (yystack_[1].value.ltl);
	      }
#line 1412 "parsetl.cc" // lalr1.cc:919
    break;

  case 72:
#line 519 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1422 "parsetl.cc" // lalr1.cc:919
    break;

  case 73:
#line 525 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::AndRat, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1428 "parsetl.cc" // lalr1.cc:919
    break;

  case 74:
#line 527 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
				    "length-matching and operator"); }
#line 1435 "parsetl.cc" // lalr1.cc:919
    break;

  case 75:
#line 530 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::AndNLM, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1441 "parsetl.cc" // lalr1.cc:919
    break;

  case 76:
#line 532 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
                                    "non-length-matching and operator"); }
#line 1448 "parsetl.cc" // lalr1.cc:919
    break;

  case 77:
#line 535 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::OrRat, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1454 "parsetl.cc" // lalr1.cc:919
    break;

  case 78:
#line 537 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "or operator"); }
#line 1460 "parsetl.cc" // lalr1.cc:919
    break;

  case 79:
#line 539 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::Concat, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1466 "parsetl.cc" // lalr1.cc:919
    break;

  case 80:
#line 541 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "concat operator"); }
#line 1472 "parsetl.cc" // lalr1.cc:919
    break;

  case 81:
#line 543 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::Fusion, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1478 "parsetl.cc" // lalr1.cc:919
    break;

  case 82:
#line 545 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "fusion operator"); }
#line 1484 "parsetl.cc" // lalr1.cc:919
    break;

  case 83:
#line 547 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = formula::sugar_delay(formula((yystack_[0].value.ltl)), (yystack_[1].value.num), (yystack_[1].value.num)).to_node_(); }
#line 1490 "parsetl.cc" // lalr1.cc:919
    break;

  case 84:
#line 549 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), fnode::tt(), yystack_[1].location, "SVA delay operator"); }
#line 1496 "parsetl.cc" // lalr1.cc:919
    break;

  case 85:
#line 551 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = formula::sugar_delay(formula((yystack_[2].value.ltl)), formula((yystack_[0].value.ltl)),
                                          (yystack_[1].value.num), (yystack_[1].value.num)).to_node_(); }
#line 1503 "parsetl.cc" // lalr1.cc:919
    break;

  case 86:
#line 554 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "SVA delay operator"); }
#line 1509 "parsetl.cc" // lalr1.cc:919
    break;

  case 87:
#line 556 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[1].value.minmax).max < (yystack_[1].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[1].location, "reversed range");
		    std::swap((yystack_[1].value.minmax).max, (yystack_[1].value.minmax).min);
		  }
                (yylhs.value.ltl) = formula::sugar_delay(formula((yystack_[0].value.ltl)),
                                          (yystack_[1].value.minmax).min, (yystack_[1].value.minmax).max).to_node_();
              }
#line 1523 "parsetl.cc" // lalr1.cc:919
    break;

  case 88:
#line 566 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), fnode::tt(), yystack_[1].location, "SVA delay operator"); }
#line 1529 "parsetl.cc" // lalr1.cc:919
    break;

  case 89:
#line 568 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[1].value.minmax).max < (yystack_[1].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[2].location, "reversed range");
		    std::swap((yystack_[1].value.minmax).max, (yystack_[1].value.minmax).min);
		  }
                (yylhs.value.ltl) = formula::sugar_delay(formula((yystack_[2].value.ltl)), formula((yystack_[0].value.ltl)),
                                          (yystack_[1].value.minmax).min, (yystack_[1].value.minmax).max).to_node_();
              }
#line 1543 "parsetl.cc" // lalr1.cc:919
    break;

  case 90:
#line 578 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "SVA delay operator"); }
#line 1549 "parsetl.cc" // lalr1.cc:919
    break;

  case 91:
#line 580 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[0].value.minmax).max < (yystack_[0].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap((yystack_[0].value.minmax).max, (yystack_[0].value.minmax).min);
		  }
		(yylhs.value.ltl) = fnode::bunop(op::Star, fnode::tt(), (yystack_[0].value.minmax).min, (yystack_[0].value.minmax).max);
	      }
#line 1562 "parsetl.cc" // lalr1.cc:919
    break;

  case 92:
#line 589 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[0].value.minmax).max < (yystack_[0].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap((yystack_[0].value.minmax).max, (yystack_[0].value.minmax).min);
		  }
		(yylhs.value.ltl) = fnode::bunop(op::Star, (yystack_[1].value.ltl), (yystack_[0].value.minmax).min, (yystack_[0].value.minmax).max);
	      }
#line 1575 "parsetl.cc" // lalr1.cc:919
    break;

  case 93:
#line 598 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[0].value.minmax).max < (yystack_[0].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap((yystack_[0].value.minmax).max, (yystack_[0].value.minmax).min);
		  }
		(yylhs.value.ltl) = fnode::bunop(op::FStar, (yystack_[1].value.ltl), (yystack_[0].value.minmax).min, (yystack_[0].value.minmax).max);
	      }
#line 1588 "parsetl.cc" // lalr1.cc:919
    break;

  case 94:
#line 607 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[0].value.minmax).max < (yystack_[0].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap((yystack_[0].value.minmax).max, (yystack_[0].value.minmax).min);
		  }
		if (sere_ensure_bool((yystack_[1].value.ltl), yystack_[1].location, "[=...]", error_list))
		  {
		    (yylhs.value.ltl) = formula::sugar_equal(formula((yystack_[1].value.ltl)),
					      (yystack_[0].value.minmax).min, (yystack_[0].value.minmax).max).to_node_();
		  }
		else
		  {
		    (yystack_[1].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1610 "parsetl.cc" // lalr1.cc:919
    break;

  case 95:
#line 625 "parsetl.yy" // lalr1.cc:919
    {
		if ((yystack_[0].value.minmax).max < (yystack_[0].value.minmax).min)
		  {
		    error_list.emplace_back(yystack_[0].location, "reversed range");
		    std::swap((yystack_[0].value.minmax).max, (yystack_[0].value.minmax).min);
		  }
		if (sere_ensure_bool((yystack_[1].value.ltl), yystack_[1].location, "[->...]", error_list))
		  {
		    (yylhs.value.ltl) = formula::sugar_goto(formula((yystack_[1].value.ltl)),
					     (yystack_[0].value.minmax).min, (yystack_[0].value.minmax).max).to_node_();
		  }
		else
		  {
		    (yystack_[1].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1632 "parsetl.cc" // lalr1.cc:919
    break;

  case 96:
#line 643 "parsetl.yy" // lalr1.cc:919
    {
		if (sere_ensure_bool((yystack_[2].value.ltl), yystack_[2].location, "`^'", error_list)
                    && sere_ensure_bool((yystack_[0].value.ltl), yystack_[0].location, "`^'", error_list))
		  {
		    (yylhs.value.ltl) = fnode::binop(op::Xor, (yystack_[2].value.ltl), (yystack_[0].value.ltl));
		  }
		else
		  {
		    (yystack_[2].value.ltl)->destroy();
		    (yystack_[0].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1650 "parsetl.cc" // lalr1.cc:919
    break;

  case 97:
#line 657 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "xor operator"); }
#line 1656 "parsetl.cc" // lalr1.cc:919
    break;

  case 98:
#line 659 "parsetl.yy" // lalr1.cc:919
    {
		if (sere_ensure_bool((yystack_[2].value.ltl), yystack_[2].location, "`->'", error_list))
		  {
		    (yylhs.value.ltl) = fnode::binop(op::Implies, (yystack_[2].value.ltl), (yystack_[0].value.ltl));
		  }
		else
		  {
		    (yystack_[2].value.ltl)->destroy();
		    (yystack_[0].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1673 "parsetl.cc" // lalr1.cc:919
    break;

  case 99:
#line 672 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "implication operator"); }
#line 1679 "parsetl.cc" // lalr1.cc:919
    break;

  case 100:
#line 674 "parsetl.yy" // lalr1.cc:919
    {
		if (sere_ensure_bool((yystack_[2].value.ltl), yystack_[2].location, "`<->'", error_list)
                    && sere_ensure_bool((yystack_[0].value.ltl), yystack_[0].location, "`<->'", error_list))
		  {
		    (yylhs.value.ltl) = fnode::binop(op::Equiv, (yystack_[2].value.ltl), (yystack_[0].value.ltl));
		  }
		else
		  {
		    (yystack_[2].value.ltl)->destroy();
		    (yystack_[0].value.ltl)->destroy();
		    (yylhs.value.ltl) = error_false_block(yylhs.location, error_list);
		  }
	      }
#line 1697 "parsetl.cc" // lalr1.cc:919
    break;

  case 101:
#line 688 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "equivalent operator"); }
#line 1703 "parsetl.cc" // lalr1.cc:919
    break;

  case 102:
#line 690 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::first_match, (yystack_[1].value.ltl)); }
#line 1709 "parsetl.cc" // lalr1.cc:919
    break;

  case 103:
#line 693 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[1].value.ltl); }
#line 1715 "parsetl.cc" // lalr1.cc:919
    break;

  case 104:
#line 695 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[1].location, "ignoring this");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1723 "parsetl.cc" // lalr1.cc:919
    break;

  case 105:
#line 699 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
					"treating this brace block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1732 "parsetl.cc" // lalr1.cc:919
    break;

  case 106:
#line 704 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[2].location + yystack_[1].location,
					"missing closing brace");
		(yylhs.value.ltl) = (yystack_[1].value.ltl);
	      }
#line 1741 "parsetl.cc" // lalr1.cc:919
    break;

  case 107:
#line 709 "parsetl.yy" // lalr1.cc:919
    { error_list. emplace_back(yystack_[1].location,
                  "ignoring trailing garbage and missing closing brace");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1750 "parsetl.cc" // lalr1.cc:919
    break;

  case 108:
#line 714 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
                    "missing closing brace, "
		    "treating this brace block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1760 "parsetl.cc" // lalr1.cc:919
    break;

  case 109:
#line 720 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) = try_recursive_parse(*(yystack_[0].value.str), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_sere, error_list);
		delete (yystack_[0].value.str);
		if (!(yylhs.value.ltl))
		  YYERROR;
	      }
#line 1773 "parsetl.cc" // lalr1.cc:919
    break;

  case 110:
#line 730 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) = try_recursive_parse(*(yystack_[0].value.str), yystack_[0].location, parse_environment,
					 debug_level(), parser_ltl, error_list);
		delete (yystack_[0].value.str);
		if (!(yylhs.value.ltl))
		  YYERROR;
	      }
#line 1785 "parsetl.cc" // lalr1.cc:919
    break;

  case 111:
#line 738 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[1].value.ltl); }
#line 1791 "parsetl.cc" // lalr1.cc:919
    break;

  case 112:
#line 740 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[1].location, "ignoring this");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1799 "parsetl.cc" // lalr1.cc:919
    break;

  case 113:
#line 744 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
		 "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1808 "parsetl.cc" // lalr1.cc:919
    break;

  case 114:
#line 749 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[2].location + yystack_[1].location, "missing closing parenthesis");
		(yylhs.value.ltl) = (yystack_[1].value.ltl);
	      }
#line 1816 "parsetl.cc" // lalr1.cc:919
    break;

  case 115:
#line 753 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[1].location,
                "ignoring trailing garbage and missing closing parenthesis");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1825 "parsetl.cc" // lalr1.cc:919
    break;

  case 116:
#line 758 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1835 "parsetl.cc" // lalr1.cc:919
    break;

  case 117:
#line 765 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 1841 "parsetl.cc" // lalr1.cc:919
    break;

  case 118:
#line 767 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) = try_recursive_parse(*(yystack_[0].value.str), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_bool, error_list);
		delete (yystack_[0].value.str);
		if (!(yylhs.value.ltl))
		  YYERROR;
	      }
#line 1854 "parsetl.cc" // lalr1.cc:919
    break;

  case 119:
#line 776 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[1].value.ltl); }
#line 1860 "parsetl.cc" // lalr1.cc:919
    break;

  case 120:
#line 778 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[1].location, "ignoring this");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1868 "parsetl.cc" // lalr1.cc:919
    break;

  case 121:
#line 782 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
		 "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1877 "parsetl.cc" // lalr1.cc:919
    break;

  case 122:
#line 787 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[2].location + yystack_[1].location,
					"missing closing parenthesis");
		(yylhs.value.ltl) = (yystack_[1].value.ltl);
	      }
#line 1886 "parsetl.cc" // lalr1.cc:919
    break;

  case 123:
#line 792 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[1].location,
                "ignoring trailing garbage and missing closing parenthesis");
		(yylhs.value.ltl) = (yystack_[2].value.ltl);
	      }
#line 1895 "parsetl.cc" // lalr1.cc:919
    break;

  case 124:
#line 797 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location,
                    "missing closing parenthesis, "
		    "treating this parenthetical block as false");
		(yylhs.value.ltl) = fnode::ff();
	      }
#line 1905 "parsetl.cc" // lalr1.cc:919
    break;

  case 125:
#line 803 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1911 "parsetl.cc" // lalr1.cc:919
    break;

  case 126:
#line 805 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 1917 "parsetl.cc" // lalr1.cc:919
    break;

  case 127:
#line 807 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1923 "parsetl.cc" // lalr1.cc:919
    break;

  case 128:
#line 809 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 1929 "parsetl.cc" // lalr1.cc:919
    break;

  case 129:
#line 811 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1935 "parsetl.cc" // lalr1.cc:919
    break;

  case 130:
#line 813 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 1941 "parsetl.cc" // lalr1.cc:919
    break;

  case 131:
#line 815 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::Or, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 1947 "parsetl.cc" // lalr1.cc:919
    break;

  case 132:
#line 817 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "or operator"); }
#line 1953 "parsetl.cc" // lalr1.cc:919
    break;

  case 133:
#line 819 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Xor, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 1959 "parsetl.cc" // lalr1.cc:919
    break;

  case 134:
#line 821 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "xor operator"); }
#line 1965 "parsetl.cc" // lalr1.cc:919
    break;

  case 135:
#line 823 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Implies, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 1971 "parsetl.cc" // lalr1.cc:919
    break;

  case 136:
#line 825 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "implication operator"); }
#line 1977 "parsetl.cc" // lalr1.cc:919
    break;

  case 137:
#line 827 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Equiv, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 1983 "parsetl.cc" // lalr1.cc:919
    break;

  case 138:
#line 829 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "equivalent operator"); }
#line 1989 "parsetl.cc" // lalr1.cc:919
    break;

  case 139:
#line 831 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::Not, (yystack_[0].value.ltl)); }
#line 1995 "parsetl.cc" // lalr1.cc:919
    break;

  case 140:
#line 833 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[1].location, "not operator"); }
#line 2001 "parsetl.cc" // lalr1.cc:919
    break;

  case 141:
#line 835 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 2007 "parsetl.cc" // lalr1.cc:919
    break;

  case 142:
#line 836 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 2013 "parsetl.cc" // lalr1.cc:919
    break;

  case 143:
#line 838 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 2019 "parsetl.cc" // lalr1.cc:919
    break;

  case 144:
#line 840 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 2025 "parsetl.cc" // lalr1.cc:919
    break;

  case 145:
#line 842 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 2031 "parsetl.cc" // lalr1.cc:919
    break;

  case 146:
#line 844 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 2037 "parsetl.cc" // lalr1.cc:919
    break;

  case 147:
#line 846 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 2043 "parsetl.cc" // lalr1.cc:919
    break;

  case 148:
#line 848 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "and operator"); }
#line 2049 "parsetl.cc" // lalr1.cc:919
    break;

  case 149:
#line 850 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::Or, {(yystack_[2].value.ltl), (yystack_[0].value.ltl)}); }
#line 2055 "parsetl.cc" // lalr1.cc:919
    break;

  case 150:
#line 852 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "or operator"); }
#line 2061 "parsetl.cc" // lalr1.cc:919
    break;

  case 151:
#line 854 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Xor, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2067 "parsetl.cc" // lalr1.cc:919
    break;

  case 152:
#line 856 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "xor operator"); }
#line 2073 "parsetl.cc" // lalr1.cc:919
    break;

  case 153:
#line 858 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Implies, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2079 "parsetl.cc" // lalr1.cc:919
    break;

  case 154:
#line 860 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "implication operator"); }
#line 2085 "parsetl.cc" // lalr1.cc:919
    break;

  case 155:
#line 862 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Equiv, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2091 "parsetl.cc" // lalr1.cc:919
    break;

  case 156:
#line 864 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "equivalent operator"); }
#line 2097 "parsetl.cc" // lalr1.cc:919
    break;

  case 157:
#line 866 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::U, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2103 "parsetl.cc" // lalr1.cc:919
    break;

  case 158:
#line 868 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "until operator"); }
#line 2109 "parsetl.cc" // lalr1.cc:919
    break;

  case 159:
#line 870 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::R, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2115 "parsetl.cc" // lalr1.cc:919
    break;

  case 160:
#line 872 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "release operator"); }
#line 2121 "parsetl.cc" // lalr1.cc:919
    break;

  case 161:
#line 874 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::W, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2127 "parsetl.cc" // lalr1.cc:919
    break;

  case 162:
#line 876 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "weak until operator"); }
#line 2133 "parsetl.cc" // lalr1.cc:919
    break;

  case 163:
#line 878 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::M, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2139 "parsetl.cc" // lalr1.cc:919
    break;

  case 164:
#line 880 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location, "strong release operator"); }
#line 2145 "parsetl.cc" // lalr1.cc:919
    break;

  case 165:
#line 882 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::F, (yystack_[0].value.ltl)); }
#line 2151 "parsetl.cc" // lalr1.cc:919
    break;

  case 166:
#line 884 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[1].location, "sometimes operator"); }
#line 2157 "parsetl.cc" // lalr1.cc:919
    break;

  case 167:
#line 886 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::Or, (yystack_[2].value.num), (yystack_[2].value.num), (yystack_[0].value.ltl));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "F[n:m] expects two parameters");
              }
#line 2166 "parsetl.cc" // lalr1.cc:919
    break;

  case 168:
#line 892 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::Or, (yystack_[4].value.num), (yystack_[2].value.num), (yystack_[0].value.ltl)); }
#line 2172 "parsetl.cc" // lalr1.cc:919
    break;

  case 169:
#line 895 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::Or, (yystack_[3].value.num),
                                            fnode::unbounded(), (yystack_[0].value.ltl)); }
#line 2179 "parsetl.cc" // lalr1.cc:919
    break;

  case 170:
#line 899 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[5].location + yystack_[1].location, "F[.] operator"); }
#line 2185 "parsetl.cc" // lalr1.cc:919
    break;

  case 171:
#line 901 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "missing closing bracket for F[.]");
                (yylhs.value.ltl) = fnode::ff(); }
#line 2192 "parsetl.cc" // lalr1.cc:919
    break;

  case 172:
#line 904 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this F[.] as a simple F");
                (yylhs.value.ltl) = fnode::unop(op::F, (yystack_[0].value.ltl)); }
#line 2200 "parsetl.cc" // lalr1.cc:919
    break;

  case 173:
#line 908 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::G, (yystack_[0].value.ltl)); }
#line 2206 "parsetl.cc" // lalr1.cc:919
    break;

  case 174:
#line 910 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[1].location, "always operator"); }
#line 2212 "parsetl.cc" // lalr1.cc:919
    break;

  case 175:
#line 913 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::And, (yystack_[4].value.num), (yystack_[2].value.num), (yystack_[0].value.ltl)); }
#line 2218 "parsetl.cc" // lalr1.cc:919
    break;

  case 176:
#line 916 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::And, (yystack_[3].value.num),
                                            fnode::unbounded(), (yystack_[0].value.ltl)); }
#line 2225 "parsetl.cc" // lalr1.cc:919
    break;

  case 177:
#line 919 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::And, (yystack_[2].value.num), (yystack_[2].value.num), (yystack_[0].value.ltl));
                error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "G[n:m] expects two parameters");
              }
#line 2234 "parsetl.cc" // lalr1.cc:919
    break;

  case 178:
#line 925 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[5].location + yystack_[1].location, "G[.] operator"); }
#line 2240 "parsetl.cc" // lalr1.cc:919
    break;

  case 179:
#line 927 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "missing closing bracket for G[.]");
                (yylhs.value.ltl) = fnode::ff(); }
#line 2247 "parsetl.cc" // lalr1.cc:919
    break;

  case 180:
#line 930 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yystack_[3].location + yystack_[1].location,
                                        "treating this G[.] as a simple G");
                (yylhs.value.ltl) = fnode::unop(op::F, (yystack_[0].value.ltl)); }
#line 2255 "parsetl.cc" // lalr1.cc:919
    break;

  case 181:
#line 934 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::X, (yystack_[0].value.ltl)); }
#line 2261 "parsetl.cc" // lalr1.cc:919
    break;

  case 182:
#line 936 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[1].location, "next operator"); }
#line 2267 "parsetl.cc" // lalr1.cc:919
    break;

  case 183:
#line 938 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::nested_unop_range(op::X, op::Or, (yystack_[2].value.num), (yystack_[2].value.num), (yystack_[0].value.ltl)); }
#line 2273 "parsetl.cc" // lalr1.cc:919
    break;

  case 184:
#line 940 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[3].location + yystack_[1].location, "X[.] operator"); }
#line 2279 "parsetl.cc" // lalr1.cc:919
    break;

  case 185:
#line 942 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "treating this X[.] as a simple X");
                (yylhs.value.ltl) = fnode::unop(op::X, (yystack_[0].value.ltl)); }
#line 2286 "parsetl.cc" // lalr1.cc:919
    break;

  case 186:
#line 945 "parsetl.yy" // lalr1.cc:919
    { error_list.emplace_back(yylhs.location, "missing closing bracket for X[.]");
                (yylhs.value.ltl) = fnode::ff(); }
#line 2293 "parsetl.cc" // lalr1.cc:919
    break;

  case 187:
#line 948 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::Not, (yystack_[0].value.ltl)); }
#line 2299 "parsetl.cc" // lalr1.cc:919
    break;

  case 188:
#line 950 "parsetl.yy" // lalr1.cc:919
    { missing_right_op((yylhs.value.ltl), yystack_[1].location, "not operator"); }
#line 2305 "parsetl.cc" // lalr1.cc:919
    break;

  case 189:
#line 952 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::Closure, (yystack_[0].value.ltl)); }
#line 2311 "parsetl.cc" // lalr1.cc:919
    break;

  case 190:
#line 954 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::UConcat, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2317 "parsetl.cc" // lalr1.cc:919
    break;

  case 191:
#line 956 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::UConcat, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2323 "parsetl.cc" // lalr1.cc:919
    break;

  case 192:
#line 958 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop_hard((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
				    "universal overlapping concat operator"); }
#line 2330 "parsetl.cc" // lalr1.cc:919
    break;

  case 193:
#line 961 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::EConcat, (yystack_[2].value.ltl), (yystack_[0].value.ltl)); }
#line 2336 "parsetl.cc" // lalr1.cc:919
    break;

  case 194:
#line 963 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop_hard((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
				    "existential overlapping concat operator");
	      }
#line 2344 "parsetl.cc" // lalr1.cc:919
    break;

  case 195:
#line 968 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::UConcat,
				  fnode::multop(op::Concat, {(yystack_[2].value.ltl), fnode::tt()}),
				  (yystack_[0].value.ltl)); }
#line 2352 "parsetl.cc" // lalr1.cc:919
    break;

  case 196:
#line 972 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop_hard((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
				  "universal non-overlapping concat operator");
	      }
#line 2360 "parsetl.cc" // lalr1.cc:919
    break;

  case 197:
#line 977 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::EConcat,
				  fnode::multop(op::Concat, {(yystack_[2].value.ltl), fnode::tt()}),
				  (yystack_[0].value.ltl)); }
#line 2368 "parsetl.cc" // lalr1.cc:919
    break;

  case 198:
#line 981 "parsetl.yy" // lalr1.cc:919
    { missing_right_binop_hard((yylhs.value.ltl), (yystack_[2].value.ltl), yystack_[1].location,
				"existential non-overlapping concat operator");
	      }
#line 2376 "parsetl.cc" // lalr1.cc:919
    break;

  case 199:
#line 986 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::EConcat, (yystack_[1].value.ltl), fnode::tt()); }
#line 2382 "parsetl.cc" // lalr1.cc:919
    break;

  case 200:
#line 988 "parsetl.yy" // lalr1.cc:919
    {
		(yylhs.value.ltl) = try_recursive_parse(*(yystack_[0].value.str), yystack_[0].location, parse_environment,
					 debug_level(),
                                         parser_sere, error_list);
		delete (yystack_[0].value.str);
		if (!(yylhs.value.ltl))
		  YYERROR;
		(yylhs.value.ltl) = fnode::binop(op::EConcat, (yylhs.value.ltl), fnode::tt());
	      }
#line 2396 "parsetl.cc" // lalr1.cc:919
    break;

  case 201:
#line 998 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = (yystack_[0].value.ltl); }
#line 2402 "parsetl.cc" // lalr1.cc:919
    break;

  case 202:
#line 1000 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::Not, (yystack_[0].value.ltl)); }
#line 2408 "parsetl.cc" // lalr1.cc:919
    break;

  case 203:
#line 1002 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::And, {(yystack_[1].value.ltl), (yystack_[0].value.ltl)}); }
#line 2414 "parsetl.cc" // lalr1.cc:919
    break;

  case 204:
#line 1004 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::multop(op::Or, {(yystack_[1].value.ltl), (yystack_[0].value.ltl)}); }
#line 2420 "parsetl.cc" // lalr1.cc:919
    break;

  case 205:
#line 1006 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Xor, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2426 "parsetl.cc" // lalr1.cc:919
    break;

  case 206:
#line 1008 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Implies, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2432 "parsetl.cc" // lalr1.cc:919
    break;

  case 207:
#line 1010 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::Equiv, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2438 "parsetl.cc" // lalr1.cc:919
    break;

  case 208:
#line 1012 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::X, (yystack_[0].value.ltl)); }
#line 2444 "parsetl.cc" // lalr1.cc:919
    break;

  case 209:
#line 1014 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::F, (yystack_[0].value.ltl)); }
#line 2450 "parsetl.cc" // lalr1.cc:919
    break;

  case 210:
#line 1016 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::unop(op::G, (yystack_[0].value.ltl)); }
#line 2456 "parsetl.cc" // lalr1.cc:919
    break;

  case 211:
#line 1018 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::U, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2462 "parsetl.cc" // lalr1.cc:919
    break;

  case 212:
#line 1020 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::R, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2468 "parsetl.cc" // lalr1.cc:919
    break;

  case 213:
#line 1022 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::R, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2474 "parsetl.cc" // lalr1.cc:919
    break;

  case 214:
#line 1024 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::W, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2480 "parsetl.cc" // lalr1.cc:919
    break;

  case 215:
#line 1026 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::binop(op::M, (yystack_[1].value.ltl), (yystack_[0].value.ltl)); }
#line 2486 "parsetl.cc" // lalr1.cc:919
    break;

  case 216:
#line 1028 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::tt(); }
#line 2492 "parsetl.cc" // lalr1.cc:919
    break;

  case 217:
#line 1030 "parsetl.yy" // lalr1.cc:919
    { (yylhs.value.ltl) = fnode::ff(); }
#line 2498 "parsetl.cc" // lalr1.cc:919
    break;


#line 2502 "parsetl.cc" // lalr1.cc:919
            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -85;

  const signed char parser::yytable_ninf_ = -25;

  const short
  parser::yypact_[] =
  {
     232,  1136,   334,   591,   385,    25,   -54,  1293,   -85,   -85,
     -85,   626,  1300,  1348,  1355,  1403,     9,    13,    21,   -85,
     -85,   -85,   -85,   -85,   -85,    63,   -85,   236,   -85,    15,
    2026,  2026,  2026,  2026,  2026,  2026,  2026,  2026,  2026,  2026,
    2026,  2026,  2026,  2026,   -85,   -85,   -85,   -85,   -85,     1,
     661,   -85,   626,  1143,   -85,   -85,   -85,    99,     2,   696,
     138,   -85,   -85,   -85,   -85,   -85,   -85,   731,   -85,   536,
     -85,   529,   -85,   557,   -85,   -85,   -85,   598,   -85,   -85,
      20,   241,    -9,   459,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -14,     8,   -13,    30,   -15,    -1,    38,    46,
      22,   -85,   -85,  1410,  1458,  1465,  1513,   -85,  1520,  1568,
    1575,  1623,  1630,  1678,  1685,  1733,  1740,  1788,  1795,   -85,
     -85,   -85,  2026,  2026,  2026,  2026,  2026,   -85,   -85,   -85,
    2026,  2026,  2026,  2026,  2026,   -85,   -85,    36,  1190,   489,
     -85,    45,   121,   109,   -85,    47,    26,   -85,  1143,   -85,
    2094,    51,    39,   -85,   -85,  2094,   766,   801,   836,   871,
     906,   941,   -85,   -85,   270,   336,   382,   976,  1011,   -85,
    1046,   -85,   -85,   -85,   -85,   -85,  1081,    40,   414,   -85,
     -85,  1210,  1286,  1883,  1911,  1931,  1940,  1949,   -85,   -85,
     -85,   -85,    61,   -85,   -85,   -85,   -85,    11,   -85,   -85,
     -85,  1997,  1843,   -85,  1997,  1997,   131,    74,   -85,  1997,
    1997,   142,    80,   -85,   -85,  1104,   -85,  1104,   -85,  1104,
     -85,  1104,   -85,  1371,   -85,  1426,   -85,   123,   -85,   123,
     -85,  1104,   -85,  1104,   -85,   269,   -85,   269,   -85,   269,
     -85,   269,   -85,   269,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,
     184,    97,   112,   -85,   -85,   -85,  1238,   -85,   -85,   -85,
    2103,   -85,  2127,   -85,   448,   -85,   448,   -85,  2094,   -85,
    2094,   136,   126,   -85,   150,   141,   -85,   158,   -85,   122,
     255,   160,   152,   -85,  2000,   -85,  2047,   -85,  2094,   -85,
    2094,   -85,   -85,    70,   -85,   -85,   -85,   110,   -85,   132,
     -85,   177,   -85,   177,   -85,   201,   -85,   201,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   170,
    1997,   -85,   -85,   174,  1997,   182,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   265,   188,   200,   -85,   -85,
     -85,   -85,  1850,   -85,  1898,   -85,   -85,   206,   -85,   -85,
     -85,   -85,   -85,   -85,   -85
  };

  const unsigned char
  parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     0,     0,     0,   110,   109,
     200,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      63,    64,    18,     5,     3,    60,   141,   189,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,   217,    17,    15,   201,     0,
       0,    68,     0,     0,    39,    40,    42,     0,     0,     0,
       0,    57,    58,    13,    11,    41,    91,     0,    65,     0,
      67,     0,   118,     0,     9,     7,   117,     0,     1,    19,
       0,     0,     0,     0,   166,   165,   174,   173,   182,   181,
     188,   187,    25,     0,     0,    25,     0,     0,    25,     0,
       0,    62,    61,     0,     0,     0,     0,   191,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     2,
       4,   202,     0,     0,     0,     0,     0,   208,   209,   210,
       0,     0,     0,     0,     0,    14,    16,     0,     0,     0,
      66,    25,     0,    20,    23,     0,     0,    43,     0,    84,
      83,    25,     0,    54,    88,    87,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,     0,     0,    10,
       0,    12,    95,    92,    93,    94,     0,     0,     0,   140,
     139,     0,     0,     0,     0,     0,     0,     0,     6,     8,
     113,   116,     0,   111,   114,   105,   108,     0,   103,   199,
     106,     0,     0,   186,     0,     0,    20,     0,   171,     0,
       0,    20,     0,   179,   192,   190,   194,   193,   196,   195,
     198,   197,   150,   149,   152,   151,   144,   143,   146,   145,
     154,   153,   156,   155,   158,   157,   160,   159,   162,   161,
     164,   163,   148,   147,   203,   204,   205,   206,   207,   211,
     212,   213,   214,   215,    70,    72,    69,    71,    44,    30,
      20,     0,     0,    21,    29,    45,     0,    55,    56,    78,
      77,    97,    96,    74,    73,    76,    75,    99,    98,   101,
     100,    25,     0,    48,    25,     0,    51,    25,    35,     0,
      20,     0,     0,    80,    79,    82,    81,    86,    85,    90,
      89,   121,   124,     0,   119,   122,   132,   131,   134,   133,
     126,   125,   128,   127,   136,   135,   138,   137,   130,   129,
     112,   115,   104,   107,   185,   184,   183,   172,   167,     0,
       0,   180,   177,     0,     0,     0,    27,    28,   102,    49,
      50,    52,    53,    37,    36,    20,     0,     0,    34,    38,
     120,   123,     0,   169,     0,   176,    26,     0,    32,    33,
     170,   168,   178,   175,    31
  };

  const short
  parser::yypgoto_[] =
  {
     -85,   -85,    58,    16,   -84,   -85,    23,   -47,   -85,   -85,
     -66,   -85,   -85,   -26,   319,   119,     0,   172,   186,   -50,
      -7,   333
  };

  const short
  parser::yydefgoto_[] =
  {
      -1,     5,    23,    24,   144,   145,    94,   147,   172,    65,
      66,   174,   175,    67,    25,    26,    69,    27,    28,    77,
      29,    49
  };

  const short
  parser::yytable_[] =
  {
      81,    79,     6,   173,   195,    85,    87,    89,    91,   148,
      92,    83,   207,   153,    95,   212,     6,   173,    47,    64,
      75,   178,    98,   180,   322,    78,   205,   201,   190,   206,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      97,   100,   203,   176,   254,   120,   196,   118,   301,   202,
     138,    93,   139,   140,   208,    96,   135,   176,   261,   150,
      46,    63,    74,    99,   -24,   136,   323,   155,   -24,   320,
     119,   204,   173,   173,   173,   191,   -24,   213,   350,   209,
     146,   265,   291,   152,   173,   171,   258,   210,   264,   173,
     211,   255,   267,   189,   268,   302,   215,   217,   219,   221,
     141,   223,   225,   227,   229,   231,   233,   235,   237,   239,
     241,   243,   176,   176,   176,   330,   321,   283,   286,   101,
     102,   334,    68,    76,   176,   351,   182,   183,   184,   176,
      68,   307,   309,   311,   313,   315,   317,   319,   336,   151,
     -22,   142,   187,   143,   114,   115,   116,   117,   266,   183,
     184,   262,   263,   337,   -24,   118,   270,   272,   274,   276,
     278,   280,   259,   344,   187,   260,   345,   294,   296,    68,
     298,    68,    68,   329,   263,    70,   300,   339,    68,   -22,
     142,   340,   143,    70,   333,   263,    68,   282,   285,   292,
      76,   341,    76,   -24,   324,   326,   342,   327,   328,   343,
     173,   348,   331,   332,   173,   346,   173,   349,   173,   187,
     173,   352,   173,   107,   173,   354,   181,   182,   183,   184,
     185,   186,    70,   356,    70,    70,   335,   263,   173,   358,
     173,    70,   173,   187,   173,     1,     2,     3,     4,    70,
     176,   359,   192,     7,   176,     8,   176,   364,   176,   193,
     176,     0,   176,     0,   176,     0,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,     0,    68,   176,     0,
     176,   281,   176,   118,   176,    68,    68,    68,    68,    68,
      68,   103,   104,   105,   106,     0,    68,    68,     0,    68,
     114,   115,   116,   117,     0,    68,   194,   347,   263,     0,
      76,    76,    76,    76,    76,    76,    76,   357,   263,     0,
       0,   -22,   142,     0,   143,     0,     0,     0,     0,     0,
      70,    48,     0,   353,     0,   -24,     0,   355,    70,    70,
      70,    70,    70,    70,     0,     6,     0,   284,     0,    70,
      70,     0,    70,     0,     0,   361,     0,   363,    70,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   -22,   142,     0,
     143,     0,     0,   287,    19,     0,     6,     0,     0,    22,
       0,   -24,    71,     0,    72,     0,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,    73,     0,   303,     0,     0,     0,     0,
       0,     0,   304,   288,   289,     0,   290,     0,     0,   181,
     182,   183,   184,   185,   186,    19,     0,   -24,    20,    21,
      22,    48,    48,    48,    48,    48,   187,     0,     0,    48,
      48,    48,    48,    48,     0,   244,   245,   246,   247,   248,
     197,     0,     0,   249,   250,   251,   252,   253,     0,   305,
       0,     0,   198,   199,   156,   157,   158,   159,   160,   161,
      54,    55,   162,    56,   163,    57,   164,   165,   166,     0,
     197,    54,    55,   162,    56,   163,    57,   164,   165,   166,
       0,     0,   198,     0,   156,   157,   158,   159,   160,   161,
     167,   168,     0,     0,   200,     0,     0,   170,    60,    61,
      62,    54,    55,   162,    56,   163,    57,   164,   165,   166,
     177,     0,     0,     0,     0,     0,    71,     6,    72,     0,
     167,   168,     0,     0,   200,     0,     0,   170,    60,    61,
      62,   156,   157,   158,   159,   160,   161,    73,   179,     0,
       0,     0,     0,     0,    71,     0,    72,     0,    54,    55,
     162,    56,   163,    57,   164,   165,   166,     0,     0,    19,
       0,     0,    20,    21,     0,    73,     0,   167,   168,     0,
       0,   169,     6,     0,   170,    60,    61,    62,    50,     6,
      51,     9,     0,    52,     0,     0,     0,    19,     0,     0,
      20,    21,     0,   181,   182,   183,   184,   185,   186,    53,
       0,     0,     0,    54,    55,     0,    56,    82,    57,     0,
     187,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,    22,     0,     0,    59,
      60,    61,    62,   188,    53,     0,     0,     0,    54,    55,
       0,    56,   137,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   149,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   154,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   269,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   271,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   273,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   275,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   277,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   279,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   293,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   295,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,   297,    57,     0,
       0,     0,     0,    50,     0,    51,     9,     0,    52,     0,
      58,    19,     0,     0,    20,    21,     0,     0,     0,    59,
      60,    61,    62,     0,    53,     0,     0,     0,    54,    55,
       0,    56,   299,    57,     0,     0,     0,     0,    50,     0,
      51,     9,     0,    52,     0,    58,    19,     0,     0,    20,
      21,     0,     0,     0,    59,    60,    61,    62,     0,    53,
       0,     0,     0,    54,    55,     0,    56,     0,    57,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,     0,
      58,    19,     0,     0,    20,    21,   118,     6,     0,    59,
      60,    61,    62,     7,     0,     8,     9,    10,    11,     0,
      50,     0,    51,     9,     0,    52,     0,     0,     0,     0,
       0,    12,    13,    14,    15,    16,    17,    18,     0,     0,
       0,    53,     0,     0,     0,    54,    55,     0,    56,     0,
      57,     0,     0,     0,     0,     0,    19,     0,     0,    20,
      21,    22,    58,    19,     0,     0,    20,    21,   256,     0,
       0,    59,    60,    61,    62,   156,   157,   158,   159,   160,
     161,   306,     0,     0,     0,     0,     0,    71,     0,    72,
       0,     0,    54,    55,   162,    56,   163,    57,   164,   165,
     166,     0,     0,     0,     0,     0,     0,     0,    73,     0,
       0,   167,   168,     0,     0,   257,   338,     0,   170,    60,
      61,    62,     0,   156,   157,   158,   159,   160,   161,     0,
      19,     0,     0,    20,    21,     0,     0,     0,     0,     0,
      54,    55,   162,    56,   163,    57,   164,   165,   166,     0,
       0,     0,     0,     0,     0,     0,     0,   308,     0,   167,
     168,     0,     0,    71,    80,    72,   170,    60,    61,    62,
       7,    84,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,    73,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,     0,     0,     0,    19,     0,     0,    20,
      21,     0,     0,    19,     0,     0,    20,    21,     0,    86,
      19,     0,     0,    20,    21,     7,    88,     8,     9,    10,
      11,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,    13,    14,    15,    16,    17,    18,
      12,    13,    14,    15,    16,    17,    18,   109,   110,   111,
       0,     0,   114,   115,   116,   117,     0,     0,    19,     0,
       0,    20,    21,   118,    90,    19,     0,     0,    20,    21,
       7,   214,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,   110,   111,     0,     0,   114,   115,   116,
     117,     0,     0,    19,     0,     0,    20,    21,   118,   216,
      19,     0,     0,    20,    21,     7,   218,     8,     9,    10,
      11,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,    13,    14,    15,    16,    17,    18,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,    20,    21,     0,   220,    19,     0,     0,    20,    21,
       7,   222,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,    20,    21,     0,   224,
      19,     0,     0,    20,    21,     7,   226,     8,     9,    10,
      11,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,    13,    14,    15,    16,    17,    18,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,    20,    21,     0,   228,    19,     0,     0,    20,    21,
       7,   230,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,    20,    21,     0,   232,
      19,     0,     0,    20,    21,     7,   234,     8,     9,    10,
      11,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,    13,    14,    15,    16,    17,    18,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,    20,    21,     0,   236,    19,     0,     0,    20,    21,
       7,   238,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,    20,    21,     0,   240,
      19,     0,     0,    20,    21,     7,   242,     8,     9,    10,
      11,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,    13,    14,    15,    16,    17,    18,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,    20,    21,     0,   325,    19,     0,     0,    20,    21,
       7,   360,     8,     9,    10,    11,     0,     7,     0,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12,    13,
      14,    15,    16,    17,    18,    12,    13,    14,    15,    16,
      17,    18,     0,     0,   310,     0,     0,     0,     0,     0,
      71,     0,    72,    19,     0,     0,    20,    21,     0,   362,
      19,     0,     0,    20,    21,     7,     0,     8,     9,    10,
      11,    73,   312,     0,     0,     0,     0,     0,    71,     0,
      72,     0,     0,    12,    13,    14,    15,    16,    17,    18,
       0,     0,   314,    19,     0,     0,    20,    21,    71,    73,
      72,   316,     0,     0,     0,     0,     0,    71,    19,    72,
     318,    20,    21,     0,     0,     0,    71,     0,    72,    73,
       0,    19,     0,     0,    20,    21,     0,     0,    73,     0,
       0,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,    19,     0,     0,    20,    21,     0,     0,     0,     0,
      19,     0,     0,    20,    21,     0,     0,     0,     0,    19,
       0,     0,    20,    21,     7,     0,     8,     9,    10,    11,
       0,     0,     0,     0,     0,   156,   157,   158,   159,   160,
     161,     0,    12,    13,    14,    15,    16,    17,    18,     0,
       0,     0,    54,    55,   162,    56,   163,    57,   164,   165,
     166,     0,     0,     0,     0,     0,     0,    19,     0,     0,
      20,    21,   168,     0,     0,     0,     0,     0,   170,    60,
      61,    62,   156,   157,   158,   159,   160,   161,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,    54,
      55,   162,    56,   163,    57,   164,   165,   166,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,   170,    60,    61,    62,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,   157,
     158,   159,     0,     0,     0,     0,    54,    55,   162,    56,
     163,    57,   164,   165,   166,    54,    55,   162,    56,   163,
      57,   164,   165,   166,   158,   159,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    54,
      55,   162,    56,   163,    57,   164,   165,   166
  };

  const short
  parser::yycheck_[] =
  {
       7,    55,     1,    69,    13,    12,    13,    14,    15,     7,
       1,    11,    96,    60,     1,    99,     1,    83,     2,     3,
       4,    71,     1,    73,    13,     0,    41,    41,     8,    44,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      17,    18,    55,    69,     8,    29,    55,    32,     8,    41,
      50,    42,    52,    53,    55,    42,    55,    83,   142,    59,
       2,     3,     4,    42,    55,    49,    55,    67,    55,     8,
      55,    41,   138,   139,   140,    55,    55,    55,     8,    41,
      57,    55,   166,    60,   150,    69,    41,    41,    41,   155,
      44,    55,    41,    77,    55,    55,   103,   104,   105,   106,
       1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   138,   139,   140,    41,    55,   164,   165,    56,
      57,    41,     3,     4,   150,    55,    16,    17,    18,   155,
      11,   181,   182,   183,   184,   185,   186,   187,    41,     1,
      41,    42,    32,    44,    21,    22,    23,    24,   148,    17,
      18,    42,    43,    41,    55,    32,   156,   157,   158,   159,
     160,   161,    41,    41,    32,    44,    44,   167,   168,    50,
     170,    52,    53,    42,    43,     3,   176,    41,    59,    41,
      42,    55,    44,    11,    42,    43,    67,   164,   165,   166,
      71,    41,    73,    55,   201,   202,    55,   204,   205,    41,
     266,    41,   209,   210,   270,   289,   272,    55,   274,    32,
     276,    41,   278,    27,   280,    41,    15,    16,    17,    18,
      19,    20,    50,    41,    52,    53,    42,    43,   294,    41,
     296,    59,   298,    32,   300,     3,     4,     5,     6,    67,
     266,    41,     1,     7,   270,     9,   272,    41,   274,     8,
     276,    -1,   278,    -1,   280,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,   148,   294,    -1,
     296,     1,   298,    32,   300,   156,   157,   158,   159,   160,
     161,    45,    46,    47,    48,    -1,   167,   168,    -1,   170,
      21,    22,    23,    24,    -1,   176,    55,    42,    43,    -1,
     181,   182,   183,   184,   185,   186,   187,    42,    43,    -1,
      -1,    41,    42,    -1,    44,    -1,    -1,    -1,    -1,    -1,
     148,     2,    -1,   330,    -1,    55,    -1,   334,   156,   157,
     158,   159,   160,   161,    -1,     1,    -1,     1,    -1,   167,
     168,    -1,   170,    -1,    -1,   352,    -1,   354,   176,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    41,    42,    -1,
      44,    -1,    -1,     1,    50,    -1,     1,    -1,    -1,    55,
      -1,    55,     7,    -1,     9,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    -1,    28,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    -1,     8,    41,    42,    -1,    44,    -1,    -1,    15,
      16,    17,    18,    19,    20,    50,    -1,    55,    53,    54,
      55,   122,   123,   124,   125,   126,    32,    -1,    -1,   130,
     131,   132,   133,   134,    -1,   122,   123,   124,   125,   126,
       1,    -1,    -1,   130,   131,   132,   133,   134,    -1,    55,
      -1,    -1,    13,    14,    15,    16,    17,    18,    19,    20,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
       1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    13,    -1,    15,    16,    17,    18,    19,    20,
      51,    52,    -1,    -1,    55,    -1,    -1,    58,    59,    60,
      61,    32,    33,    34,    35,    36,    37,    38,    39,    40,
       1,    -1,    -1,    -1,    -1,    -1,     7,     1,     9,    -1,
      51,    52,    -1,    -1,    55,    -1,    -1,    58,    59,    60,
      61,    15,    16,    17,    18,    19,    20,    28,     1,    -1,
      -1,    -1,    -1,    -1,     7,    -1,     9,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,    50,
      -1,    -1,    53,    54,    -1,    28,    -1,    51,    52,    -1,
      -1,    55,     1,    -1,    58,    59,    60,    61,     7,     1,
       9,    10,    -1,    12,    -1,    -1,    -1,    50,    -1,    -1,
      53,    54,    -1,    15,    16,    17,    18,    19,    20,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      32,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    55,    -1,    -1,    58,
      59,    60,    61,    55,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,     1,    37,    -1,
      -1,    -1,    -1,     7,    -1,     9,    10,    -1,    12,    -1,
      49,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    -1,    28,    -1,    -1,    -1,    32,    33,
      -1,    35,     1,    37,    -1,    -1,    -1,    -1,     7,    -1,
       9,    10,    -1,    12,    -1,    49,    50,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    -1,    28,
      -1,    -1,    -1,    32,    33,    -1,    35,    -1,    37,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      49,    50,    -1,    -1,    53,    54,    32,     1,    -1,    58,
      59,    60,    61,     7,    -1,     9,    10,    11,    12,    -1,
       7,    -1,     9,    10,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    32,    33,    -1,    35,    -1,
      37,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    53,
      54,    55,    49,    50,    -1,    -1,    53,    54,     8,    -1,
      -1,    58,    59,    60,    61,    15,    16,    17,    18,    19,
      20,     1,    -1,    -1,    -1,    -1,    -1,     7,    -1,     9,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    51,    52,    -1,    -1,    55,     8,    -1,    58,    59,
      60,    61,    -1,    15,    16,    17,    18,    19,    20,    -1,
      50,    -1,    -1,    53,    54,    -1,    -1,    -1,    -1,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    51,
      52,    -1,    -1,     7,     1,     9,    58,    59,    60,    61,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    28,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    50,    -1,    -1,    53,
      54,    -1,    -1,    50,    -1,    -1,    53,    54,    -1,     1,
      50,    -1,    -1,    53,    54,     7,     1,     9,    10,    11,
      12,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      25,    26,    27,    28,    29,    30,    31,    16,    17,    18,
      -1,    -1,    21,    22,    23,    24,    -1,    -1,    50,    -1,
      -1,    53,    54,    32,     1,    50,    -1,    -1,    53,    54,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    17,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    -1,    50,    -1,    -1,    53,    54,    32,     1,
      50,    -1,    -1,    53,    54,     7,     1,     9,    10,    11,
      12,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    53,    54,    -1,     1,    50,    -1,    -1,    53,    54,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    53,    54,    -1,     1,
      50,    -1,    -1,    53,    54,     7,     1,     9,    10,    11,
      12,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    53,    54,    -1,     1,    50,    -1,    -1,    53,    54,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    53,    54,    -1,     1,
      50,    -1,    -1,    53,    54,     7,     1,     9,    10,    11,
      12,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    53,    54,    -1,     1,    50,    -1,    -1,    53,    54,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    53,    54,    -1,     1,
      50,    -1,    -1,    53,    54,     7,     1,     9,    10,    11,
      12,    -1,     7,    -1,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    53,    54,    -1,     1,    50,    -1,    -1,    53,    54,
       7,     1,     9,    10,    11,    12,    -1,     7,    -1,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
       7,    -1,     9,    50,    -1,    -1,    53,    54,    -1,     1,
      50,    -1,    -1,    53,    54,     7,    -1,     9,    10,    11,
      12,    28,     1,    -1,    -1,    -1,    -1,    -1,     7,    -1,
       9,    -1,    -1,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,     1,    50,    -1,    -1,    53,    54,     7,    28,
       9,     1,    -1,    -1,    -1,    -1,    -1,     7,    50,     9,
       1,    53,    54,    -1,    -1,    -1,     7,    -1,     9,    28,
      -1,    50,    -1,    -1,    53,    54,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    50,    -1,    -1,    53,    54,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    53,    54,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    53,    54,     7,    -1,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      20,    -1,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      53,    54,    52,    -1,    -1,    -1,    -1,    -1,    58,    59,
      60,    61,    15,    16,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    -1,    58,    59,    60,    61,    15,
      16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,    16,
      17,    18,    -1,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    17,    18,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     3,     4,     5,     6,    79,     1,     7,     9,    10,
      11,    12,    25,    26,    27,    28,    29,    30,    31,    50,
      53,    54,    55,    80,    81,    92,    93,    95,    96,    98,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    80,    81,    92,    99,
       7,     9,    12,    28,    32,    33,    35,    37,    49,    58,
      59,    60,    61,    80,    81,    87,    88,    91,    93,    94,
      95,     7,     9,    28,    80,    81,    93,    97,     0,    55,
       1,    98,     1,    94,     1,    98,     1,    98,     1,    98,
       1,    98,     1,    42,    84,     1,    42,    84,     1,    42,
      84,    56,    57,    45,    46,    47,    48,    96,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    32,    55,
      81,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    55,    81,     1,    94,    94,
      94,     1,    42,    44,    82,    83,    84,    85,     7,     1,
      94,     1,    84,    85,     1,    94,    15,    16,    17,    18,
      19,    20,    34,    36,    38,    39,    40,    51,    52,    55,
      58,    81,    86,    88,    89,    90,    91,     1,    97,     1,
      97,    15,    16,    17,    18,    19,    20,    32,    55,    81,
       8,    55,     1,     8,    55,    13,    55,     1,    13,    14,
      55,    41,    41,    55,    41,    41,    44,    82,    55,    41,
      41,    44,    82,    55,     1,    98,     1,    98,     1,    98,
       1,    98,     1,    98,     1,    98,     1,    98,     1,    98,
       1,    98,     1,    98,     1,    98,     1,    98,     1,    98,
       1,    98,     1,    98,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,     8,    55,     8,    55,    41,    41,
      44,    82,    42,    43,    41,    55,    94,    41,    55,     1,
      94,     1,    94,     1,    94,     1,    94,     1,    94,     1,
      94,     1,    84,    85,     1,    84,    85,     1,    41,    42,
      44,    82,    84,     1,    94,     1,    94,     1,    94,     1,
      94,     8,    55,     1,     8,    55,     1,    97,     1,    97,
       1,    97,     1,    97,     1,    97,     1,    97,     1,    97,
       8,    55,    13,    55,    98,     1,    98,    98,    98,    42,
      41,    98,    98,    42,    41,    42,    41,    41,     8,    41,
      55,    41,    55,    41,    41,    44,    82,    42,    41,    55,
       8,    55,    41,    98,    41,    98,    41,    42,    41,    41,
       1,    98,     1,    98,    41
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    78,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    80,    81,
      82,    82,    83,    83,    84,    84,    85,    85,    85,    85,
      85,    86,    86,    86,    86,    86,    86,    86,    86,    87,
      87,    88,    88,    88,    88,    88,    89,    89,    89,    89,
      89,    90,    90,    90,    91,    91,    91,    91,    91,    92,
      93,    93,    93,    93,    93,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    95,    95,    95,    95,    95,    95,    95,
      96,    96,    96,    96,    96,    96,    96,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     1,     2,
       1,     2,     0,     1,     0,     1,     4,     3,     3,     2,
       2,     5,     4,     4,     3,     2,     3,     3,     3,     1,
       1,     1,     1,     2,     3,     3,     1,     1,     2,     3,
       3,     2,     3,     3,     2,     3,     3,     1,     1,     1,
       1,     2,     2,     1,     1,     1,     2,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     3,     3,     2,     2,     3,
       3,     1,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     4,     3,     4,     3,     3,     4,     3,     1,
       1,     3,     4,     3,     3,     4,     3,     1,     1,     3,
       4,     3,     3,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     4,     6,     5,
       6,     3,     4,     2,     2,     6,     5,     4,     6,     3,
       4,     2,     2,     4,     4,     4,     3,     2,     2,     1,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     2,     3,     3,     3,     3,     3,     2,     2,
       2,     3,     3,     3,     3,     3,     1,     1
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "$end", "error", "$undefined", "\"LTL start marker\"",
  "\"LBT start marker\"", "\"SERE start marker\"",
  "\"BOOLEAN start marker\"", "\"opening parenthesis\"",
  "\"closing parenthesis\"", "\"(...) block\"", "\"{...} block\"",
  "\"{...}! block\"", "\"opening brace\"", "\"closing brace\"",
  "\"closing brace-bang\"", "\"or operator\"", "\"xor operator\"",
  "\"and operator\"", "\"short and operator\"", "\"implication operator\"",
  "\"equivalent operator\"", "\"until operator\"", "\"release operator\"",
  "\"weak until operator\"", "\"strong release operator\"",
  "\"sometimes operator\"", "\"always operator\"", "\"next operator\"",
  "\"not operator\"", "\"X[.] operator\"", "\"F[.] operator\"",
  "\"G[.] operator\"", "\"star operator\"", "\"bracket star operator\"",
  "\"bracket fusion-star operator\"", "\"plus operator\"",
  "\"fusion-plus operator\"", "\"opening bracket for star operator\"",
  "\"opening bracket for fusion-star operator\"",
  "\"opening bracket for equal operator\"",
  "\"opening bracket for goto operator\"", "\"closing bracket\"",
  "\"number for square bracket operator\"", "\"unbounded mark\"",
  "\"separator for square bracket operator\"",
  "\"universal concat operator\"", "\"existential concat operator\"",
  "\"universal non-overlapping concat operator\"",
  "\"existential non-overlapping concat operator\"", "\"first_match\"",
  "\"atomic proposition\"", "\"concat operator\"", "\"fusion operator\"",
  "\"constant true\"", "\"constant false\"", "\"end of formula\"",
  "\"negative suffix\"", "\"positive suffix\"", "\"SVA delay operator\"",
  "\"opening bracket for SVA delay operator\"", "\"##[+] operator\"",
  "\"##[*] operator\"", "'!'", "'&'", "'|'", "'^'", "'i'", "'e'", "'X'",
  "'F'", "'G'", "'U'", "'V'", "'R'", "'W'", "'M'", "'t'", "'f'", "$accept",
  "result", "emptyinput", "enderror", "OP_SQBKT_SEP_unbounded",
  "OP_SQBKT_SEP_opt", "error_opt", "sqbracketargs", "gotoargs",
  "kleen_star", "starargs", "fstarargs", "equalargs", "delayargs",
  "atomprop", "booleanatom", "sere", "bracedsere", "parenthesedsubformula",
  "boolformula", "subformula", "lbtformula", YY_NULLPTR
  };

#if TLYYDEBUG
  const unsigned short
  parser::yyrline_[] =
  {
       0,   290,   290,   295,   300,   305,   307,   312,   317,   322,
     324,   329,   334,   339,   341,   346,   351,   356,   359,   365,
     371,   371,   372,   373,   374,   375,   378,   380,   382,   384,
     386,   390,   392,   394,   396,   398,   400,   402,   405,   410,
     410,   412,   414,   416,   418,   421,   425,   427,   429,   431,
     435,   440,   442,   445,   450,   452,   455,   459,   461,   464,
     472,   473,   474,   478,   480,   483,   484,   496,   497,   506,
     508,   514,   518,   524,   526,   529,   531,   534,   536,   538,
     540,   542,   544,   546,   548,   550,   553,   555,   565,   567,
     577,   579,   588,   597,   606,   624,   642,   656,   658,   671,
     673,   687,   689,   692,   694,   698,   703,   708,   713,   719,
     729,   737,   739,   743,   748,   752,   757,   765,   766,   775,
     777,   781,   786,   791,   796,   802,   804,   806,   808,   810,
     812,   814,   816,   818,   820,   822,   824,   826,   828,   830,
     832,   835,   836,   837,   839,   841,   843,   845,   847,   849,
     851,   853,   855,   857,   859,   861,   863,   865,   867,   869,
     871,   873,   875,   877,   879,   881,   883,   885,   890,   893,
     897,   900,   903,   907,   909,   911,   914,   918,   923,   926,
     929,   933,   935,   937,   939,   941,   944,   947,   949,   951,
     953,   955,   957,   960,   962,   966,   971,   975,   980,   984,
     987,   998,   999,  1001,  1003,  1005,  1007,  1009,  1011,  1013,
    1015,  1017,  1019,  1021,  1023,  1025,  1027,  1029
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // TLYYDEBUG

  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const token_number_type
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    62,     2,     2,     2,     2,    63,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      69,    70,     2,     2,     2,     2,     2,    75,     2,     2,
       2,     2,    73,     2,     2,    71,    72,    74,    68,     2,
       2,     2,     2,     2,    65,     2,     2,     2,     2,     2,
       2,    67,    77,     2,     2,    66,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    76,     2,     2,     2,
       2,     2,     2,     2,    64,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61
    };
    const unsigned user_token_number_max_ = 316;
    const token_number_type undef_token_ = 2;

    if (static_cast<int> (t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // tlyy
#line 3570 "parsetl.cc" // lalr1.cc:1242
#line 1033 "parsetl.yy" // lalr1.cc:1243


void
tlyy::parser::error(const location_type& location, const std::string& message)
{
  error_list.emplace_back(location, message);
}

namespace spot
{
  parsed_formula
  parse_infix_psl(const std::string& ltl_string,
		  environment& env,
		  bool debug, bool lenient)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_LTL,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_infix_boolean(const std::string& ltl_string,
		      environment& env,
		      bool debug, bool lenient)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_BOOL,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_prefix_ltl(const std::string& ltl_string,
		   environment& env,
		   bool debug)
  {
    parsed_formula result(ltl_string);
    flex_set_buffer(ltl_string,
		    tlyy::parser::token::START_LBT,
		    false);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  parsed_formula
  parse_infix_sere(const std::string& sere_string,
		   environment& env,
		   bool debug,
		   bool lenient)
  {
    parsed_formula result(sere_string);
    flex_set_buffer(sere_string,
		    tlyy::parser::token::START_SERE,
		    lenient);
    tlyy::parser parser(result.errors, env, result.f);
    parser.set_debug_level(debug);
    parser.parse();
    flex_unset_buffer();
    return result;
  }

  formula
  parse_formula(const std::string& ltl_string, environment& env)
  {
    parsed_formula pf = parse_infix_psl(ltl_string, env);
    std::ostringstream s;
    if (pf.format_errors(s))
      {
	parsed_formula pg = parse_prefix_ltl(ltl_string, env);
	if (pg.errors.empty())
	  return pg.f;
	else
	  throw parse_error(s.str());
      }
    return pf.f;
  }
}

// Local Variables:
// mode: c++
// End:
