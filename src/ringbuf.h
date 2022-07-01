#ifndef RINGBUF
#define RINGBUF

struct ringbuffer;

/* Allocate a ring buffer of provided size */
struct ringbuffer *ringbuffer_create(size_t size);

/* Check that the ring buffer is a valid one */
_Bool ringbuffer_valid(const struct ringbuffer *rb);

/* Release a ring buffer */
void ringbuffer_destroy(struct ringbuffer *rb);

/* Returns the size the ring buffer was created with */
size_t ringbuffer_size(const struct ringbuffer *rb);

/* Returns information about ring buffer:
 *
 * offset = ringbuffer_offset(rb);
 * written = ringbuffer_written(rb);
 *
 * implies that any byte read before offset or after offset+written will be 0.
 * written is lesser than or equal to ringbuffer_size(rb).
 *
 * When written is 0, offset can be any value.
 * In this case, offset value is not specified
 * (this implementation forces it to be set to 0,
 * but caller should not rely on it).
 */
size_t ringbuffer_offset(const struct ringbuffer *rb);
size_t ringbuffer_written(const struct ringbuffer *rb);

/* Clear part of the ringbuffer, making these bytes read as 0.
 * Calling ringbuffer_erase(rb) one one end of the ring buffer
 * will decrease ringbuffer_written(rb). This is necessary if the ring buffer
 * is filled and a call to ringbuffer_write needs to be performed.
 * Of course, erased data is completely lost.
 */
void ringbuffer_erase(struct ringbuffer *rb, size_t offset, size_t size);

/* Reads the ring buffer as if it were a normal buffer or arbitrary size.
 * Bytes which have never been written or which have been erased are read as 0.
 * Returns -1 if data is NULL while size is positive, or 0 otherwise.
 */
int ringbuffer_read(const struct ringbuffer *rb, size_t offset, char *data, size_t size);

/* Writes the ring buffer as if it were a normal buffer or arbitrary size.
 *
 * Following cases are considered errors, and in these cases,
 * the ring buffer is not modified:
 * - data is NULL while size is positive
 * - ring buffer is not valid
 * - the ring buffer would overlap with itself: ringbuffer_written(rb)
 *   would become greater than ringbuffer_size(rb).
 *
 * After a successful call, ringbuffer_written(rb) cannot be decreased,
 * while ringbuffer_offset(rb) cannot be increased
 * (excepted if ringbuffer_written(rb) was 0).
 */
int ringbuffer_write(struct ringbuffer *rb, size_t offset, const char *data, size_t size);

#endif

