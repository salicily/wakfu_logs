#include "log_engine.h"
#include "entry_parser.h"
#include "ringbuf.h"
#include "characters.h"
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

struct logs {
	int logfile;
	struct characters *chars;
	struct ringbuffer *rb;
	size_t max_entries;
	size_t used_entries;
	size_t next_entry;
	size_t buf_cursor;
	char buf[1024];
	struct entry entries[];
};

struct logs *logs_create(int logfile, size_t names, size_t rb_size, size_t entries) {
	if (entries == 0) {
		return NULL;
	}
	struct logs *res = malloc(sizeof(*res) + entries * sizeof(res->entries[0]));
	if (res == NULL) {
		return res;
	}
	res->rb = ringbuffer_create(rb_size);
	if (res->rb == NULL) {
		free(res);
		return NULL;
	}
	res->chars = characters_create(names);
	if (res->chars == NULL) {
		ringbuffer_destroy(res->rb);
		free(res);
		return NULL;
	}
	res->logfile = logfile;
	res->used_entries = 0;
	res->next_entry = 0;
	res->buf_cursor = 0;
	res->max_entries = entries;
	return res;
}

void logs_destroy(struct logs *logs) {
	if (logs != NULL) {
		if (logs->chars != NULL) {
			characters_destroy(logs->chars);
		}
		if (logs->rb != NULL) {
			ringbuffer_destroy(logs->rb);
		}
		memset(logs, 0, sizeof(*logs));
		free(logs);
	}
	return;
}

static void add_to_logs(struct logs *lgs, const char *text, const struct entry *entry) {
	size_t start = ringbuffer_offset(lgs->rb);
	size_t next_start = start + ringbuffer_written(lgs->rb);
	size_t free_space = ringbuffer_size(lgs->rb) - ringbuffer_written(lgs->rb);
	size_t to_be_erased = 0;
	if (lgs->used_entries == lgs->max_entries) {
		/* Clear oldest entry */
		to_be_erased += lgs->entries[lgs->next_entry % lgs->max_entries].text.size;
		--lgs->used_entries;
	}
	while ((free_space + to_be_erased) < entry->text.size) {
		to_be_erased += lgs->entries[(lgs->next_entry - lgs->used_entries) % lgs->max_entries].text.size;
		--lgs->used_entries;
	}
	ringbuffer_erase(lgs->rb, start, to_be_erased);
	(void)ringbuffer_write(lgs->rb, next_start, text + entry->text.offset, entry->text.size);
	lgs->entries[lgs->next_entry % lgs->max_entries] = *entry;
	lgs->entries[lgs->next_entry % lgs->max_entries].text.offset = next_start;
	++lgs->next_entry;
	++lgs->used_entries;
	return;
}

int logs_refresh(struct logs *lgs) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	ssize_t rd = read(lgs->logfile, lgs->buf + lgs->buf_cursor, sizeof(lgs->buf) - lgs->buf_cursor);
	if (rd < 0) {
		return -1;
	}
        if (rd == 0) {
		return 1;
	}
	size_t bc = 0;
	while (1) {
		while ((rd > 0) && (lgs->buf[lgs->buf_cursor] != '\n')) {
			++lgs->buf_cursor;
			--rd;
		}
		if (rd <= 0) {
			memmove(lgs->buf, lgs->buf + bc, lgs->buf_cursor - bc);
			lgs->buf_cursor -= bc;
			return 0;
		} else {
			struct entry e;
			int r = entry_parser(lgs->chars, lgs->buf + bc, lgs->buf_cursor - bc, &e);
			if ((r == 0) && (e.text.size <= ringbuffer_size(lgs->rb))) {
				if ((e.text.size > 0) && (lgs->buf[bc + e.text.offset + e.text.size - 1] == '\r')) {
					--e.text.size;
				}
				add_to_logs(lgs, lgs->buf + bc, &e);
			}
			++lgs->buf_cursor;
			--rd;
			bc = lgs->buf_cursor;
		}
	}
}

int logs_get_text(const struct logs *lgs, size_t start, size_t size, char *data) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return ringbuffer_read(lgs->rb, start, data, size);
}

int logs_get_entry(const struct logs *lgs, size_t index, struct entry *entry) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	if (entry == NULL) {
		errno = EFAULT;
		return -1;
	}
	if (index >= lgs->next_entry) {
		errno = EDOM;
		return -1;
	}
	if ((lgs->next_entry - lgs->used_entries) > index) {
		errno = EDOM;
		return -1;
	}
	*entry = lgs->entries[index % lgs->max_entries];
	return 0;
}

size_t logs_get_used_entries(const struct logs *lgs) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return lgs->used_entries;
}

size_t logs_get_next_entry(const struct logs *lgs) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return lgs->next_entry;
}

int logs_index_source(struct logs *lgs, const char *name, size_t *index) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return characters_hash(lgs->chars, name, index);
}

int logs_deindex_source(struct logs *lgs, size_t index) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return characters_unhash(lgs->chars, index);
}

int logs_name_source(struct logs *lgs, size_t index, char *name, size_t max_name_size) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return characters_get_name(lgs->chars, index, name, max_name_size);
}

int logs_name_complete(struct logs *lgs, const char *name, size_t *level, size_t *index) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return characters_complete(lgs->chars, name, level, index);
}

int logs_name_next_complete(struct logs *lgs, size_t level, size_t *index) {
	if (lgs == NULL) {
		errno = EFAULT;
		return -1;
	}
	return characters_next_complete(lgs->chars, level, index);
}

