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

#include "Puma/CLexer.h"

namespace Puma {

  void CLexer::add_keywords (lexertl::rules &rules) {
    add_c89_keywords (rules);
    add_c99_keywords (rules);
  }


  // different kinds of keywords; selected depending on the standard to be supported
  void CLexer::add_c89_keywords (lexertl::rules &rules) {
    // C keywords
    rules.push ("asm", TOK_ASM, LID(Token::keyword_id));
    rules.push ("auto", TOK_AUTO, LID(Token::keyword_id));
    rules.push ("break", TOK_BREAK, LID(Token::keyword_id));
    rules.push ("case", TOK_CASE, LID(Token::keyword_id));
    rules.push ("char", TOK_CHAR, LID(Token::keyword_id));
    rules.push ("const", TOK_CONST, LID(Token::keyword_id));
    rules.push ("continue", TOK_CONTINUE, LID(Token::keyword_id));
    rules.push ("default", TOK_DEFAULT, LID(Token::keyword_id));
    rules.push ("do", TOK_DO, LID(Token::keyword_id));
    rules.push ("double", TOK_DOUBLE, LID(Token::keyword_id));
    rules.push ("else", TOK_ELSE, LID(Token::keyword_id));
    rules.push ("enum", TOK_ENUM, LID(Token::keyword_id));
    rules.push ("extern", TOK_EXTERN, LID(Token::keyword_id));
    rules.push ("float", TOK_FLOAT, LID(Token::keyword_id));
    rules.push ("for", TOK_FOR, LID(Token::keyword_id));
    rules.push ("goto", TOK_GOTO, LID(Token::keyword_id));
    rules.push ("if", TOK_IF, LID(Token::keyword_id));
    rules.push ("inline", TOK_INLINE, LID(Token::keyword_id));
    rules.push ("int", TOK_INT, LID(Token::keyword_id));
    rules.push ("long", TOK_LONG, LID(Token::keyword_id));
    rules.push ("register", TOK_REGISTER, LID(Token::keyword_id));
    rules.push ("return", TOK_RETURN, LID(Token::keyword_id));
    rules.push ("short", TOK_SHORT, LID(Token::keyword_id));
    rules.push ("signed", TOK_SIGNED, LID(Token::keyword_id));
    rules.push ("sizeof", TOK_SIZEOF, LID(Token::keyword_id));
    rules.push ("static", TOK_STATIC, LID(Token::keyword_id));
    rules.push ("struct", TOK_STRUCT, LID(Token::keyword_id));
    rules.push ("switch", TOK_SWITCH, LID(Token::keyword_id));
    rules.push ("typedef", TOK_TYPEDEF, LID(Token::keyword_id));
    rules.push ("union", TOK_UNION, LID(Token::keyword_id));
    rules.push ("unsigned", TOK_UNSIGNED, LID(Token::keyword_id));
    rules.push ("void", TOK_VOID, LID(Token::keyword_id));
    rules.push ("volatile", TOK_VOLATILE, LID(Token::keyword_id));
    rules.push ("while", TOK_WHILE, LID(Token::keyword_id));
  }


  void CLexer::add_c99_keywords (lexertl::rules &rules) {
    rules.push ("_Bool", TOK_C_BOOL, LID(Token::keyword_id));
    rules.push ("restrict", TOK_RESTRICT, LID(Token::keyword_id));
  }


  void CLexer::add_rules (lexertl::rules &rules) {
    rules.push ("[ \\t]+[\\n\\r\\f\\v]*", TOK_WSPACE, LID(Token::white_id));
    rules.push ("[\\n\\r\\f\\v]+", TOK_WSPACE, LID(Token::white_id));

    // ------------
    // preprocessor
    // ------------

    // a single line comment
    rules.push ("\\/\\/[^\\n\\r\\f\\v]*", TOK_CCSINGLE, LID(Token::comment_id));

    // a multi-line comment
    rules.push ("\\/\\*(.|\\n)*?\\*\\/", TOK_CCOMMENT, LID(Token::comment_id));

    // this ugly DirPrefix expression allows something like this: "# /*abc*/ include <stdio.h>"
    rules.insert_macro("DirPrefix", "([ \\t]|(\\/\\*([^*]|\\*+[^/*])*\\*+\\/))*#([ \\t]|(\\/\\*([^*]|\\*+[^/*])*\\*+\\/))*");

    rules.push ("^{DirPrefix}define", TOK_PRE_DEFINE, LID(Token::pre_id));
    rules.push ("^{DirPrefix}assert", TOK_PRE_ASSERT, LID(Token::pre_id));
    rules.push ("^{DirPrefix}unassert", TOK_PRE_UNASSERT, LID(Token::pre_id));
    rules.push ("^{DirPrefix}if", TOK_PRE_IF, LID(Token::pre_id));
    rules.push ("^{DirPrefix}elif", TOK_PRE_ELIF, LID(Token::pre_id));
    rules.push ("^{DirPrefix}warning", TOK_PRE_WARNING, LID(Token::pre_id));
    rules.push ("^{DirPrefix}error", TOK_PRE_ERROR, LID(Token::pre_id));
    rules.push ("^{DirPrefix}include", TOK_PRE_INCLUDE, LID(Token::pre_id));
    rules.push ("^{DirPrefix}include_next", TOK_PRE_INCLUDE_NEXT, LID(Token::pre_id));
    rules.push ("^{DirPrefix}ifdef", TOK_PRE_IFDEF, LID(Token::pre_id));
    rules.push ("^{DirPrefix}ifndef", TOK_PRE_IFNDEF, LID(Token::pre_id));
    rules.push ("^{DirPrefix}else", TOK_PRE_ELSE, LID(Token::pre_id));
    rules.push ("^{DirPrefix}endif", TOK_PRE_ENDIF, LID(Token::pre_id));
    rules.push ("^{DirPrefix}undef", TOK_PRE_UNDEF, LID(Token::pre_id));

    // Win specific preprocessor directives
    rules.push ("^{DirPrefix}import", TOK_PRE_INCLUDE, LID(Token::pre_id));

    // compiler directives
    rules.push ("^{DirPrefix}pragma", 1, LID(Token::dir_id));
    rules.push ("^{DirPrefix}line", 1, LID(Token::dir_id));
    rules.push ("^{DirPrefix}ident", 1, LID(Token::dir_id));
    rules.push ("^{DirPrefix}", 1, LID(Token::dir_id));

    // preprocessor macro operations
    rules.push ("#", TOK_MO_HASH, LID(Token::macro_op_id));
    rules.push ("##", TOK_MO_HASHHASH, LID(Token::macro_op_id));

    // ----------
    // core rules
    // ----------

    // macros
    rules.insert_macro("Dec", "\\d");
    rules.insert_macro("Hex", "[0-9a-fA-F]");
    rules.insert_macro("U", "[uU]");
    rules.insert_macro("L", "[lL]");
    rules.insert_macro("Exp", "[Ee]");
    rules.insert_macro("F", "[fF]");

    // nonreserved tokens:
    rules.push ("@", TOK_AT, LID(Token::cpp_id));

    // an integer constant:
    rules.push ("0", TOK_ZERO_VAL, LID(Token::cpp_id));
    rules.push ("({Dec}+|(0[xX]?{Hex}+))({U}|{L}|{L}{U}|{U}{L}|{L}{L}|{U}{L}{L}|{L}{U}{L}|{L}{L}{U})?", TOK_INT_VAL, LID(Token::cpp_id));

    // a floating point constant:
    rules.push ("{Dec}+(\\.{Dec}*)?({Exp}([\\+\\-])?{Dec}+)?({L}|{F})?", TOK_FLT_VAL, LID(Token::cpp_id));
    rules.push ("\\.{Dec}+({Exp}([\\+\\-])?{Dec}+)?({L}|{F})?", TOK_FLT_VAL, LID(Token::cpp_id));

    // a hex floating point constant:
    rules.push ("0[xX]{Hex}+(\\.{Hex}*)?[pP]([\\+\\-])?{Dec}+({L}|{F})?", TOK_FLT_VAL, LID(Token::cpp_id));
    rules.push ("0[xX]\\.{Hex}+[pP]([\\+\\-])?{Dec}+({L}|{F})?", TOK_FLT_VAL, LID(Token::cpp_id));

    // a string or character constant:
    rules.push ("(L)?\\\"([^\\\\\\\"]|\\\\.)*\\\"", TOK_STRING_VAL, LID(Token::cpp_id));
    rules.push ("(L)?\\\'([^\\\\\\\']|\\\\.)*\\\'", TOK_CHAR_VAL, LID(Token::cpp_id));

    // operators
  #define LIT(x) "\"" x "\""
    rules.push (LIT(","), TOK_COMMA, LID(Token::comma_id));
    rules.push (LIT("="), TOK_ASSIGN, LID(Token::cpp_id));
    rules.push (LIT("?"), TOK_QUESTION, LID(Token::cpp_id));
    rules.push (LIT("|"), TOK_OR, LID(Token::cpp_id));
    rules.push (LIT("^"), TOK_ROOF, LID(Token::cpp_id));
    rules.push (LIT("&"), TOK_AND, LID(Token::cpp_id));
    rules.push (LIT("+"), TOK_PLUS, LID(Token::cpp_id));
    rules.push (LIT("-"), TOK_MINUS, LID(Token::cpp_id));
    rules.push (LIT("*"), TOK_MUL, LID(Token::cpp_id));
    rules.push (LIT("/"), TOK_DIV, LID(Token::cpp_id));
    rules.push (LIT("%"), TOK_MODULO, LID(Token::cpp_id));
    rules.push (LIT("<"), TOK_LESS, LID(Token::cpp_id));
    rules.push (LIT(">"), TOK_GREATER, LID(Token::cpp_id));
    rules.push (LIT("("), TOK_OPEN_ROUND, LID(Token::open_id));
    rules.push (LIT(")"), TOK_CLOSE_ROUND, LID(Token::close_id));
    rules.push (LIT("["), TOK_OPEN_SQUARE, LID(Token::cpp_id));
    rules.push (LIT("]"), TOK_CLOSE_SQUARE, LID(Token::cpp_id));
    rules.push (LIT("{"), TOK_OPEN_CURLY, LID(Token::cpp_id));
    rules.push (LIT("}"), TOK_CLOSE_CURLY, LID(Token::cpp_id));
    rules.push (LIT(";"), TOK_SEMI_COLON, LID(Token::cpp_id));
    rules.push (LIT(":"), TOK_COLON, LID(Token::cpp_id));
    rules.push (LIT("!"), TOK_NOT, LID(Token::cpp_id));
    rules.push (LIT("~"), TOK_TILDE, LID(Token::cpp_id));
    rules.push (LIT("."), TOK_DOT, LID(Token::cpp_id));

    rules.push (LIT("*="), TOK_MUL_EQ, LID(Token::cpp_id));
    rules.push (LIT("/="), TOK_DIV_EQ, LID(Token::cpp_id));
    rules.push (LIT("%="), TOK_MOD_EQ, LID(Token::cpp_id));
    rules.push (LIT("+="), TOK_ADD_EQ, LID(Token::cpp_id));
    rules.push (LIT("-="), TOK_SUB_EQ, LID(Token::cpp_id));
    rules.push (LIT("<<="), TOK_LSH_EQ, LID(Token::cpp_id));
    rules.push (LIT(">>="), TOK_RSH_EQ, LID(Token::cpp_id));
    rules.push (LIT("&="), TOK_AND_EQ, LID(Token::cpp_id));
    rules.push (LIT("^="), TOK_XOR_EQ, LID(Token::cpp_id));
    rules.push (LIT("|="), TOK_IOR_EQ, LID(Token::cpp_id));
    rules.push (LIT("||"), TOK_OR_OR, LID(Token::cpp_id));
    rules.push (LIT("&&"), TOK_AND_AND, LID(Token::cpp_id));
    rules.push (LIT("=="), TOK_EQL, LID(Token::cpp_id));
    rules.push (LIT("!="), TOK_NEQ, LID(Token::cpp_id));
    rules.push (LIT("<="), TOK_LEQ, LID(Token::cpp_id));
    rules.push (LIT(">="), TOK_GEQ, LID(Token::cpp_id));
    rules.push (LIT("<<"), TOK_LSH, LID(Token::cpp_id));
    rules.push (LIT(">>"), TOK_RSH, LID(Token::cpp_id));
    rules.push (LIT(".*"), TOK_DOT_STAR, LID(Token::cpp_id));
    rules.push (LIT("->*"), TOK_PTS_STAR, LID(Token::cpp_id));
    rules.push (LIT("++"), TOK_INCR, LID(Token::cpp_id));
    rules.push (LIT("--"), TOK_DECR, LID(Token::cpp_id));
    rules.push (LIT("->"), TOK_PTS, LID(Token::cpp_id));
    rules.push (LIT("::"), TOK_COLON_COLON, LID(Token::cpp_id));
    rules.push (LIT("..."), TOK_ELLIPSIS, LID(Token::cpp_id));

    // Keywords
    add_keywords (rules);

    // Identifiers
    rules.insert_macro ("UniversalChar", "\\\\(u{Hex}{4}|U{Hex}{8})");
    rules.insert_macro ("Alpha", "[a-zA-Z_\\$]|{UniversalChar}");

    rules.push ("{Alpha}({Alpha}|\\d)*", TOK_ID, LID(Token::identifier_id));
  }

} // namespace Puma
