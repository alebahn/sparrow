#include <stdio.h>

typedef struct {
  char* name;
  void* func;
} pair;

void console_print(void* this, char* string);

const pair console_vtab[1] = {(pair){"print",console_print}};

void* console = &console_vtab;

void console_print(void* this, char* string) {
  printf("%s", string);
}
