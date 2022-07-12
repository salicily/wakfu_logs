#include "command.h"
#include <stdio.h>

#define mark printf("[c:%d]\r\n", __LINE__);

void refresh_inputs(struct config *cfg, struct logs *lgs, size_t scol, size_t cols, size_t sline, size_t lines, const char *inputs, size_t inputs_size, _Bool resized, size_t *focused_entry, _Bool *quit, _Bool *lv_needs_refresh) {
	(void)lgs;
	*lv_needs_refresh = resized;
	static char keycodes[128];
	if (sizeof(keycodes) <= (2 * inputs_size)) {
		inputs_size = (sizeof(keycodes) / 2);
	}
	for (size_t i = 0; i < inputs_size; ++i) {
		printf("%02x", inputs[i]);
		if (inputs[i] == 'a') {
			++*focused_entry;
			*lv_needs_refresh = 1;
		}
		if (inputs[i] == 'u') {
			--*focused_entry;
			*lv_needs_refresh = 1;
		}
		if (inputs[i] == 'q') {
			*quit = 1;
		}
	}
	return;
}

