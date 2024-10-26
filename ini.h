#ifndef INI_H
#define INI_H

#include <stdbool.h>

typedef struct ini_file ini_file;
typedef struct ini_value ini_value;

struct ini_file {
  char *data;
  char *end;
};

struct ini_value {
  bool ok;
  union {
    int Integer;
    const char *String;
    bool Boolean;
    double Decimal;
  } value;
};


extern ini_file* ini_load(const char *filename);
extern void ini_free(ini_file *ini);

extern ini_value ini_getint(ini_file *ini, const char *tablename, const char *key);
extern ini_value ini_getbool(ini_file *ini, const char *tablename, const char *key);
extern ini_value ini_getstring(ini_file *ini, const char *tablename, const char *key);
extern ini_value ini_getdecimal(ini_file *ini, const char *tablename, const char *key);

#endif
