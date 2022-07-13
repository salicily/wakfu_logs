#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"

struct window {
	size_t line;
	size_t column;
	size_t width;
	size_t height;
};

struct window_stream {
	const struct window win;
	size_t inner_line;
	size_t inner_column;
};

static const char * const graph_table[32] = {
	[ 0] =  "", [ 1] = "☺", [ 2] = "☻", [ 3] = "♥",
	[ 4] = "♦", [ 5] = "♣", [ 6] = "♠", [ 7] = "•",
	[ 8] = "◘", [ 9] = "○", [10] = "◙", [11] = "♂",
	[12] = "♀", [13] = "♪", [14] = "♫", [15] = "☼",
	[16] = "►", [17] = "◄", [18] = "↕", [19] = "‼",
	[20] = "¶", [21] = "§", [22] = "▬", [23] = "↨",
	[24] = "↑", [25] = "↓", [26] = "→", [27] = "←",
	[28] = "∟", [29] = "↔", [30] = "▲", [31] = "▼",
};

static const char * const graph_del = "⌂";

static char buf[256];
static size_t bytes = 0;

void write_ostream(const char *text, size_t text_size) {
	size_t rem = sizeof(buf) - bytes;
	while (text_size > rem) {
		memcpy(buf + bytes, text, rem);
		(void)write(1, buf, sizeof(buf));
		text_size -= rem;
		text += rem;
		rem = sizeof(buf);
		bytes = 0;
	}
	memcpy(buf + bytes, text, text_size);
	bytes += text_size;
	return;
}

void flush_ostream(void) {
	write(1, buf, bytes);
	bytes = 0;
	return;
}

/* Returns:
 * -1 if invalid character was found, or if ends up in non reset state for multibyte
 *  0 if the string is fully parsed
 *  1 if string terminator is found before end of text
 *  2 if text does not fit in line
 * In all cases, returns the number of parsed valid characters in *columns that fit a line if not NULL.
 * If 2 is returned, *text is updated for the next line, and *text_size also.
 */
static int get_text_line(size_t width, const char **text, size_t *text_size, size_t *columns, _Bool force) {
	const char *saved_text = *text;
	size_t saved_text_size = *text_size;
	size_t saved_columns = 0;
	size_t tot = 0;
	while (*text_size > 0) {
		int l = mblen(*text, *text_size);
		if (l <= 0) {
			if (columns != NULL) {
				*columns = tot;
			}
			return (l == 0) ? 1 : -1;
		}
		/* Extra check, should be removed for performances */
		if (l > *text_size) {
			dprintf(2, "\x1b[3J\x1b[0m%s:%d: should never happen\n", __FILE__, __LINE__);
			exit(-1);
			return -1;
		}
		tot += 1;
		if (tot > width) {
			if (columns != NULL) {
				*columns = saved_columns;
			}
			*text = saved_text;
			*text_size = saved_text_size;
			return 2;
		}
		if (force || ((l == 1) && (**text == ' '))) {
			saved_text = *text + 1;
			saved_text_size = *text_size - 1;
			saved_columns = tot;
		}
		*text += l;
		*text_size -= l;
	}
	if (columns != NULL) {
		*columns = tot;
	}
	return 0;
}

ssize_t text_lines(size_t line, size_t col, size_t width, const char *text, size_t text_size, _Bool print, _Bool ljust) {
	if (width <= 0) {
		return -1;
	}
	char header[32];
	size_t cols;
	ssize_t lines = 0;
	int r;
	_Bool force = 0;
	if (text_size == 0) {
		return 0;
	}
	r = 2;
	while (r == 2) {
		const char *old = text;
		r = get_text_line(width, &text, &text_size, &cols, force);
		if ((r >= 0) && (cols > 0) && print) {
			int w = sprintf(header, "\x1b[%zu;%zuH", line + lines, col);
			if (w < 0) {
				return -1;
			}
			write_ostream(header, w);
			if (ljust) {
				for (size_t i = 0; i < (width - cols); ++i) {
					write_ostream(" ", 1);
				}
			}
			write_ostream(old, text - old);
			if (!ljust) {
				for (size_t i = 0; i < (width - cols); ++i) {
					write_ostream(" ", 1);
				}
			}
		}
		force = (cols <= 0);
		lines += !force;
	}
	if ((r == 0) || (r == 1)) {
		return lines;
	}
	return -1;
}

void clear_lines(size_t line, size_t col, size_t width, size_t height) {
	if (width <= 0) {
		return;
	}
	char header[32];
	while (height > 0) {
		int w = sprintf(header, "\x1b[%zu;%zuH", line, col);
		if (w < 0) {
			return;
		}
		write_ostream(header, w);
		for (size_t i = 0; i < width; ++i) {
			write_ostream(" ", 1);
		}
		--height;
		++line;
	}
	return;
}

