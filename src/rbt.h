#ifndef RBT_HEADER
#define RBT_HEADER

#include <stddef.h>

/* Abstract structure for RedBlack trees. */
struct rbt;

/* Returns the required alignment to store a RedBlack tree. */
size_t rbt_alignment(void);

/* Returns the number of bytes required to store the RedBlack tree storing at most [keys] keys. */
size_t rbt_required_size(size_t keys);

/* Initializes a part of the provided memory area.
 * NULL is returned in case of failure.
 * Otherwise a valid pointer is returned.
 * [*data] is the start of the memory data where the RedBlack tree has to be allocated,
 *         it is then updated to store the first byte after the RedBlack tree in memory.
 * [*data_size] is the number of bytes of the provided memory range,
 *              it is updated to store the number of bytes of the memory area after the shrink.
 * [key_size] is the size of all keys used to sort data in RedBlack tree.
 * [cell_size] is the gap between two keys in a separate array, so that keys can be recovered from hashes.
 * [first_key] is the start of the keys array. The key bound to hash [h] is at address [first_key] + [h] * [cell_size].
 * [keys] is the maximum number of keys to be managed by the RedBlack tree.
 */
struct rbt *rbt_init_empty(void **data, size_t *data_size, size_t key_size, size_t cell_size, void *first_key, size_t keys);

/* All following functions return 1 in case of success, and 0 in case of failure */

/* Successful if [hash] is not bound in [rbt]. */
_Bool rbt_is_free_hash(const struct rbt *rbt, size_t hash);

/* Successful if [hash] is bound in [rbt]. */
_Bool rbt_is_bound_hash(const struct rbt *rbt, size_t hash);

/* Successful if [rbt] is not full, returns a free hash in [*hash] if so. */
_Bool rbt_get_free(const struct rbt *rbt, size_t *hash);

/* Successful if [rbt] is not empty, returns the minimum in [*hash] if so. */
_Bool rbt_get_least(const struct rbt *rbt, size_t *hash);

/* Successful if [rbt] is not empty, returns the maximum in [*hash] if so. */
_Bool rbt_get_greatest(const struct rbt *rbt, size_t *hash);

/* Successful if [hash] is bound, but not the greatest, returns the following bound in [*hash]. */
_Bool rbt_get_next_hash(const struct rbt *rbt, size_t hash, size_t *next);

/* Successful if [hash] is bound, but not the least, returns the previous bound in [*hash]. */
_Bool rbt_get_previous_hash(const struct rbt *rbt, size_t hash, size_t *previous);

/* Successful if [key] is bound in [rbt], returns its associated hash in [*hash] if so. */
_Bool rbt_get_hash(const struct rbt *rbt, const void *key, size_t *hash);

/* Successful in the following two cases:
 * - [rbt] is not empty, [key] is not bound in [rbt] and [*hash] is free in [rbt],
 * - [key] is bound to some hash in [rbt].
 * In the first case, the [key] is bound to [*hash].
 * In the second case, [*hash] is updated to contain the hash bound to [key].
 */
_Bool rbt_bind_key(struct rbt *rbt, const void *key, size_t *hash);

/* Successful if [hash] is bound to some key in [rbt], in that case, it is then unbound. */
_Bool rbt_unbind(struct rbt *rbt, size_t hash);

#endif

