/* Generate C++ parser */
%skeleton "lalr1.cc"
%require "2.4"
%defines

%locations

%code requires{
#include "mist_ast.hh"
#include <memory>
class MistParser;
class MistScanner;
}

%code {
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
}

/* 
 * Outer 'parser' object contain all information needed for parsing:
 * filename, AST builded, scanner.
 */
%parse-param {MistParser* outerParser}

/*
 * Reentrant LEX scanner accept additional parameter.
 */
%lex-param  {MistParser* outerParser}

%initial-action {
    @$.initialize(&getFilename(outerParser));
}

/********************* Tokens ******************************************/

%token BEGIN_MARKER END_MARKER

%token IF_KEYWORD ENDIF_KEYWORD ELSE_KEYWORD ELSEIF_KEYWORD

%token WITH_KEYWORD ENDWITH_KEYWORD


%union {
    std::string* str;
}
%destructor { delete $$;} <str>

%token <str> TEXT

%token <str> ID

%token <str> UNKNOWN

/************************ Grouping ************************************/

%union {
    MistASTTemplateName* templateName;
}
%destructor {delete $$;} <templateName>

%union {
    MistASTTemplate* astTemplate;
}
%destructor {delete $$;} <astTemplate>

%union {
    MistASTTemplateRef* astRef;
    MistASTText* astText;
    MistASTIf* astIf;
    MistASTWith* astWith;
    MistASTTemplateSequence* astSequence;
    MistASTFunction* astFunction;
}
%destructor {delete $$;} <astRef, astText, astIf, astWith, astSequence, astFunction>

%start whole_template

%type <astSequence> template_sequence

%type<astTemplate> template_internal

%type <templateName> template_name

%type <astTemplate> template_ref
%type <astTemplate> text
%type <astTemplate> if_statement
%type <astTemplate> with_scope

/* Function without argument. */
%type <astFunction> func_apply

/* 
 * Common part of any if statement:
 *  'if' + condition + sequence [+ 'elseif' + condition + sequence]* BEGIN_MARKER
 */
%type <astIf> if_statement_common

/*
 * If statement up to 'endif' keyword [+ func_apply]*
 */
%type <astTemplate> if_statement_terminated


/* Template argument: name [+ func_apply]* */
%type <astTemplate> template_arg



/*
 * With scope up to 'endwith' keyword [+ func_apply]*
 */
%type <astTemplate> with_scope_terminated


%%
whole_template          : template_sequence
                        {getAST(outerParser)->astSequence = $1;}

template_sequence       : /* empty */
                        {$$ = new MistASTTemplateSequence();}
                        | template_sequence template_internal
                        {$$ = $1; $$->addTemplate(ptr($2));}

template_internal       : template_ref
                        | text
                        | if_statement
                        | with_scope


template_ref            : BEGIN_MARKER template_arg END_MARKER
                        {$$ = $2;}

template_arg            : template_name
                        {$$ = new MistASTTemplateRef(ptr($1));}
                        | template_arg func_apply
                        {$2->templateInternal = ptr($1); $$ = $2;}

func_apply              : ':' ID
                        {$$ = new MistASTFunction(ptr($2)); }
                        | ':' ID TEXT
                        {$$ = new MistASTFunction(ptr($2), ptr($3)); }

text                    : TEXT
                        {$$ = new MistASTText(ptr($1));}

if_statement            : if_statement_terminated END_MARKER

if_statement_terminated : if_statement_common ENDIF_KEYWORD
                        {$$ = $1;}
                        | if_statement_common ELSE_KEYWORD END_MARKER template_sequence BEGIN_MARKER ENDIF_KEYWORD
                        {$1->elsePart = ptr($4); $$ = $1;}
                        | if_statement_terminated func_apply
                        {$2->templateInternal = ptr($1); $$ = $2;}

if_statement_common     : BEGIN_MARKER IF_KEYWORD template_arg END_MARKER template_sequence BEGIN_MARKER
                        {$$ = new MistASTIf(ptr($3), ptr($5));}
                        | if_statement_common ELSEIF_KEYWORD template_arg END_MARKER template_sequence BEGIN_MARKER
                        {$$ = $1; $$->addConditionPart(ptr($3), ptr($5)); }
                        
with_scope              : with_scope_terminated END_MARKER
                        {$$ = $1;}

with_scope_terminated   : BEGIN_MARKER WITH_KEYWORD template_name END_MARKER template_sequence BEGIN_MARKER ENDWITH_KEYWORD
                        {$$ = new MistASTWith(ptr($3), ptr($5));}
                        | with_scope_terminated func_apply
                        {$2->templateInternal = ptr($1); $$ = $2;}
                        
template_name           : ID
                        {$$ = new MistASTTemplateName(ptr($1));}
                        | '.' ID
                        {$$ = new MistASTTemplateName(ptr($2), true);}
                        | template_name '.' ID
                        {$$ = $1; $$->addComponent(ptr($3));}
%%

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
