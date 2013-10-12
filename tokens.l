%{
#include <string>
#include "parser.tab.hpp"
  extern "C" int yywrap() { }
%}

%%
[0-9]+          { yylval.ival = atoi(yytext); return NUMBER; }
[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return FNUMBER; }
@[a-zA-Z]+      { yylval.sval = new std::string(yytext, yyleng); return NAME; }
[ \t]           ; /* ignore white space except newline */
.               return yytext[0];
%%
