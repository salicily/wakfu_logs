#ifndef COMMAND
#define COMMAND

#include "config.h"
#include "../../log_engine.h"
#include <stddef.h>

void refresh_inputs(struct config *cfg, struct logs *lgs, size_t scol, size_t cols, size_t sline, size_t lines, const char *inputs, size_t inputs_size, _Bool resized, size_t *focused_entry, _Bool *quit, _Bool *lv_needs_refresh);

#endif /* COMMAND */

