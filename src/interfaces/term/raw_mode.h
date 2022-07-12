#ifndef RAW_MODE_H
#define RAW_MODE_H

/* Enter raw mode, does nothing if alreayd entered */
int enter_raw_mode(void);

/* Leave raw mode, does nothing if already left,
 * also called on leaving program.
 */
void restore_mode(void);

#endif /* RAW_MODE_H */

