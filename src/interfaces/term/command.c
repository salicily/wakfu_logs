#include "command.h"
#include "debug.h"
#include <stdio.h>
#include "key.h"

void refresh_inputs(struct config *cfg, struct logs *lgs, size_t scol, size_t cols, size_t sline, size_t lines, const char *inputs, size_t inputs_size, _Bool resized, size_t *focused_entry, _Bool *quit, _Bool *lv_needs_refresh) {
	(void)lgs;
	*lv_needs_refresh = resized;
	struct key k;
	enum key_state ks = Start;
	for (size_t i = 0; i < inputs_size; ++i) {
		int r = parse_key(inputs[i], &ks, &k);
		if (r < 0) {
			/* ??? parser is broken, just skip the inputs */
debug("Unknown character at offset %zu in %.*s\n", i, (int)inputs_size, inputs);
			return;
		}
		if (r == 0) {
			continue;
		}
		/* r == 1 */
		if (is_char(&k, 'q')) {
			*quit = 1;
			continue;
		}
		if (is_key(&k, &key_up)) {
			--*focused_entry;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_key(&k, &key_down)) {
			++*focused_entry;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_key(&k, &key_pup)) {
			*focused_entry -= 20;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_key(&k, &key_pdown)) {
			*focused_entry += 20;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_key(&k, &key_fpup)) {
			*focused_entry = 0;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_key(&k, &key_fpdown)) {
			*focused_entry = SIZE_MAX;
			*lv_needs_refresh = 1;
			continue;
		}
debug("unknown key [%u:%u]\n", k.c, k.m);
	}
	return;
}

