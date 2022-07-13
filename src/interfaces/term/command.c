#include "command.h"
#include "debug.h"
#include <stdio.h>

enum key_state {
	Start,
	Esc,
	Csi,
	Kc,
};

struct key {
	uint8_t c;
	uint8_t keycode:1;
	uint8_t shift:1;
	uint8_t alt:1;
	uint8_t ctrl:1;
	uint8_t meta:1;
	uint8_t aux;
};

static void set_mods(struct key *k, uint8_t m) {
	--m;
	k->shift = m & 1;
	k->alt = (m >> 1) & 1;
	k->ctrl = (m >> 2) & 1;
	k->meta = (m >> 3) & 1;
	return;
}

static int parse_key(char in, enum key_state *ks, struct key *k) {
	switch(*ks) {
		case Start:
			if (in == '\x1b') {
				*ks = Esc;
				return 0;
			}
			k->c = in;
			k->keycode = 0;
			k->shift = 0;
			k->alt = 0;
			k->ctrl = 0;
			k->meta = 0;
			return 1;
		case Esc:
			if (in == '\x1b') {
				*ks = Start;
				k->c = 0x1b;
				k->keycode = 0;
				k->shift = 0;
				k->alt = 0;
				k->ctrl = 0;
				k->meta = 0;
				return 1;
			}
			if (in == '[') {
				*ks = Csi;
				k->c = 0;
				k->keycode = 1;
				k->shift = 0;
				k->alt = 0;
				k->ctrl = 0;
				k->meta = 0;
				return 0;
			}
			return -1;
		case Csi:
			if ((in >= '0') && (in <= '9')) {
				k->c *= 10;
				k->c += (in - '0');
				return 0;
			}
			if (in == ';') {
				*ks = Kc;
				if (k->c == 0) {
					k->c = 1;
				}
				k->aux = 0;
				return 0;
			}
			if (in == '~') {
				*ks = Start;
				if (k->c == 0) {
					k->c = 1;
				}
				return 1;
			}
			*ks = Start;
			if (k->c == 0) {
				k->c = 1;
			}
			set_mods(k, k->c);
			k->c = (uint8_t)in;
			return 1;
		case Kc:
			if ((in >= '0') && (in <= '9')) {
				k->aux *= 10;
				k->aux += (in - '0');
				return 0;
			}
			if (in == '~') {
				*ks = Start;
				if (k->aux == 0) {
					k->aux = 1;
				}
				set_mods(k, k->aux);
				return 1;
			}
			return -1;
		default:
			return -1;
	}
}

static _Bool nomod(struct key *k) {
	return (k->shift == 0) && (k->alt == 0) && (k->ctrl == 0) && (k->meta == 0);
}

static _Bool just_shift(struct key *k) {
	return (k->shift == 1) && (k->alt == 0) && (k->ctrl == 0) && (k->meta == 0);
}

static _Bool is_up(struct key *k) {
	return k->keycode && (k->c == (uint8_t)'A') && nomod(k);
}

static _Bool is_down(struct key *k) {
	return k->keycode && (k->c == (uint8_t)'B') && nomod(k);
}

static _Bool is_pup(struct key *k) {
	return k->keycode && (k->c == 5) && nomod(k);
}

static _Bool is_pdown(struct key *k) {
	return k->keycode && (k->c == 6) && nomod(k);
}

static _Bool is_fpup(struct key *k) {
	return k->keycode && (k->c == 5) && just_shift(k);
}

static _Bool is_fpdown(struct key *k) {
	return k->keycode && (k->c == 6) && just_shift(k);
}

static _Bool is_char(struct key *k, char c) {
	return (!k->keycode) && (k->c == (uint8_t)c);
}

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
		if (is_up(&k)) {
			--*focused_entry;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_down(&k)) {
			++*focused_entry;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_pup(&k)) {
			*focused_entry -= 20;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_pdown(&k)) {
			*focused_entry += 20;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_fpup(&k)) {
			*focused_entry = 0;
			*lv_needs_refresh = 1;
			continue;
		}
		if (is_fpdown(&k)) {
			*focused_entry = SIZE_MAX;
			*lv_needs_refresh = 1;
			continue;
		}
debug("unknown key [%u]: keycode %u shift %u alt %u ctrl %u meta %u\n",
		k.c, k.keycode, k.shift, k.alt, k.ctrl, k.meta);
	}
	return;
}

