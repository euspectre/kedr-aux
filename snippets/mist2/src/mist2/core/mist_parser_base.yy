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

%token IF_KEYWORD ELSE_KEYWORD ENDIF_KEYWORD

%token JOIN_KEYWORD CONCAT_KEYWORD

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
    MistASTJoin* astJoin;
    MistASTWith* astWith;
    MistASTTemplateSequence* astSequence;
}
%destructor {delete $$;} <astRef, astText, astIf, astJoin, astWith, astSequence>

%start whole_template

%type <astSequence> template_sequence

%type<astTemplate> template_internal

%type <templateName> template_name

%type <astRef> template_ref
%type <astText> text
%type <astIf> if_statement
%type <astJoin> join_statement
%type <astWith> with_scope

/* Condition part of 'if' directive */
%type <astTemplate> if_condition
/* Join expression */
%type <astJoin> join_expression

/* Template reference expression */
%type <astRef> ref_expression


/* 'if' directive contain only condition semantic, so it has same type */
%type <astTemplate> if_directive

%type <str> join_text

/* 'with' directive contain only parameter name semantic, so it has same type */
%type <templateName> with_directive


%%
whole_template          : template_sequence
                        {getAST(outerParser)->astSequence = $1;}

template_sequence       : /* empty */
                        {$$ = new MistASTTemplateSequence();}
                        | template_sequence template_internal
                        {$$ = $1; $$->addTemplate(ptr($2));}

template_internal       : template_ref
                        {$$ = $1;}
                        | text
                        {$$ = $1;}
                        | if_statement
                        {$$ = $1;}
                        | join_statement
                        {$$ = $1;}
                        | with_scope
                        {$$ = $1;}


template_ref            : BEGIN_MARKER ref_expression END_MARKER
                        {$$ = $2;}

ref_expression          : template_name
                        {$$ = new MistASTTemplateRef(ptr($1));}

text                    : TEXT
                        {$$ = new MistASTText(ptr($1));}

if_statement            : if_directive template_sequence endif_directive
                        {$$ = new MistASTIf(ptr($1), ptr($2));}
                        | if_directive template_sequence else_directive template_sequence endif_directive
                        {$$ = new MistASTIf(ptr($1), ptr($2), ptr($4));}

if_directive            : BEGIN_MARKER IF_KEYWORD if_condition END_MARKER
                            {$$ = $3;}

else_directive          : BEGIN_MARKER ELSE_KEYWORD END_MARKER
endif_directive         : BEGIN_MARKER ENDIF_KEYWORD END_MARKER

if_condition            : ref_expression
                        {$$ = $1;}
                        | join_expression
                        {$$ = $1;}

join_expression         : ref_expression ':' JOIN_KEYWORD join_text
                        { $$ = new MistASTJoin(ptr($1), ptr($4));}

join_text               : TEXT {$$ = $1;}
                        | /* empty */ {$$ = NULL;}

join_statement          : BEGIN_MARKER join_expression END_MARKER
                        {$$ = $2;}

                        
with_scope              : with_directive template_sequence endwith_directive
                        {$$ = new MistASTWith(ptr($1), ptr($2));}

with_directive          : BEGIN_MARKER WITH_KEYWORD template_name END_MARKER
                            {$$ = $3;}
endwith_directive       : BEGIN_MARKER ENDWITH_KEYWORD END_MARKER

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
