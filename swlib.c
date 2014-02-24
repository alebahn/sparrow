#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "swruntime.h"

void* console_print(void* this, void* string);
void* console_println(void* this, void* string);
void* console_readln(void* this);

/*alphabetical*/
const pair console_vtab[] = {
  (pair){"print",console_print},
  (pair){"println",console_println},
  (pair){"readln",console_readln}
};

typedef struct {
  pair** vtab;
  char* str;
} s_string;

void* string_toString(void* this);
char* string_stringPrimitive(void* this);
void* string_lessThan_string(void* this, void* other);
void* string_lessEqual_string(void* this, void* other);
void* string_greaterThan_string(void* this, void* other);
void* string_greaterEqual_string(void* this, void* other);
void* string_notEqual_string(void* this, void* other);
void* string_equalTo_string(void* this, void* other);

/*alphabetical*/
const pair string_vtab[] = {
  (pair){"equalTo_string",string_equalTo_string},
  (pair){"greaterEqual_string",string_greaterEqual_string},
  (pair){"greaterThan_string",string_greaterThan_string},
  (pair){"lessEqual_string",string_lessEqual_string},
  (pair){"lessThan_string",string_lessThan_string},
  (pair){"notEqual_string",string_notEqual_string},
  (pair){"stringPrimitive",string_stringPrimitive},
  (pair){"toString",string_toString}
};

typedef struct {
  pair** vtab;
  int val;
} s_int;

void* int_toString(void* this);
int int_intPrimitive(void* this);
void* int_lessThan_int(void* this, void* other);
void* int_lessEqual_int(void* this, void* other);
void* int_greaterThan_int(void* this, void* other);
void* int_greaterEqual_int(void* this, void* other);
void* int_notEqual_int(void* this, void* other);
void* int_equalTo_int(void* this, void* other);

/*alphabetical*/
const pair int_vtab[] = {
  (pair){"equalTo_int",int_equalTo_int},
  (pair){"greaterEqual_int",int_greaterEqual_int},
  (pair){"greaterThan_int",int_greaterThan_int},
  (pair){"intPrimitive",int_intPrimitive},
  (pair){"lessEqual_int",int_lessEqual_int},
  (pair){"lessThan_int",int_lessThan_int},
  (pair){"notEqual_int",int_notEqual_int},
  (pair){"toString",int_toString}
};

typedef struct {
  pair** vtab;
  char val;
} s_bool;

typedef unsigned char bool;
void* bool_toString(void* this);
bool bool_boolPrimitive(void* this);
void* bool_not(void* this);

const pair bool_vtab[] = {
  (pair){"boolPrimitive",bool_boolPrimitive},
  (pair){"not",bool_not},
  (pair){"toString",bool_toString}
};

/****class console****/
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

void* console_readln(void* this) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = NULL;

  size_t bufflen;
  ssize_t len = getline(&result->str, &bufflen, stdin);
  if (len==-1) {
    perror("");
    //TODO: handle error
  }
  return result;
}

/****class string****/
s_string string = (s_string){(pair**)&string_vtab, ""};

int strObjCmp(void* first, void* second) {
  char* f_val = ((s_string*)first)->str;
  char* s_val = ((s_string*)second)->str;
  return strcmp(f_val,s_val);
}

void* string_toString(void* this) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = ((s_string*)this)->str;
  return result;
}
char* string_stringPrimitive(void* this) {
  return ((s_string*)this)->str;
}
void* string_lessThan_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)==-1;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* string_lessEqual_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)!=1;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* string_greaterThan_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)==1;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* string_greaterEqual_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)!=-1;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* string_notEqual_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)!=0;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* string_equalTo_string(void* this, void* other) {
  bool r_val = strObjCmp(this, other)==0;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}

/****class int****/
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
void* int_lessThan_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val<o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* int_lessEqual_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val<=o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* int_greaterThan_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val>o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* int_greaterEqual_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val>=o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* int_notEqual_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val!=o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}
void* int_equalTo_int(void* this, void* other) {
  int val = ((s_int*)this)->val;
  int o_val = ((s_int*)other)->val;
  bool r_val = val==o_val;
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = r_val;
  return result;
}

/****class bool****/
s_bool _bool = (s_bool){(pair**)&bool_vtab, 0};

void* bool_toString(void* this) {
  s_string* result = malloc(sizeof(s_string));
  result->vtab = (pair**)&string_vtab;
  result->str = (((s_bool*)this)->val?"true":"false");
  return result;
}
bool bool_boolPrimitive(void* this) {
  return ((s_bool*)this)->val;
}
void* bool_not(void* this) {
  s_bool* result = malloc(sizeof(s_bool));
  result->vtab = (pair**)&bool_vtab;
  result->val = !((s_bool*)this)->val;
  return result;
}
