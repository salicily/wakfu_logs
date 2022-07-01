#ifndef LOG_ENTRY
#define LOG_ENTRY

#include "entry.h"
#include <stddef.h>

struct logs;

/* Create a new log engine with:
 * - logfile: opened descriptor on a stream containing the logs issued by Wakfu
 * - names: maximum number of supported players
 * - rb_size: maximum number of bytes to be logged
 * - entries: maximum number of logged entries (older entries are automatically discarded when full)
 *
 * Returns NULL on failure
 */
struct logs *logs_create(int logfile, size_t names, size_t rb_size, size_t entries);

void logs_destroy(struct logs *logs);

/* Read the log file to update the entries.
 * Returns 0 on success with updates, 1 if nothing has been updated, or -1 on failure.
 */
int logs_refresh(struct logs *lgs);

/* Returns the text from the indicated buffer, usually start is e->offset, and size e->size where e is an entry */
int logs_get_text(const struct logs *lgs, size_t start, size_t size, char *data);

/* Get the corresponding entry, returns 0 on success, -1 on failure.
 * Failure includes the following cases:
 * - next_entry - used_entries > index: in this case, the entry has been discarded since,
 * - next_entry <= index: in this case the entry has not been emitted yet
 */
int logs_get_entry(const struct logs *lgs, size_t index, struct entry *entry);

/* Get the number of currently stored entries */
size_t logs_get_used_entries(const struct logs *lgs);

/* Get the next entry number to be issued after refresh */
size_t logs_get_next_entry(const struct logs *lgs);

/* Indexes provided source, either name is already known and its index returned,
 * either name is not yet known and this makes it known and its index returned.
 * Returns 0 on success, -1 on failure.
 */
int logs_index_source(struct logs *lgs, const char *name, size_t *index);

/* When too many characters are present, it is possible to remove some from the database.
 * Note that this will not remove corresponding entries.
 * Such entries will just be "orphaned" (no known player), or bound to a new player
 * (this can be fixed by storing somewhere the first entry bound to a player).
 */
int logs_deindex_source(struct logs *lgs, size_t index);

/* Get the explicit name of a player provided its index */
int logs_name_source(struct logs *lgs, size_t index, char *name, size_t max_name_size);

/* Provided a prefix of a player find the first matching player,
 * returns 0 if found, -1 on error.
 * Level is the size of the provided name, it is used for later calls to log_name_next_complete,
 * so that returned result still have the correct prefix.
 */
int logs_name_complete(struct logs *lgs, const char *name, size_t *level, size_t *index);

/* Provided a prefix of a player find the next matching player,
 * returns 0 if found, -1 on error.
 */
int logs_name_next_complete(struct logs *lgs, size_t level, size_t *index);

#endif /* LOG_ENGINE */

