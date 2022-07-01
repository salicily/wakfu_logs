#include <stdlib.h>
#include "ringbuf.h"

struct ringbuffer {
	size_t size;
	size_t start;
	size_t used;
	char data[];
};

struct ringbuffer *ringbuffer_create(size_t size) {
	if (size <= 0) {
		return NULL;
	}
	struct ringbuffer *rb = malloc(sizeof(*rb) + size);
	if (rb == NULL) {
		return NULL;
	}
	rb->size = size;
	rb->start = 0;
	rb->used = 0;
	return rb;
}

_Bool ringbuffer_valid(const struct ringbuffer *rb) {
	if (rb == NULL) {
		return 0;
	}
	if (rb->size <= 0) {
		return 0;
	}
	if (rb->used > rb->size) {
		return 0;
	}
	return 1;
}

void ringbuffer_destroy(struct ringbuffer *rb) {
	if (!ringbuffer_valid(rb)) {
		return;
	}
	rb->start = 0;
	rb->used = 0;
	rb->size = 0;
	return free(rb);
}

size_t ringbuffer_size(const struct ringbuffer *rb) {
	if (!ringbuffer_valid(rb)) {
		return 0;
	}
	return rb->size;
}

size_t ringbuffer_offset(const struct ringbuffer *rb) {
	if (!ringbuffer_valid(rb)) {
		return 0;
	}
	return rb->start;
}

size_t ringbuffer_written(const struct ringbuffer *rb) {
	if (!ringbuffer_valid(rb)) {
		return 0;
	}
	return rb->used;
}

static void rbc__(char *data, size_t size) {
	while (size > 0) {
		*data = 0;
		++data;
		--size;
	}
	return;
}

static void rbr__(const struct ringbuffer *rb, size_t offset, size_t size, char *data) {
	size_t right = rb->size - offset;
	const char *buf = rb->data + offset;
	if (right < size) {
		size -= right;
		while (right > 0) {
			*data = *buf;
			++buf;
			++data;
			--right;
		}
		buf = rb->data;
	}
	while (size > 0) {
		*data = *buf;
		++buf;
		++data;
		--size;
	}
	return;
}

static void rbw__(struct ringbuffer *rb, size_t offset, size_t size, const char *data) {
	size_t right = rb->size - offset;
	char *buf = rb->data + offset;
	if (right < size) {
		size -= right;
		while (right > 0) {
			*buf = *data;
			++buf;
			++data;
			--right;
		}
		buf = rb->data;
	}
	while (size > 0) {
		*buf = *data;
		++buf;
		++data;
		--size;
	}
	return;
}

static void rbz__(struct ringbuffer *rb, size_t offset, size_t size) {
	size_t right = rb->size - offset;
	char *buf = rb->data + offset;
	if (right < size) {
		size -= right;
		while (right > 0) {
			*buf = 0;
			++buf;
			--right;
		}
		buf = rb->data;
	}
	while (size > 0) {
		*buf = 0;
		++buf;
		--size;
	}
	return;
}

void ringbuffer_erase(struct ringbuffer *rb, size_t offset, size_t size) {
	if (!ringbuffer_valid(rb)) {
		return;
	}
	if (offset <= rb->start) {
		size_t gap = rb->start - offset;
		if (gap >= size) {
			return;
		}
		size_t beheaded = size - gap;
		if (beheaded > rb->used) {
			rb->start = 0;
			rb->used = 0;
			return;
		}
		rb->start += beheaded;
		rb->used -= beheaded;
		return;
	}
	size_t gap = offset - rb->start;
	if (gap >= rb->used) {
		return;
	}
	size_t beheaded = rb->used - gap;
	if (beheaded <= size) {
		rb->used = gap;
		return;
	}
	rbz__(rb, offset % rb->size, size);
	return;
}

int ringbuffer_read(const struct ringbuffer *rb, size_t offset, char *data, size_t size) {
	if (size <= 0) {
		return 0;
	}
	if (data == NULL) {
		return -1;
	}
	if (!ringbuffer_valid(rb)) {
		rbc__(data, size);
		return 0;
	}
	if (offset < rb->start) {
		size_t gap = rb->start - offset;
		if (gap > size) {
			gap = size;
		}
		size -= gap;
		offset += gap;
		rbc__(data, gap);
		data += gap;
	}
	size_t gap = offset - rb->start;
	if (gap < rb->used) {
		size_t rem = rb->used - gap;
		if (rem > size) {
			rem = size;
		}
		rbr__(rb, offset % rb->size, rem, data);
		offset += rem;
		data += rem;
		size -= rem;
	}
	rbc__(data, size);
	return 0;
}

int ringbuffer_write(struct ringbuffer *rb, size_t offset, const char *data, size_t size) {
	if (!ringbuffer_valid(rb)) {
		return -1;
	}
	if (size <= 0) {
		return 0;
	}
	if (data == NULL) {
		return -1;
	}
	if (rb->used <= 0) {
		rb->start = offset;
	}
	if (offset < rb->start) {
		size_t gap = rb->start - offset;
		size_t rem = rb->size - rb->used;
		if (gap > rem) {
			return -1;
		}
		rb->start = offset % rb->size;
		rb->used += gap;
		rbw__(rb, rb->start, size, data);
		data += size;
		offset += size;
		if (gap > size) {
			rbz__(rb, offset % rb->size, gap - size);
		}
		return 0;
	}
	size_t gap = offset - rb->start;
	if (gap > rb->size) {
		return -1;
	}
	size_t rem = rb->size - gap;
	if (size > rem) {
		return -1;
	}
	size_t old_end = rb->start + rb->used;
	if (gap > rb->used) {
		rbz__(rb, old_end % rb->size, gap - rb->used);
	}
	rbw__(rb, offset % rb->size, size, data);
	size_t delta = gap + size;
	if (delta > rb->used) {
		rb->used = delta;
	}
	return 0;
}

