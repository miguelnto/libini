#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "ini.h"

static const char *ini_get(ini_file *ini, const char *section, const char *key);
static bool strcmpci(const char *a, const char *b);
static char *next(ini_file *ini, char *p);
static void trim_back(ini_file *ini, char *p);
static char *discard_line(ini_file *ini, char *p);
static char *unescape_quoted_value(ini_file *ini, char *p);
static void split_data(ini_file *ini);

/* Case insensitive string comparision. Returns true if the strings are equal.*/
bool strcmpci(const char *a, const char *b) {
  for (;;) {
    bool d = tolower(*a) - tolower(*b);
    if (d != false || !*a) {
      return d == 0 ? true : false;
    }
    a++, b++;
  }
}

/* Returns the next string in the split data. */
char *next(ini_file *ini, char *p) {
  p += strlen(p);
  while (p < ini->end && *p == '\0') {
    p++;
  }
  return p;
}

void trim_back(ini_file *ini, char *p) {
  while (p >= ini->data && (*p == ' ' || *p == '\t' || *p == '\r')) {
    *p-- = '\0';
  }
}

char *discard_line(ini_file *ini, char *p) {
  while (p < ini->end && *p != '\n') {
    *p++ = '\0';
  }
  return p;
}

/* Unescape a quoted string. */
char *unescape_quoted_value(ini_file *ini, char *p) {
  /* Use `q` as write-head and `p` as read-head, `p` is always ahead of `q`
   * as escape sequences are always larger than their resultant data */
  char *q = p;
  p++;
  while (p < ini->end && *p != '"' && *p != '\r' && *p != '\n') {
    if (*p == '\\') {
      /* Handle escaped char */
      p++;
      switch (*p) {
        default   : *q = *p;    break;
        case 'r'  : *q = '\r';  break;
        case 'n'  : *q = '\n';  break;
        case 't'  : *q = '\t';  break;
        case '\r' : case '\n' :
        case '\0' : return q;
      }
    } else {
      /* Handle normal char */
      *q = *p;
    }
    q++, p++;
  }
  return q;
}

/* Splits data in place into strings containing section-headers, keys and
* values using one or more '\0' as a delimiter. Unescapes quoted values. */
void split_data(ini_file *ini) {
  char *value_start, *line_start;
  char *p = ini->data;

  while (p < ini->end) {
    switch (*p) {
      case '\r':
      case '\n':
      case '\t':
      case ' ':
        *p = '\0';
        /* Fall through */
      case '\0':
        p++;
        break;
      case '[':
        p += strcspn(p, "]\n");
        *p = '\0';
        break;
      case ';':
        p = discard_line(ini, p);
        break;
      default:
        line_start = p;
        p += strcspn(p, "=\n");
        /* Is the line missing a '='? */
        if (*p != '=') {
          p = discard_line(ini, line_start);
          break;
        }
        trim_back(ini, p - 1);

        /* Replace '=' and whitespace after it with '\0' */
        do {
          *p++ = '\0';
        } while (*p == ' ' || *p == '\r' || *p == '\t');

        /* Is a value after '=' missing? */
        if (*p == '\n' || *p == '\0') {
          p = discard_line(ini, line_start);
          break;
        }

        if (*p == '"') {
          /* Handle quoted string value */
          value_start = p;
          p = unescape_quoted_value(ini, p);
          /* Was the string empty? */
          if (p == value_start) {
            p = discard_line(ini, line_start);
            break;
          }
          /* Discard the rest of the line after the string value */
          p = discard_line(ini, p);
        } else {
          /* Handle normal non-quoted value */
          p += strcspn(p, "\n");
          trim_back(ini, p - 1);
        }
        break;
    }
  }
}

ini_file* ini_load(const char *filename) {
  ini_file *ini = NULL;
  FILE *fp = NULL;

  ini = calloc(1,sizeof(*ini));
  if (!ini) {
    return NULL;
  }

  fp = fopen(filename, "rb");
  if (!fp) {
    free(ini);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  int sz = ftell(fp);
  rewind(fp);

  /* Load file content into memory, null terminate, and init the end variable */
  ini->data = malloc(sz + 1);
  if (!ini->data) {
    free(ini);
    fclose(fp);
    return NULL;
  }
  ini->data[sz] = '\0';
  ini->end = ini->data  + sz;
  
  int n = fread(ini->data, 1, sz, fp);
  if (n != sz) {
    ini_free(ini);
    fclose(fp);
    return NULL;
  }

  /* Prepare data */
  split_data(ini);

  /* Clean up and return */
  fclose(fp);
  return ini;
}

void ini_free(ini_file *ini) {
  free(ini->data);
  free(ini);
}

const char* ini_get(ini_file *ini, const char *section, const char *key) {
  char *current_section = "";
  char *p = ini->data;

  if (*p == '\0') {
    p = next(ini, p);
  }

  while (p < ini->end) {
    if (*p == '[') {
      /* Handle section */
      current_section = p + 1;
    } else {
      /* Handle key */
      char *val = next(ini, p);
      if (!section || strcmpci(section, current_section)) {
        if (strcmpci(p, key)) {
          return val;
        }
      }
      p = val;
    }
    p = next(ini, p);
  }
  return NULL;
}

ini_value ini_getint(ini_file *ini, const char *tablename, const char *key) {
  ini_value result;
  memset(&result, 0, sizeof(result));
  char *eptr = NULL;
  const char *s = ini_get(ini, tablename, key);
  result.ok = false;
  if (!s) {
    return result;
  }
  errno = 0;
  result.value.Integer = strtoll(s, &eptr, 10);
  result.ok = (errno || *eptr) ? false : true;
  return result;
}

ini_value ini_getbool(ini_file *ini, const char *tablename, const char *key) {
  ini_value result;
  memset(&result, 0, sizeof(result));
  const char *s = ini_get(ini, tablename, key);
  result.ok = false;
  if (!s) {
    return result;
  }

  if (strcmp(s, "true") == 0) {
    result.ok = true;
    result.value.Boolean = true;
  } else if (strcmp(s, "false") == 0) {
    result.ok = true;
    result.value.Boolean = false;
  }
  return result;
}

ini_value ini_getstring(ini_file *ini, const char *tablename, const char *key) {
  ini_value result;
  memset(&result, 0, sizeof(result));
  const char *s = ini_get(ini, tablename, key);
  result.ok = false;
  if (!s) {
    return result;
  }
  result.value.String = s;
  result.ok = true;
  return result;
}

ini_value ini_getdecimal(ini_file *ini, const char *tablename, const char *key) {
  ini_value result;
  memset(&result, 0, sizeof(result));
  const char *s = ini_get(ini, tablename, key);
  result.ok = false;
  if (!s) {
    return result;
  }
  char *eptr = NULL;
  errno = 0;
  result.value.Decimal = strtod(s, &eptr);
  result.ok = (errno || *eptr) ? false : true;
  return result;
}
