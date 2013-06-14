#ifndef CACHE_H
#define CACHE_H

void cache_init(void);
void *cache_find(const char *key);
void cache_insert(const char *key, void *val);

#endif
