/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "Parser.y" /* yacc.c:1915  */

#include "Expression.h"

#line 48 "Parser.h" /* yacc.c:1915  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INTEGER = 258,
    FLOAT = 259,
    STRING = 260,
    NOTE_NO = 261,
    LOCATION = 262,
    MB_LENGTH = 263,
    RESOLUTION = 264,
    TITLE = 265,
    TIME = 266,
    TEMPO = 267,
    MARKER = 268,
    SOUND = 269,
    SELECT = 270,
    CHANNEL = 271,
    VELOCITY = 272,
    GATETIME = 273,
    CUTOFF = 274,
    NOTE = 275,
    STEP = 276,
    FROM = 277,
    TO = 278,
    REPLACE = 279,
    MIX = 280,
    OFFSET = 281,
    LENGTH = 282,
    REST = 283,
    TIE = 284,
    PLUS = 285,
    MINUS = 286,
    DIVISION = 287,
    MULTIPLY = 288,
    ASSIGN = 289,
    SEMICOLON = 290,
    COMMA = 291,
    LPAREN = 292,
    RPAREN = 293,
    LCURLY = 294,
    RCURLY = 295,
    IDENTIFIER = 296,
    EOL = 297,
    TIME_SIGN = 298,
    SOUND_SELECT = 299,
    INTEGER_LIST = 300,
    GATETIME_CUTOFF = 301,
    NOTE_BLOCK = 302,
    NOTE_NO_LIST = 303,
    PATTERN_DEFINE = 304,
    PATTERN_BLOCK = 305,
    PATTERN_EXPAND = 306,
    PATTERN_EXTEND_BLOCK = 307
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 24 "Parser.y" /* yacc.c:1915  */

    int i;
    float f;
    char *s;
    Expression *expression;

#line 120 "Parser.h" /* yacc.c:1915  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void *scanner, Expression **expression);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
