#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "debug.h"

static int fd = -1;

static void vdebug(const char *format, va_list va) {
	if (fd < 0) {
		fd = open("debug", O_CREAT | O_TRUNC | O_RDWR, 0755);
		if (fd < 0) {
			return;
		}
	}
	(void)vdprintf(fd, format, va);
	return;
}

void debug(const char *format, ...) {
	va_list va;
       	va_start(va, format);
	vdebug(format, va);
	va_end(va);
	return;
}

