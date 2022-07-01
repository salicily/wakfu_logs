#ifndef ENTRY
#define ENTRY

enum chan_id {
	chan_commerce,
	chan_guilde,
	chan_proximite,
	chan_recrutement,
	chan_prive_from,
	chan_prive_to,
	chan_group,
	chan_invalid,
};

struct string {
	unsigned int size:8;
	unsigned int offset:24;
};

struct entry {
	unsigned int time:18;
	unsigned int chan:4;
	unsigned int src:10;
	struct string text;
};

#endif /* ENTRY */

