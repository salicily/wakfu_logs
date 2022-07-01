#ifndef ENTRY_PARSER
#define ENTRY_PARSER

#include "entry.h"
#include <stddef.h>
#include "characters.h"

int entry_parser(struct characters *chars, const char *text, size_t text_size, struct entry *entry);

#endif /* ENTRY_PARSER */

