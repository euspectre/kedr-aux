/* No wrapping after EOF */
%option noyywrap
/* These functions(macros) are not used, do not generate code for them */
%option nounput
%option noinput
%option noyylineno

/* Use state stack*/
%option stack
/* ...but without yy_top_state - do not generate code for it */
%option noyy_top_state

/* Use reentrant scanner for free it after parsing is ended. */
%option reentrant

%top{
#include "mist_scanner.hh"
#include "mist_parser_base.tab.hh"
}
/* Extra data is pointer to the parsed stream. */
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

#include <iostream>
#include <string>
#include <stdexcept>
#include <cassert> /* assertions */

typedef yy::parser::token token;

/* Add parameters to yylex() call. */
#undef YY_DECL
#define YY_DECL int yylex(yyscan_t yyscanner, yy::parser::semantic_type* yylval, \
yy::location* yylloc)

/* Redefine YY_INPUT for use 'std::istream' instead of 'FILE*'. */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) {   \
    std::istream& s = *yyextra;             \
    if(s.eof() || s.fail()) result = 0;     \
    else if(s.read(buf, max_size).bad())    \
        throw std::runtime_error("Error occures while read template stream."); \
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
%}

id              [a-zA-Z_][a-zA-Z_0-9]*

space           [ \t]
newline         \n

/* Inside comments */
%x COMMENTS

/* Currently inside directive */
%x DIRECTIVE

/*
 * Currently inside quotes. Outer state is stored at the top of the stack.
 */
%x STRING

%%
    /* Make step() on location each time when new token is requested. */
%{
    yylloc->step();
%}


<COMMENTS>{
"$>"           yy_pop_state(yyscanner); yylloc->step();
"$"            yylloc->step();
"\n"           yylloc->step();
[^$\n]+        yylloc->step();
<<EOF>>        throw std::logic_error("Unexpected EOF while parse comments.");
}

<STRING>{
"\""            {
    yy_pop_state(yyscanner);
    switch(YY_START)
    {
    case DIRECTIVE:
        return token::TEXT;
    break;
    default:
        throw std::logic_error("Unexpected state while parsing string is finished.");
    }
    }

"\\\""  yylval->str->push_back('\"');
"\\\\"  yylval->str->push_back('\\');
"\\n"   yylval->str->push_back('\n');
"\\"    {
            std::cerr << "Unrecognized escape sequence." << std::endl;
            yylval->str->push_back('\\');
        }

"\n"        |
[^\"\\\n]+  yylval->str->append(yytext, yyleng);

<<EOF>>     throw std::logic_error("Unexpected EOF while parse string in qoutes.");
}

<INITIAL>{
"<$!"       yy_push_state(COMMENTS, yyscanner); yylloc->step();
"<$"        BEGIN(DIRECTIVE); return token::BEGIN_MARKER;
"<"         |
[^<\n]+     |
"\n"        yylval->str = new std::string(yytext, yyleng); return token::TEXT;
<<EOF>>         return 0;
}

<DIRECTIVE>{
{space}+        yylloc->step();
{newline}+      yylloc->step();
"$>"            BEGIN(INITIAL); return token::END_MARKER;

if              return token::IF_KEYWORD;
endif           return token::ENDIF_KEYWORD;
else            return token::ELSE_KEYWORD;
concat          return token::CONCAT_KEYWORD;
join            return token::JOIN_KEYWORD;
with            return token::WITH_KEYWORD;
endwith         return token::ENDWITH_KEYWORD;

{id}            {
                    yylval->str = new std::string(yytext, yyleng);
                    return token::ID;
                }
"\."            return '.';
":"             return ':';

"\""            {
                    yylval->str = new std::string();
                    yy_push_state(STRING, yyscanner);
                }

.               {
                    yylval->str = new std::string(1, *yytext);
                    return token::UNKNOWN;
                }
<<EOF>>         throw std::logic_error("Unexpected EOF while parse directive.");
}

%%

/* As if 'Scanner' class has 'scanner' field of true type. */
#define scanner (yyscan_t)_scanner

MistScanner::MistScanner()
{
    if(yylex_init((yyscan_t*)&_scanner) != 0)
        throw std::runtime_error("Failed to initialize scanner");
}

MistScanner::~MistScanner(void)
{
    yylex_destroy(scanner);
}

void MistScanner::setStream(std::istream& s)
{
    yyset_extra(&s, scanner);
}

int MistScanner::yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc)
{
    return ::yylex(scanner, yylval, yylloc);
}
