#ifndef KEY_H
#define KEY_H

#include <stdint.h>

enum key_state {
	Start,
	Esc,
	Csi,
	Kc,
};

struct key {
	uint8_t c;
	uint8_t m;
	uint8_t aux;
};

_Bool is_keycode(struct key *k);

_Bool has_shift(struct key *k);

_Bool has_alt(struct key *k);

_Bool has_ctrl(struct key *k);

_Bool has_meta(struct key *k);

/* Returns -1 in case of an error,
 * 0 if partially parsed,
 * 1 if totally parsed (and the key *k is then set)
 */
int parse_key(char in, enum key_state *ks, struct key *k);

_Bool is_key(const struct key *ref, const struct key *cmp);

extern const struct key key_up;
extern const struct key key_down;
extern const struct key key_pup;
extern const struct key key_pdown;
extern const struct key key_fpup;
extern const struct key key_fpdown;

_Bool is_char(struct key *k, char c);

#endif

