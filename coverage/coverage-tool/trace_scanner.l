/* trace_scanner.l - scanner for the trace file. */

/*      
 *      Copyright (C) 2012, Institute for System Programming
 *                          of the Russian Academy of Sciences (ISPRAS)
 *      Author:
 *          Andrey Tsyvarev <tsyvarev@ispras.ru>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

/* No wrapping after EOF */
%option noyywrap
/* These functions(macros) are not used, do not generate code for them */
%option nounput
%option noinput
%option noyylineno

/* Doesn't use state stack*/
%option nostack

/* Use reentrant scanner for free it after parsing is ended. */
%option reentrant

%top{
#include "trace_scanner.hh"
#include "trace_parser_base.tab.hh"
}

/* Extra data is really pointer to the input stream */
%option extra-type="std::istream*"

/* 
 * No default action should be generated, so LEX check that we define
 * actions for all possible character sequences.
 */
%option nodefault
/*
 * Enable LEX warnings - for selfcontrol.
 */
%option warn
%{

#include <stdexcept>
#include <iostream>
#include <cassert> /* assertions */

typedef yy::parser::token token;

/* Add parameters to yylex() call. */
#undef YY_DECL
#define YY_DECL int yylex(yyscan_t yyscanner, yy::parser::semantic_type* yylval, \
yy::location* yylloc)


/* Redefine YY_INPUT for use 'std::istream' instead of 'FILE*'. */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) {   \
    std::istream& s = *yyextra;              \
    if(s.eof() || s.fail()) result = 0;     \
    else if(s.read(buf, max_size).bad())    \
        throw std::runtime_error("Error occures while read trace file."); \
    else result = s.gcount(); }

/*
 * Use macro for update token location each time when rule is matched.
 * 
 * These actions are correct for any non-skipping match.
 * 
 * Action which skip characters should additionally perform
 * yylloc->step();
 *
 * NOTE: This macro relies on the fact that all patterns including
 * newline characters doesn't include non-newline characters.
 */
#define YY_USER_ACTION \
    if(*yytext != '\n') yylloc->columns (yyleng);\
    else yylloc->lines(yyleng);



/* 
 * Convert text chunk to long integer.
 * 
 * The thing is that we cannot use strtol() because text chunk is not
 * null-terminated.
 * 
 * NOTE: For simplicity, function expects all symbols in text chunk
 * are valid for decimal integer.
 */
static unsigned long str_to_long(const char* str, int str_len);
%}

/* State when parse directive parameters */
%x DIRECTIVE

/* Non-zero integer */
integernz   [1-9][0-9]*

/* 
 * Name of the function or test.
 * 
 * Static variant of function may have suffix like ".isra", so dot ('.')
 * is also included into possible characters for id.
 */
id          [[:alpha:]_][[:alnum:]_.]*

/* 
 * Absolute path (to the source file).
 * 
 * TODO: possible characters for filename.
 */
path        \/[[:alnum:]_\/@\-\.]+

%%
    /* Make step() on location each time when new token is requested. */
%{
    yylloc->step();
%}

<INITIAL>{
    "TN"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_TN;
                }
    "SF"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_SF;
                }
"end_of_record" {
                    BEGIN(DIRECTIVE);
                    return token::KEY_EOR;
                }
    "FNF"       {
                    BEGIN(DIRECTIVE);
                    return token::KEY_FNF;
                }
    "FNH"       {
                    BEGIN(DIRECTIVE);
                    return token::KEY_FNH;
                }
    "FN"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_FN;
                }
    "FNDA"      {
                    BEGIN(DIRECTIVE);
                    return token::KEY_FNDA;
                }
    "BRDA"      {
                    BEGIN(DIRECTIVE);
                    return token::KEY_BRDA;
                }
    "BRF"       {
                    BEGIN(DIRECTIVE);
                    return token::KEY_BRF;
                }
    "BRH"       {
                    BEGIN(DIRECTIVE);
                    return token::KEY_BRH;
                }
    "DA"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_DA;
                }
    "LF"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_LF;
                }
    "LH"        {
                    BEGIN(DIRECTIVE);
                    return token::KEY_LH;
                }

                
    "\n"        yylloc->step();
[[:alpha:]]+    {
                    trace_parse_error(*yylloc, "Invalid directive: "
                        + std::string(yytext, yyleng) + ".");
                }
    .           {
                    trace_parse_error(*yylloc, std::string("Invalid character '")
                        + *yytext + "' while directive expected.");
                }
                

    <<EOF>>     return 0;
}

<DIRECTIVE>{
0                   {
                        yylval->number = 0;
                        return token::NUMBER;
                    }

{integernz}         {
                        yylval->number = str_to_long(yytext, yyleng);
                        return token::NUMBER;
                    }

"-"                 return '-';
","                 return ',';
":"                 return ':';

{id}                {
                        yylval->outer_string.p = yytext;
                        yylval->outer_string.l = yyleng;
                        return token::ID;
                    }
{path}              {
                        yylval->outer_string.p = yytext;
                        yylval->outer_string.l = yyleng;
                        return token::PATH;
                    }
"\n"                {
                        yylloc->step();
                        BEGIN(INITIAL);
                    }
.                   {
                        trace_parse_error(*yylloc, "Unknown character in directive");
                    }
<<EOF>>             return 0;
}
%%

/* As if this is field of the scanner class. */
#define scanner (yyscan_t&)_scanner

TraceScanner::TraceScanner()
{
    if(yylex_init(&scanner) != 0)
        throw std::runtime_error("Failed to initialize scanner");
}

TraceScanner::~TraceScanner(void)
{
    yylex_destroy(scanner);
}

void TraceScanner::setStream(std::istream& is)
{
    yyset_extra(&is, scanner);
    /* 
     * Also restart scanner for flush its state.
     * 
     * NB: Start condition is NOT set to INITIAL by that function.
     */
    yyrestart(NULL, scanner);
}

int TraceScanner::yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc)
{
    return ::yylex(scanner, yylval, yylloc);
}

unsigned long str_to_long(const char* str, int str_len)
{
    unsigned long result = 0;
    for(int i = 0; i < str_len; i++)
        result = result * 10 + (str[i] - '0');
    
    return result;
}

void trace_parse_error(const yy::location& yyloc,
    const std::string& what)
{
    std::cerr << yyloc << ": " << what << std::endl;
    throw std::runtime_error("Trace parsing failed");
}
