#include "../term.h"
#include "command.h"
#include "config.h"
#include "logview.h"
#include "raw_mode.h"
#include "window_print.h"
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <string.h>

static siginfo_t siginfo;
static sig_atomic_t lb = 0;
static sig_atomic_t la = 1;

static void term_resize_handler(int signo, siginfo_t *info, void *context) {
	if (signo != SIGWINCH) {
		return;
	}
	siginfo = *info;
	lb = la;
	return;
}


struct iface_state {
	struct config *cfg;
	size_t width;
	size_t height;
	size_t focused_entry;
};

static struct iface_state term_ = {0};

static struct iface_state *term_init(void) {
	struct sigaction old_sa;
	struct sigaction new_sa;
	int r;
	if (term_.cfg != NULL) {
		config_destroy(term_.cfg);
		term_.cfg = NULL;
	}
	term_.cfg = config_create(32768);
	if (term_.cfg == NULL) {
		return NULL;
	}
	new_sa.sa_sigaction = term_resize_handler;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = SA_SIGINFO | SA_RESTART;
	r = sigaction(SIGWINCH, &new_sa, &old_sa);
	if (r != 0) {
		if (errno == EINVAL) {
			dprintf(2, "Signal SIGWINCH is not a valid signal number for sigaction.\n");
		} else {
			dprintf(2, "Unexpected error %d during call to sigaction on signal SIGWINCH.\n", errno);
		}
		return NULL;
	}
	if (old_sa.sa_flags & SA_SIGINFO) {
		dprintf(2, "Old handler for signal SIGWINCH was %p.\n", old_sa.sa_sigaction);
	} else {
		if (old_sa.sa_handler == SIG_IGN) {
			dprintf(2, "Old handler for signal SIGWINCH was ignored.\n");
		} else if (old_sa.sa_handler == SIG_DFL) {
			dprintf(2, "Old handler for signal SIGWINCH was default.\n");
		} else {
			dprintf(2, "Old handler for signal SIGWINCH was %p\n", old_sa.sa_handler);
		}
	}
	setlocale(LC_ALL, "en_US.utf8");
 	struct winsize ws;
	if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		dprintf(2, "Cannot get terminal dimensions\n");
		return NULL;
	} else {
		term_.width = ws.ws_col;
		term_.height = ws.ws_row;
	}
	if (term_.width < 60) {
		printf("Terminal window is too narrow (%zu)\n", term_.width);
		return NULL;
	}
	if (term_.height < 10) {
		printf("Terminal window is too small (%zu)\n", term_.height);
		return NULL;
	}
	enter_raw_mode();
	return &term_;
}

static int term_refresh(struct iface_state *state, struct logs *logs) {
	static char buffer[256];
	_Bool update_window = 0;
	_Bool quit = 0;
	_Bool resized = 0;
	_Bool lv_needs_refresh;
	// siginfo_t inf;
	while (lb == la) {
		la = !la;
		// inf = siginfo;
		update_window = 1;
	}
	if (update_window) {
	       struct winsize ws;
	       if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		       printf("Cannot get terminal dimensions\r\n");
		       return -1;
	       } else {
		       resized = (state->width != ws.ws_col) || (state->height != ws.ws_row);
		       state->width = ws.ws_col;
		       state->height = ws.ws_row;
	       }
	}
	if (state->width < 60) {
		printf("Terminal window is too narrow\r\n");
		return 1;
	}
	if (state->height < 10) {
		printf("Terminal window is too small\r\n");
		return 1;
	}
	ssize_t rd = read(0, buffer, sizeof(buffer));
	if (rd < 0) {
		return -1;
	}
	refresh_inputs(state->cfg, logs, 1, state->width, state->height - 1, 2, buffer, rd, resized, &state->focused_entry, &quit, &lv_needs_refresh);
	if (quit) {
		return 0;
	}
	if (lv_needs_refresh) {
		(void)log_view(state->cfg, logs, 1, state->width, 1, state->height - 2, &state->focused_entry);
	}
	flush_ostream();
	return 1;
}

static void term_release(struct iface_state *state) {
	restore_mode();
	config_destroy(state->cfg);
	return;
}

struct interface term = {
	.name = "term",
	.init = term_init,
	.refresh = term_refresh,
	.release = term_release,
};

