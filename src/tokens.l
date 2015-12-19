%{
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <system_error>
#include "node.h"
#include "codegen.h"
#include "parser.tab.hpp"

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>

yyFlexLexer yyLex;
extern int yyparse();

extern program* root;
extern llvm::Module* module;

int yylineno = 1;

#define YY_USER_ACTION  yylloc.first_line = yylloc.last_line; \
                        yylloc.first_column = yylloc.last_column;  \
                        yylloc.last_column = yylloc.first_column+yyleng;
%}

%option noyywrap
%option c++
%option yylineno

%%
-?[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return FNUMBER; }
-?[0-9]+          { yylval.ival = atoi(yytext); return NUMBER; }
class             { return CLASS; }
def               { return DEF; }
if                { return IF; }
else              { return ELSE; }
elseif            { return ELSEIF; }
can               { return CAN; }
elsecan           { return ELSECAN; }
while             { return WHILE; }
import            { return IMPORT; }
new               { return NEW; }
this              { return THIS; }
null              { return NULL_T; }
true              { return TRUE; }
false             { return FALSE; }
\<=               { return LE; }
>=                { return GE; }
!=                { return NE; }
==                { return EQ; }
[a-zA-Z_]+        { yylval.sval = new std::string(yytext, yyleng); return NAME; }
\"(\\.|[^"])*\"   { yylval.sval = new std::string(yytext+1, yyleng-2); return STRING; }
\n                { yylloc.last_line = yylloc.first_line+1;
                    yylloc.last_column = 1;
                    return NEWLINE; }
[ \t]             ; /* ignore white space except newline */
\/\*(\*[^\/]|[^\*])*\*\/ ; /* ignore comments */
\/\/[^\n]*        ; /* ignore comments */
.                 return yytext[0];
%%

void print_usage(char* name) {
  std::cerr << "usage: " << name << " input_file" << std::endl;
}

int yylex() {
  return yyLex.yylex();
}

int main(int argc, char **argv) {
  std::istream* yyin;

  if(argc < 2) {
    print_usage(argv[0]);
    return 1;
  }
  yyin = new std::ifstream(argv[1]);
  if(!*yyin) {
    std::cerr << argv << " is not a valid input filename." << std::endl;
    return 2;
  }
  yyLex = yyFlexLexer(yyin);
  yyparse();

  root->prepass();
  dump_types();
  list* classes = root->getClasses();
  for (unsigned i=0; i<classes->getSize(); ++i) {
    class_def* cls = (class_def*)classes->getChild(i);
    cls->genCode();

    //module->dump();

    std::error_code error;
    llvm::raw_fd_ostream os(llvm::StringRef(module->getModuleIdentifier()+".bc"), error, llvm::sys::fs::OpenFlags::F_None);
    if (error) {
      std::cerr << "could not open output: " << error << std::endl;
      return 3;
    }
    llvm::WriteBitcodeToFile(module, os);
    os.close();
  }

  return 0;
}
