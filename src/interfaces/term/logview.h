#ifndef LOGVIEW
#define LOGVIEW

#include "config.h"
#include "../../log_engine.h"
#include <stddef.h>

int log_view(struct config *cfg, struct logs *lgs, size_t scol, size_t cols, size_t sline, size_t lines, size_t entry);

#endif /* LOGVIEW */

