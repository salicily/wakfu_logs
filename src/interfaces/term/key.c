#include "key.h"
#include <stdlib.h>

_Bool is_keycode(struct key *k) {
	return k->m & 1;
}

_Bool has_shift(struct key *k) {
	return k->m & 2;
}

_Bool has_alt(struct key *k) {
	return k->m & 4;
}

_Bool has_ctrl(struct key *k) {
	return k->m & 8;
}

_Bool has_meta(struct key *k) {
	return k->m & 16;
}

static uint8_t conv_mods(uint8_t m) {
	return (m == 0) ? 0 : ((m-1)<<1)|1;
}

int parse_key(char in, enum key_state *ks, struct key *k) {
	if (ks == NULL) {
		return -1;
	}
	if (k == NULL) {
		return -1;
	}
	switch(*ks) {
		case Start:
			if (in == '\x1b') {
				*ks = Esc;
				return 0;
			}
			k->c = in;
			k->m = 0;
			return 1;
		case Esc:
			if (in == '\x1b') {
				*ks = Start;
				k->c = 0x1b;
				k->m = 0;
				return 1;
			}
			if (in == '[') {
				*ks = Csi;
				k->c = 0;
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
				k->m = 1;
				return 1;
			}
			*ks = Start;
			if (k->c == 0) {
				k->c = 1;
			}
			k->m = conv_mods(k->c);
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
				k->m = conv_mods(k->aux);
				return 1;
			}
			return -1;
		default:
			return -1;
	}
}

_Bool is_key(const struct key *ref, const struct key *cmp) {
	if (ref == NULL) {
		return cmp == NULL;
	}
	if (cmp == NULL) {
		return 0;
	}
	if (ref->c != cmp->c) {
		return 0;
	}
	return ref->m == cmp->m;
}

const struct key key_up = {
	.c = (uint8_t)'A',
	.m = 1,
};

const struct key key_down = {
	.c = (uint8_t)'B',
	.m = 1,
};

const struct key key_pup = {
	.c = 5,
	.m = 1,
};

const struct key key_pdown = {
	.c = 6,
	.m = 1,
};

const struct key key_fpup = {
	.c = 5,
	.m = 3,
};

const struct key key_fpdown = {
	.c = 6,
	.m = 3,
};

_Bool is_char(struct key *k, char c) {
	struct key key_char;
	key_char.c = c;
	key_char.m = 0;
	return is_key(&key_char, k);
}

