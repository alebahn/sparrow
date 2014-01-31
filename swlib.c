#include <stdio.h>
#include <stdlib.h>

#include "swruntime.h"

/****class console****/
void console_print(void* this, void* string);

const pair console_vtab[1] = {(pair){"print",console_print}};

void* __console = &console_vtab;
void* console = &__console;

void console_print(void* this, void* string) {
  void *(*toString)(void*) = getfunc(string, "toString");
  void* convstring = toString(string);
  char *(*strPrim)(void*) = getfunc(convstring, "stringPrimitive");
  char *str = strPrim(convstring);
  printf("%s", str);
}

/****class string****/
void* string_toString(void* this);
char* string_stringPrimitive(void* this);
void* string_stringLiteral(char* str);

typedef struct {
  pair** vtab;
  char* str;
} s_string;

const pair string_vtab[2] = {(pair){"stringPrimitive",string_stringPrimitive},(pair){"toString",string_toString}};

s_string __string = (s_string){(pair**)&string_vtab, ""};
void* string = &__string;

void* string_toString(void* this) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = ((s_string*)this)->str;
  return result;
}
char* string_stringPrimitive(void* this) {
  return ((s_string*)this)->str;
}
void* string_stringLiteral(char* str) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = str;
  return result;
}
