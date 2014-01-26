typedef struct {
  char* name;
  void* func;
} pair;

#define NULL 0

int strcmp(const char *s1, const char *s2) {
  int ret = 0;
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) ++s1, ++s2;
  if (ret < 0)
    ret = -1;
  else if (ret > 0)
    ret = 1;

  return ret;
}

void* getfunc(const void* object, const char* name) {
  pair* funcmap;
  for(funcmap = *(pair**)object; funcmap; ++funcmap) {
    int diff = strcmp(funcmap->name,name);
    if (diff==0)
      return funcmap->func;
    if (diff==1)
      return NULL;
  }
  return NULL;
}
