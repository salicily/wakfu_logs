#include "../../log_engine.h"
#include "config.h"
#include "debug.h"
#include "logview.h"
#include "window_print.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define mark printf("[l:%d]\r\n", __LINE__);

static void reset_style(void) {
	char header[32];
	int w;
	w = sprintf(header, "\x1b[0m");
	write_ostream(header, w);
	return;
}

static void apply_style(struct style *s) {
	char header[32];
	int w;
	/* Set channel style */
	if (s->has_background) {
		w = sprintf(header, "\x1b[48:5:%um", s->background);
		write_ostream(header, w);
	}
	if (s->has_foreground) {
		w = sprintf(header, "\x1b[38:5:%um", s->foreground);
		write_ostream(header, w);
	}
	if (s->bold) {
		w = sprintf(header, "\x1b[1m");
		write_ostream(header, w);
	}
	if (s->faint) {
		w = sprintf(header, "\x1b[2m");
		write_ostream(header, w);
	}
	if (s->italic) {
		w = sprintf(header, "\x1b[3m");
		write_ostream(header, w);
	}
	if (s->underline) {
		w = sprintf(header, "\x1b[4m");
		write_ostream(header, w);
	}
	return;
}

static void dbg_style(int i) {
	struct style s = {0};
	switch(i) {
		case 0:
			s.has_background = 1;
			s.background = 1;
			break;
		case 1:
			s.has_foreground = 1;
			s.foreground = 2;
			break;
		case 2:
			s.underline = 1;
			break;
		case 3:
			s.bold = 1;
			break;
	}
	reset_style();
	apply_style(&s);
	return;
}

int log_view(struct config *cfg, struct logs *lgs, size_t scol, size_t cols, size_t sline, size_t lines, size_t *entry_) {
debug("START\n");
	size_t time_size = 0;
	size_t name_size = 32;
	if (cfg->show_time) {
		time_size = 8;
	}
	size_t marge = time_size + name_size;
	if (marge >= cols) {
		return -1;
	}
	size_t text_width = cols - marge;
	size_t first_line = sline + lines;
	size_t next_entry = logs_get_next_entry(lgs);
	size_t used_entries = logs_get_used_entries(lgs);
	size_t entry = *entry_;
debug("  entry: %zu\n", entry);
debug("  first_line: %zu\n", first_line);
debug("  text_width: %zu\n", text_width);
debug("  next_entry: %zu\n", next_entry);
debug("  used_entries: %zu\n", used_entries);
	if (entry > next_entry) {
		entry = next_entry;
	}
	size_t first_entry = next_entry - used_entries;
	if (first_entry > entry) {
		entry = first_entry;
	}
	*entry_ = entry;
	size_t gap = next_entry - entry;
	used_entries -= gap;
debug("  used_entries: %zu\n", used_entries);
	while (used_entries > 0) {
		--entry;
debug("  entry: %zu\n", entry);
		--used_entries;
debug("  used_entries: %zu\n", used_entries);
		static char text[1024];
		int r;
		struct entry e;
		r = logs_get_entry(lgs, entry, &e);
		if (r != 0) {
			return -1;
		}
		if (e.chan >= (sizeof(cfg->channels) /  sizeof(cfg->channels[0]))) {
debug("  skipped: invalid channel\n");
			continue;
		}
		struct style *cst = cfg->channels + e.chan;
		struct style *st;
		if ((e.src >= cfg->max_profiles) || (!cfg->profiles[e.src].style.listed)) {
			st = &cfg->default_profile;
		} else {
			st = &cfg->profiles[e.src].style;
		}
		if (cst->hide || st->hide) {
debug("  skipped: hide\n");
			continue;
		}
		static char name[67];
		r = logs_name_source(lgs, e.src, name, sizeof(name));
		if (r != 0) {
			return -1;
		}
		size_t ts = (e.text.size > sizeof(text)) ? sizeof(text) : e.text.size;
debug("  ts: %zu\n", ts);
	       	r = logs_get_text(lgs, e.text.offset, ts, text);
		if (r != 0) {
			return -1;
		}
		ssize_t tl = text_lines(first_line, scol + marge, text_width, text, ts, 0, 0);
debug("  tl: %zd\n", tl);
		if (tl <= 0) {
			return -1;
		}
		if ((sline + tl) > first_line) {
			clear_lines(sline, scol, cols, first_line - sline);
			return 0;
		}
		first_line -= tl;
debug("  first_line: %zu\n", first_line);
		reset_style();
		apply_style(cst);
		apply_style(st);
dbg_style(0);
		text_lines(first_line, scol + marge, text_width, text, ts, 1, 0);
dbg_style(1);
		size_t nlen = strlen(name);
		if ((nlen + 3) <= sizeof(name)) {
			name[nlen] = ' ';
			name[nlen+1] = ':';
			name[nlen+2] = ' ';
			nlen += 3;
		}
		text_lines(first_line, scol + time_size, name_size, name, nlen, 1, 1);
		if (time_size > 0) {
			char time[10];
			unsigned int seconds = e.time % 60;
			unsigned int minutes = (e.time / 60) % 60;
			unsigned int hours = e.time / 3600;
			r = sprintf(time, "%02u:%02u:%02u", hours, minutes, seconds);
			if (r < 0) {
				return -1;
			}
dbg_style(2);
			text_lines(first_line, scol, time_size, time, r, 1, 0);
		}
dbg_style(3);
		clear_lines(first_line + 1, scol, marge, tl - 1);
	}
dbg_style(4);
	clear_lines(sline, scol, cols, first_line - sline);
	return 0;
}

