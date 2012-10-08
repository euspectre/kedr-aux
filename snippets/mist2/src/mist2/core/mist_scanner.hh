#ifndef MIST_SCANNER_H_INCLUDED
#define MIST_SCANNER_H_INCLUDED

#include <iostream>
#include "mist_parser_base.tab.hh"
#include "location.hh"

class MistScanner
{
public:
	MistScanner();
	~MistScanner();
	
    void setStream(std::istream& s);
    
	int yylex(yy::parser::semantic_type* yylval, yy::location* yylloc);
	
private:
	/* Not copiable and assignable*/
	MistScanner(const MistScanner& scanner);
	MistScanner& operator =(const MistScanner& scanner);

	/* Real type is yyscan_t */
	void* _scanner;
};

#endif /* MIST_SCANNER_H_INCLUDED */