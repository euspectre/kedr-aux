// trace_parser_base.yy - parser for the trace file.

//      
//      Copyright (C) 2012, Institute for System Programming
//                          of the Russian Academy of Sciences (ISPRAS)
//      Author:
//          Andrey Tsyvarev <tsyvarev@ispras.ru>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

/* Generate C++ parser */
%skeleton "lalr1.cc"
%require "2.4"
%defines

%locations

%code requires{
#include "trace_parser.hh"
class TraceScanner;
}

%code {
#include "location.hh"
static int yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc, TraceScanner* scanner);

static TraceEventProcessor* getEventProcessor(TraceParser::Impl* parserImpl);
/* Shortcat for use in parser */
#define EP getEventProcessor(parserImpl)

}
/* 
 * Reentrant LEX scanner accept additional parameter
 */
%lex-param  {TraceScanner* scanner}
%parse-param {TraceScanner* scanner}
/* Parser accept indirect reference to the TraceEventProcessor */
%parse-param {TraceParser::Impl* parserImpl}

/********************* Tokens ******************************************/

/* Keywords for directives */
%token KEY_TN KEY_SF KEY_EOR KEY_FNF KEY_FNH KEY_FN KEY_FNDA
%token KEY_DA KEY_LF KEY_LH KEY_BRDA KEY_BRF KEY_BRH

%union {
    unsigned long number;
}

%token <number> NUMBER

%union {
    struct
    {
        const char* p;
        int l;
    } outer_string;
}

%token <outer_string> PATH ID

/************************ Grouping ************************************/

%start trace

%%
trace               : /* empty */
                    | trace test
                    
test                : test_noend
                        {
                            /* Use end position here! */
                            EP->onTestEnd(@$.end.line);
                        }
test_noend          : directive_tn
                    | test_noend source

source              : source_noend directive_eor
                        {/* Source end event is triggered in directive_eor */}
source_noend        : directive_sf
                    | source_noend directive

directive           : directive_fnf
                    | directive_fnh
                    | directive_fn
                    | directive_fnda
                    | directive_da
                    | directive_lf
                    | directive_lh
                    | directive_brda
                    | directive_brf
                    | directive_brh


directive_tn        : KEY_TN ':' ID
                        {EP->onTestStart(std::string($3.p, $3.l), @$.begin.line);}

directive_tn        : KEY_TN ':'
                        {EP->onTestStart(std::string(), @$.begin.line);}

directive_sf        : KEY_SF ':' PATH
                        {EP->onSourceStart(std::string($3.p, $3.l), @$.begin.line);}
directive_eor       : KEY_EOR
                        {EP->onSourceEnd(@$.begin.line);}

directive_fnf       : KEY_FNF ':' NUMBER
                        {EP->onFunctionsTotal($3, @$.begin.line);}
directive_fnh       : KEY_FNH ':' NUMBER
                        {EP->onFunctionsTotalHit($3, @$.begin.line);}

directive_fn        : KEY_FN ':' NUMBER ',' ID
                        {EP->onFunction(std::string($5.p, $5.l), $3, @$.begin.line);}

directive_fnda      : KEY_FNDA ':' NUMBER ',' ID
                        {EP->onFunctionCounter(std::string($5.p, $5.l), $3, @$.begin.line);}

directive_da        : KEY_DA ':' NUMBER ',' NUMBER
                        {EP->onLineCounter($3, $5, @$.begin.line);}
					| KEY_DA ':' NUMBER ',' '-' NUMBER
                        {
							// TODO: Add option to not print this warning
							//std::cerr << @$ << " Warning: negative line counter. Assume it to be zero." << std::endl; 
							EP->onLineCounter($3, 0, @$.begin.line);
						}
directive_lf        : KEY_LF ':' NUMBER
                        {EP->onLinesTotal($3, @$.begin.line);}

directive_lh        : KEY_LH ':' NUMBER
                        {EP->onLinesTotalHit($3, @$.begin.line);}

directive_brda      : KEY_BRDA ':' NUMBER ',' NUMBER ',' NUMBER ',' NUMBER
                        {EP->onBranchCoverage($3, $5, $7, $9, @$.begin.line);}
                    | KEY_BRDA ':' NUMBER ',' NUMBER ',' NUMBER ',' '-'
                        {EP->onBranchNotCovered($3, $5, $7, @$.begin.line);}

directive_brf       : KEY_BRF ':' NUMBER
                        {EP->onBranchesTotal($3, @$.begin.line);}

directive_brh        : KEY_BRH ':' NUMBER
                        {EP->onBranchesTotalHit($3, @$.begin.line);}

%%

/* Error method of the parser */
#include <stdexcept>


#include "trace_scanner.hh"
void yy::parser::error(const yy::location& yyloc,
    const std::string& what)
{
    trace_parse_error(yyloc, what);
}

/* Implementation of scanner routine*/
int yylex(yy::parser::semantic_type* yylval,
    yy::location* yylloc, TraceScanner* scanner)
{
    return scanner->yylex(yylval, yylloc);
}


/* Implementation of TraceParser methods */

class TraceParser::Impl
{
public:
    TraceScanner scanner;
    /* Set only when parsing */
    TraceEventProcessor* eventProcessor;
    /* 
     * Base parser class use 'scanner' in its constructor, so this field
     * should come after it.
     */
    yy::parser parserBase;
    
    Impl(): parserBase(&scanner, this) {}
};

TraceParser::TraceParser(): impl(new Impl()) {}

TraceParser::~TraceParser()
{
    delete impl;
}


void TraceParser::parse(std::istream& is,
    TraceEventProcessor& eventProcessor, const char* filename)
{
    impl->scanner.setStream(is);
    impl->eventProcessor = &eventProcessor;
    impl->parserBase.parse();
}

TraceEventProcessor* getEventProcessor(TraceParser::Impl* parserImpl)
{
    return parserImpl->eventProcessor;
}
