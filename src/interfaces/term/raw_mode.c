#include "raw_mode.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios original_termios;
static _Bool already_raw = 0;

void restore_mode(void) {
	if (already_raw) {
		tcsetattr(0, TCSAFLUSH, &original_termios);
		already_raw = 0;
	}
	return;
}

int enter_raw_mode(void) {
	int r;
	if (already_raw) {
		return -1;
	}
	r = tcgetattr(0, &original_termios);
	if (r != 0) {
		return r;
	}
	struct termios copy = original_termios;
	copy.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	copy.c_oflag &= ~(OPOST);
	copy.c_cflag |=  (CS8);
	copy.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	copy.c_cc[VMIN] = 0;
	copy.c_cc[VTIME] = 10;
	r = tcsetattr(1, TCSAFLUSH, &copy);
	if (r != 0) {
		return r;
	}
	atexit(restore_mode);
	already_raw = 1;
	return 0;
}

