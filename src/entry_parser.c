#include "entry_parser.h"
#include <string.h>
#include <stdint.h>

struct channel {
	const char *prefix;
	const char *infix;
};

static const struct channel channels[] = {
	[chan_commerce   ] = { .prefix = "[Commerce] "         , .infix = " : "  , },
	[chan_guilde     ] = { .prefix = "[Guilde] "           , .infix = " : "  , },
	[chan_proximite  ] = { .prefix = "[Proximité] "        , .infix = " : "  , },
	[chan_recrutement] = { .prefix = "[Recrutement] "      , .infix = " : "  , },
	[chan_prive_from ] = { .prefix = "[Privé] FROM \""     , .infix = "\" : ", },
	[chan_prive_to   ] = { .prefix = "[Privé] TO \""       , .infix = "\" : ", },
	[chan_group      ] = { .prefix = "[Groupe] "           , .infix = " : "  , },
	[chan_in         ] = { .prefix = "[Information (jeu)] ", .infix = " ("   , },
	[chan_out        ] = { .prefix = "[Information (jeu)] ", .infix = " ("   , },
};

static int skip_prefix(const char *prefix, const char *text, size_t text_size, size_t *offset) {
	size_t offback = *offset;
	while (*offset < text_size) {
		if (*prefix == '\0') {
			return 0;
		}
		if (text[*offset] != *prefix) {
			*offset = offback;
			return -1;
		}
		++prefix;
		++*offset;
	}
	if (*prefix == '\0') {
		return 0;
	}
	*offset = offback;
	return -1;
}

static int find_infix(const char *infix, const char *text, size_t text_size, size_t *offset, struct string *s) {
	size_t offback = *offset;
	s->offset = *offset;
	s->size = 0;
	while (*offset < text_size) {
		int r = skip_prefix(infix, text, text_size, offset);
		if (r == 0) {
			return 0;
		}
		++*offset;
		++s->size;
	}
	*offset = offback;
	return -1;
}

static int parse_channel(struct characters *chars, enum chan_id cid, struct entry *e, const char *text, size_t text_size) {
	size_t offset = 0;
	int r = skip_prefix(channels[cid].prefix, text, text_size, &offset);
	if (r != 0) {
		return -1;
	}
	r = find_infix(channels[cid].infix, text, text_size, &offset, &e->text);
	if (r != 0) {
		return -1;
	}
	if (cid == chan_in) {
		size_t suboff = offset;
		struct string sub;
		r = find_infix(") a rejoint notre monde", text, text_size, &suboff, &sub);
		if (r != 0) {
			return -1;
		}
		text_size = sub.offset + sub.size;
	}
	if (cid == chan_out) {
		size_t suboff = offset;
		struct string sub;
		r = find_infix(") vient de quitter notre monde", text, text_size, &suboff, &sub);
		if (r != 0) {
			return -1;
		}
		text_size = sub.offset + sub.size;
	}
	size_t hash;
	char name[64];
	size_t sz = (e->text.size >= sizeof(name)) ? sizeof(name) - 1 : e->text.size;
	memcpy(name, text + e->text.offset, sz);
	name[sz] = '\0';
	r = characters_hash(chars, name, &hash);
	if (r != 0) {
		return -1;
	}
	e->src = hash;
	e->chan = cid;
	e->text.offset = offset;
	e->text.size = text_size - offset;
	return 0;
}

int entry_parser(struct characters *chars, const char *text, size_t text_size, struct entry *entry) {
	if ((entry == NULL) || (text == NULL) || (chars == NULL)) {
		return -1;
	}
	/* Parse time "xx:xx:xx,xxx - " */
	if (text_size < 15) {
		return -1;
	}
	if ((text[2] != ':') || (text[5] != ':') || (text[8] !=',') || (text[12] != ' ') || (text[13] != '-') || (text[14] != ' ')) {
		return -1;
	}
	uint32_t hour = (text[0] - '0') * 10 + (text[1] - '0');
	uint32_t min = (text[3] - '0') * 10 + (text[4] - '0');
	uint32_t sec = (text[6] - '0') * 10 + (text[7] - '0');
	uint32_t milli = (text[9] - '0') * 100 + (text[10] - '0') * 10 + (text[11] - '0');
	entry->time = ((hour * 60 + min) * 60 + sec) * 1000 + milli;
	text += 15;
	text_size -= 15;
	enum chan_id cid = 0;
	while (cid < chan_invalid) {
		int r = parse_channel(chars, cid, entry, text, text_size);
		if (r == 0) {
			entry->text.offset += 15;
			return 0;
		}
		++cid;
	}
	return -1;
}

