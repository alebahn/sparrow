#pragma once
typedef struct {
  char* name;
  void* func;
} pair;

void* getfunc(const void* object, const char* name);
