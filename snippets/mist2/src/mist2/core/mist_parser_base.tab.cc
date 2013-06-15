/* A Bison parser, made by GNU Bison 2.5.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2011 Free Software Foundation, Inc.
   
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


/* First part of user declarations.  */


/* Line 293 of lalr1.cc  */
#line 39 "mist_parser_base.tab.cc"


#include "mist_parser_base.tab.hh"

/* User implementation prologue.  */


/* Line 299 of lalr1.cc  */
#line 48 "mist_parser_base.tab.cc"
/* Unqualified %code blocks.  */

/* Line 300 of lalr1.cc  */
#line 15 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"

#include "location.hh"
static int yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc, MistParser* outerParser);
    
/* Helper for wrap pointer into auto_ptr object of corresponded type.*/
template<class T> std::auto_ptr<T> ptr(T* t) {return std::auto_ptr<T>(t);}

/* 
 * While construct base parser, outer parser is not defined.
 * Wraps its fields accessors.
 */
static MistAST* getAST(MistParser* outerParser);
static std::string& getFilename(MistParser* outerParser);



/* Line 300 of lalr1.cc  */
#line 71 "mist_parser_base.tab.cc"

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                               \
 do                                                                    \
   if (N)                                                              \
     {                                                                 \
       (Current).begin = YYRHSLOC (Rhs, 1).begin;                      \
       (Current).end   = YYRHSLOC (Rhs, N).end;                        \
     }                                                                 \
   else                                                                \
     {                                                                 \
       (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;        \
     }                                                                 \
 while (false)
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {

/* Line 382 of lalr1.cc  */
#line 157 "mist_parser_base.tab.cc"

  /// Build a parser object.
  parser::parser (MistParser* outerParser_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      outerParser (outerParser_yyarg)
  {
  }

  parser::~parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 11: /* "TEXT" */

/* Line 480 of lalr1.cc  */
#line 58 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{ delete (yyvaluep->str);};

/* Line 480 of lalr1.cc  */
#line 224 "mist_parser_base.tab.cc"
	break;
      case 12: /* "ID" */

/* Line 480 of lalr1.cc  */
#line 58 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{ delete (yyvaluep->str);};

/* Line 480 of lalr1.cc  */
#line 233 "mist_parser_base.tab.cc"
	break;
      case 13: /* "UNKNOWN" */

/* Line 480 of lalr1.cc  */
#line 58 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{ delete (yyvaluep->str);};

/* Line 480 of lalr1.cc  */
#line 242 "mist_parser_base.tab.cc"
	break;
      case 19: /* "template_internal" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 251 "mist_parser_base.tab.cc"
	break;
      case 20: /* "template_ref" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 260 "mist_parser_base.tab.cc"
	break;
      case 21: /* "template_arg" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 269 "mist_parser_base.tab.cc"
	break;
      case 23: /* "text" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 278 "mist_parser_base.tab.cc"
	break;
      case 24: /* "if_statement" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 287 "mist_parser_base.tab.cc"
	break;
      case 25: /* "if_statement_terminated" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 296 "mist_parser_base.tab.cc"
	break;
      case 27: /* "with_scope" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 305 "mist_parser_base.tab.cc"
	break;
      case 28: /* "with_scope_terminated" */

/* Line 480 of lalr1.cc  */
#line 76 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->astTemplate);};

/* Line 480 of lalr1.cc  */
#line 314 "mist_parser_base.tab.cc"
	break;
      case 29: /* "template_name" */

/* Line 480 of lalr1.cc  */
#line 71 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
	{delete (yyvaluep->templateName);};

/* Line 480 of lalr1.cc  */
#line 323 "mist_parser_base.tab.cc"
	break;

	default:
	  break;
      }
  }

  void
  parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
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
#endif

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* User initialization code.  */
    
/* Line 565 of lalr1.cc  */
#line 42 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
{
    yylloc.initialize(&getFilename(outerParser));
}

/* Line 565 of lalr1.cc  */
#line 420 "mist_parser_base.tab.cc"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, &yylloc, outerParser);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:

/* Line 690 of lalr1.cc  */
#line 129 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {getAST(outerParser)->astSequence = (yysemantic_stack_[(1) - (1)].astSequence);}
    break;

  case 3:

/* Line 690 of lalr1.cc  */
#line 132 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astSequence) = new MistASTTemplateSequence();}
    break;

  case 4:

/* Line 690 of lalr1.cc  */
#line 134 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astSequence) = (yysemantic_stack_[(2) - (1)].astSequence); (yyval.astSequence)->addTemplate(ptr((yysemantic_stack_[(2) - (2)].astTemplate)));}
    break;

  case 9:

/* Line 690 of lalr1.cc  */
#line 143 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = (yysemantic_stack_[(3) - (2)].astTemplate);}
    break;

  case 10:

/* Line 690 of lalr1.cc  */
#line 146 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = new MistASTTemplateRef(ptr((yysemantic_stack_[(1) - (1)].templateName)));}
    break;

  case 11:

/* Line 690 of lalr1.cc  */
#line 148 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yysemantic_stack_[(2) - (2)].astFunction)->templateInternal = ptr((yysemantic_stack_[(2) - (1)].astTemplate)); (yyval.astTemplate) = (yysemantic_stack_[(2) - (2)].astFunction);}
    break;

  case 12:

/* Line 690 of lalr1.cc  */
#line 151 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astFunction) = new MistASTFunction(ptr((yysemantic_stack_[(2) - (2)].str))); }
    break;

  case 13:

/* Line 690 of lalr1.cc  */
#line 153 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astFunction) = new MistASTFunction(ptr((yysemantic_stack_[(3) - (2)].str)), ptr((yysemantic_stack_[(3) - (3)].str))); }
    break;

  case 14:

/* Line 690 of lalr1.cc  */
#line 156 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = new MistASTText(ptr((yysemantic_stack_[(1) - (1)].str)));}
    break;

  case 16:

/* Line 690 of lalr1.cc  */
#line 161 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = (yysemantic_stack_[(2) - (1)].astIf);}
    break;

  case 17:

/* Line 690 of lalr1.cc  */
#line 163 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yysemantic_stack_[(6) - (1)].astIf)->elsePart = ptr((yysemantic_stack_[(6) - (4)].astSequence)); (yyval.astTemplate) = (yysemantic_stack_[(6) - (1)].astIf);}
    break;

  case 18:

/* Line 690 of lalr1.cc  */
#line 165 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yysemantic_stack_[(2) - (2)].astFunction)->templateInternal = ptr((yysemantic_stack_[(2) - (1)].astTemplate)); (yyval.astTemplate) = (yysemantic_stack_[(2) - (2)].astFunction);}
    break;

  case 19:

/* Line 690 of lalr1.cc  */
#line 168 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astIf) = new MistASTIf(ptr((yysemantic_stack_[(6) - (3)].astTemplate)), ptr((yysemantic_stack_[(6) - (5)].astSequence)));}
    break;

  case 20:

/* Line 690 of lalr1.cc  */
#line 170 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astIf) = (yysemantic_stack_[(6) - (1)].astIf); (yyval.astIf)->addConditionPart(ptr((yysemantic_stack_[(6) - (3)].astTemplate)), ptr((yysemantic_stack_[(6) - (5)].astSequence))); }
    break;

  case 21:

/* Line 690 of lalr1.cc  */
#line 173 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = (yysemantic_stack_[(2) - (1)].astTemplate);}
    break;

  case 22:

/* Line 690 of lalr1.cc  */
#line 176 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.astTemplate) = new MistASTWith(ptr((yysemantic_stack_[(7) - (3)].templateName)), ptr((yysemantic_stack_[(7) - (5)].astSequence)));}
    break;

  case 23:

/* Line 690 of lalr1.cc  */
#line 178 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yysemantic_stack_[(2) - (2)].astFunction)->templateInternal = ptr((yysemantic_stack_[(2) - (1)].astTemplate)); (yyval.astTemplate) = (yysemantic_stack_[(2) - (2)].astFunction);}
    break;

  case 24:

/* Line 690 of lalr1.cc  */
#line 181 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.templateName) = new MistASTTemplateName(ptr((yysemantic_stack_[(1) - (1)].str)));}
    break;

  case 25:

/* Line 690 of lalr1.cc  */
#line 183 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.templateName) = new MistASTTemplateName(ptr((yysemantic_stack_[(2) - (2)].str)), true);}
    break;

  case 26:

/* Line 690 of lalr1.cc  */
#line 185 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"
    {(yyval.templateName) = (yysemantic_stack_[(3) - (1)].templateName); (yyval.templateName)->addComponent(ptr((yysemantic_stack_[(3) - (3)].str)));}
    break;



/* Line 690 of lalr1.cc  */
#line 679 "mist_parser_base.tab.cc"
	default:
          break;
      }
    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
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

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (int, int)
  {
    return YY_("syntax error");
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char parser::yypact_ninf_ = -14;
  const signed char
  parser::yypact_[] =
  {
       -14,     2,    26,   -14,    11,   -14,   -14,   -14,   -14,   -14,
       1,    46,   -14,    13,    -6,    -6,   -14,    35,    18,    -7,
     -14,    37,   -14,   -14,    36,    -6,   -14,   -14,    21,    -4,
     -14,   -14,   -14,    43,    19,   -14,    24,   -14,   -14,   -14,
     -14,    28,   -14,    30,    31,    -2,    33,    11,     9,   -14,
      11,   -14
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  parser::yydefact_[] =
  {
         3,     0,     2,     1,     0,    14,     4,     5,     6,     7,
       0,     0,     8,     0,     0,     0,    24,     0,     0,    10,
      15,     0,    18,    16,     0,     0,    21,    23,     0,     0,
      25,     9,    11,     0,    12,     3,     0,     3,     3,    26,
      13,     0,     3,     0,     0,     0,     0,    19,     0,    17,
      20,    22
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  parser::yypgoto_[] =
  {
       -14,   -14,     8,   -14,   -14,   -13,    38,   -14,   -14,   -14,
     -14,   -14,   -14,    41
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  parser::yydefgoto_[] =
  {
        -1,     1,     2,     6,     7,    18,    32,     8,     9,    10,
      11,    12,    13,    19
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char parser::yytable_ninf_ = -1;
  const unsigned char
  parser::yytable_[] =
  {
        38,    28,     3,    14,    49,    20,    16,    15,    33,    17,
      16,    33,    36,    17,    14,    21,    14,    26,    15,    51,
      15,    16,    31,    16,    17,    37,    17,    21,    42,     4,
      40,    45,    21,    47,    48,    21,    50,     5,    21,     5,
      35,     5,     5,    41,     5,    43,    44,    30,    22,    34,
      46,    27,    23,    24,    25,    39,    29
  };

  /* YYCHECK.  */
  const unsigned char
  parser::yycheck_[] =
  {
         4,    14,     0,     5,     6,     4,    12,     9,    15,    15,
      12,    15,    25,    15,     5,    14,     5,     4,     9,    10,
       9,    12,     4,    12,    15,     4,    15,    14,     4,     3,
      11,     3,    14,     3,     3,    14,     3,    11,    14,    11,
       4,    11,    11,    35,    11,    37,    38,    12,    10,    12,
      42,    13,     6,     7,     8,    12,    15
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  parser::yystos_[] =
  {
         0,    17,    18,     0,     3,    11,    19,    20,    23,    24,
      25,    26,    27,    28,     5,     9,    12,    15,    21,    29,
       4,    14,    22,     6,     7,     8,     4,    22,    21,    29,
      12,     4,    22,    15,    12,     4,    21,     4,     4,    12,
      11,    18,     4,    18,    18,     3,    18,     3,     3,     6,
       3,    10
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    58,    46
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  parser::yyr1_[] =
  {
         0,    16,    17,    18,    18,    19,    19,    19,    19,    20,
      21,    21,    22,    22,    23,    24,    25,    25,    25,    26,
      26,    27,    28,    28,    29,    29,    29
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  parser::yyr2_[] =
  {
         0,     2,     1,     0,     2,     1,     1,     1,     1,     3,
       1,     2,     2,     3,     1,     2,     2,     6,     2,     6,
       6,     2,     7,     2,     1,     2,     3
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const parser::yytname_[] =
  {
    "$end", "error", "$undefined", "BEGIN_MARKER", "END_MARKER",
  "IF_KEYWORD", "ENDIF_KEYWORD", "ELSE_KEYWORD", "ELSEIF_KEYWORD",
  "WITH_KEYWORD", "ENDWITH_KEYWORD", "TEXT", "ID", "UNKNOWN", "':'", "'.'",
  "$accept", "whole_template", "template_sequence", "template_internal",
  "template_ref", "template_arg", "func_apply", "text", "if_statement",
  "if_statement_terminated", "if_statement_common", "with_scope",
  "with_scope_terminated", "template_name", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const parser::rhs_number_type
  parser::yyrhs_[] =
  {
        17,     0,    -1,    18,    -1,    -1,    18,    19,    -1,    20,
      -1,    23,    -1,    24,    -1,    27,    -1,     3,    21,     4,
      -1,    29,    -1,    21,    22,    -1,    14,    12,    -1,    14,
      12,    11,    -1,    11,    -1,    25,     4,    -1,    26,     6,
      -1,    26,     7,     4,    18,     3,     6,    -1,    25,    22,
      -1,     3,     5,    21,     4,    18,     3,    -1,    26,     8,
      21,     4,    18,     3,    -1,    28,     4,    -1,     3,     9,
      29,     4,    18,     3,    10,    -1,    28,    22,    -1,    12,
      -1,    15,    12,    -1,    29,    15,    12,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  parser::yyprhs_[] =
  {
         0,     0,     3,     5,     6,     9,    11,    13,    15,    17,
      21,    23,    26,    29,    33,    35,    38,    41,    48,    51,
      58,    65,    68,    76,    79,    81,    84
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned char
  parser::yyrline_[] =
  {
         0,   128,   128,   132,   133,   136,   137,   138,   139,   142,
     145,   147,   150,   152,   155,   158,   160,   162,   164,   167,
     169,   172,   175,   177,   180,   182,   184
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    15,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    14,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int parser::yyeof_ = 0;
  const int parser::yylast_ = 56;
  const int parser::yynnts_ = 14;
  const int parser::yyempty_ = -2;
  const int parser::yyfinal_ = 3;
  const int parser::yyterror_ = 1;
  const int parser::yyerrcode_ = 256;
  const int parser::yyntokens_ = 16;

  const unsigned int parser::yyuser_token_number_max_ = 268;
  const parser::token_number_type parser::yyundef_token_ = 2;


} // yy

/* Line 1136 of lalr1.cc  */
#line 1114 "mist_parser_base.tab.cc"


/* Line 1138 of lalr1.cc  */
#line 186 "/home/andrew/kedr/aux-sources/snippets/mist2/src/mist2/core/mist_parser_base.yy"


/* Error method of the parser */
#include <stdexcept>
void yy::parser::error(const yy::location& yyloc,
    const std::string& what)
{
    std::cerr << yyloc << ": " << what << std::endl;
    throw std::runtime_error("Template parsing failed");
}

/* Implementation of scanner routine*/
#include "mist_scanner.hh"
#include "mist_parser.hh"
int yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc, MistParser* outerParser)
{
    return outerParser->scanner.yylex(yylval, yylloc);
}

/* Implementation of MistParser methods */
MistParser::MistParser() : parserBase(this) {}

void MistParser::parse(std::istream& stream, MistAST& ast, const std::string& filename)
{
    scanner.setStream(stream);
    this->ast = &ast;
    this->filename = filename;
    
    parserBase.parse();
}

/* Implementation of wrappers */
MistAST* getAST(MistParser* outerParser)
    {return outerParser->ast;}
std::string& getFilename(MistParser* outerParser)
    {return outerParser->filename;}

