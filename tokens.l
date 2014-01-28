%{
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "node.h"
#include "codegen.h"
#include "parser.tab.hpp"

#include <llvm/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>

  //extern "C" int yywrap() { }
extern std::map<std::string,void*> symbolTable;

yyFlexLexer yyLex;
extern int yyparse();

extern program* root;
extern llvm::Module* module;

%}

%option noyywrap
%option c++

%%
[0-9]+          { yylval.ival = atoi(yytext); return NUMBER; }
[0-9]+\.[0-9]+  { yylval.fval = atof(yytext); return FNUMBER; }
class           { return CLASS; }
def             { return DEF; }
import          { return IMPORT; }
[a-zA-Z_]+      { yylval.sval = new std::string(yytext, yyleng); return NAME; }
\"(\\.|[^"])*\" { yylval.sval = new std::string(yytext+1, yyleng-2); return STRING; }
\n              { return NEWLINE; }
[ \t]           ; /* ignore white space except newline */
.               return yytext[0];
%%

void print_usage(char* name) {
  std::cerr << "usage: " << name << " input_file" << std::endl;
}

int yylex() {
  yyLex.yylex();
}

int main(int argc, char **argv) {
  //symbolTable["print"] = (void*)&print;
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
  for (int i=0; i<classes->getSize(); ++i) {
    class_def* cls = (class_def*)classes->getChild(i);
    cls->genCode();

    root->genCode();
    module->dump();

    std::string error;
    llvm::raw_fd_ostream os((module->getModuleIdentifier()+".bc").c_str(), error, 0);
    if (error != "") {
      std::cerr << "could not open output: " << error << std::endl;
      return 3;
    }
    llvm::WriteBitcodeToFile(module, os);
    os.close();
  }

  return 0;
}
