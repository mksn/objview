#include "objview.h"
#include "cache.h"

#include <search.h>
#include <errno.h>

#define MAXFILES 4096

void cache_init(void)
{
  hcreate(MAXFILES);
}

void *cache_find(const char *key)
{
  ENTRY e = { (char*)key, NULL };
  ENTRY *r = hsearch(e, FIND);
  if (r) {
    printf("cache found: %s\n", key);
    return r->data;
  }
  return NULL;
}

void cache_insert(const char *key, void *data)
{
  ENTRY e = { strdup(key), data };
  ENTRY *r = hsearch(e, ENTER);
  if (!r) {
    fprintf(stderr, "cannot insert file into cache: %s\n", strerror(errno));
    abort();
  } else {
    printf("cache insert: %s\n", key);
  }
}
