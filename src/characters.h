#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <stddef.h>

struct characters;

/* Returns NULL if not enough memory for a characters table of names entries */
struct characters *characters_create(size_t names);

/* Release resources allocated for the characters table */
void characters_destroy(struct characters *chars);

/* Return number of managed names */
size_t characters_max_names(struct characters *chars);

/* Hash a name, if not previously hashed, bind it to the default configuration */
int characters_hash(struct characters *chars, const char *name, size_t *hash);

/* Unhash a name (ie. make it unbound) */
int characters_unhash(struct characters *chars, size_t hash);

/* Get the name bound to a hash, take care of having name_size (size of allocated buffer for name) big enough */
int characters_get_name(struct characters *chars, size_t hash, char *name, size_t name_size);

/* Complete the start of a string to get an already hashed name, *level is set to length of string */
int characters_complete(struct characters *chars, const char *name, size_t *level, size_t *hash);

/* Return next completion, level is the length of string from which to search */
int characters_next_complete(struct characters *chars, size_t level, size_t *hash);

#endif /* CHARACTERS_H */

