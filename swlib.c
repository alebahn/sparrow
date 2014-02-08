#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "swruntime.h"

/****class console****/
void* console_print(void* this, void* string);
void* console_println(void* this, void* string);

const pair console_vtab[2] = {(pair){"print",console_print}, (pair){"println",console_println}};

void* console = &console_vtab;

void* console_print(void* this, void* string) {
  void *(*toString)(void*) = getfunc(string, "toString");
  void* convstring = toString(string);
  char *(*strPrim)(void*) = getfunc(convstring, "stringPrimitive");
  char *str = strPrim(convstring);
  printf("%s", str);
  return this;
}

void* console_println(void* this, void* string) {
  void *(*toString)(void*) = getfunc(string, "toString");
  void* convstring = toString(string);
  char *(*strPrim)(void*) = getfunc(convstring, "stringPrimitive");
  char *str = strPrim(convstring);
  printf("%s\n", str);
  return this;
}

/****class string****/
void* string_toString(void* this);
char* string_stringPrimitive(void* this);

typedef struct {
  pair** vtab;
  char* str;
} s_string;

const pair string_vtab[2] = {(pair){"stringPrimitive",string_stringPrimitive},(pair){"toString",string_toString}};

s_string string = (s_string){(pair**)&string_vtab, ""};

void* string_toString(void* this) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = ((s_string*)this)->str;
  return result;
}
char* string_stringPrimitive(void* this) {
  return ((s_string*)this)->str;
}

/****class int****/
void* int_toString(void* this);
int int_intPrimitive(void* this);

typedef struct {
  pair** vtab;
  int val;
} s_int;

const pair int_vtab[2] = {(pair){"intPrimitive",int_intPrimitive},(pair){"toString",int_toString}};

s_int _int = (s_int){(pair**)&int_vtab, 0};

void* int_toString(void* this) {
  char *str;
  int val = ((s_int*)this)->val;
  unsigned str_len;
  if (val == 0) {
    str = "0";
  } else {
    int testVal = val;
    if (val < 0)
      testVal = -val;
    str_len = log10(testVal)+1;
    if (val < 0)
      ++str_len;
    str = malloc(sizeof(char)*(str_len+1));
    str[str_len]=0;
    int i;
    for (i=str_len-1; i>=0; --i) {
      str[i]='0'+testVal%10;
      testVal = testVal/10;
    }
    if (val < 0)
      str[0]='-';
  }
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = str;
  return result;
}
int int_intPrimitive(void* this) {
  return ((s_int*)this)->val;
}
