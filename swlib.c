#include <stdio.h>

typedef struct {
  char* name;
  void* func;
} pair;

/****class console****/
void console_print(void* this, char* string);

const pair console_vtab[1] = {(pair){"print",console_print}};

void* __console = &console_vtab;
void* console = &__console;

void console_print(void* this, char* string) {
  printf("%s", string);
}

/****class string****/
void* string_toString(void* this);
char* string_stringPrimitive(void* this);
void* string_stringLiteral(char* str);

struct string {
  pair** vtab;
  char* str;
};;

const pair string_vtab[2] = {(pair){"toString",string_toString},(pair){"stringPrim",string_toString}};

void* string_toString(void* this) {
}
char* string_stringPrimitive(void* this) {
}
void* string_stringLiteral(char* str) {
}
