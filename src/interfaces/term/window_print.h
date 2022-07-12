#ifndef WINDOW_PRINT
#define WINDOW_PRINT

#include <unistd.h>
#include <stddef.h>

/* Returns a negative value in case of an error, otherwise, returns the number of lines required for the print.
 * If print is set, also tries to print the provided text.
 */
ssize_t text_lines(size_t line, size_t col, size_t width, const char *text, size_t text_size, _Bool print, _Bool ljust);

/* Erase a rectangular area.
 */
void clear_lines(size_t line, size_t col, size_t width, size_t height);

/* Direct control on stream to set style */
void write_ostream(const char *text, size_t text_size);

void flush_ostream(void);

#endif /* WINDOW_PRINT */

