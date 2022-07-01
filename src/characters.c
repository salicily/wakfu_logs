#include "characters.h"
#include <string.h>
#include "rbt.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

struct character_entry {
	char name[64];
};

struct characters {
	size_t tot_size;
	size_t max_names;
	struct rbt *rbt;
	struct character_entry *config;
	char data_pool[];
};

struct characters *characters_create(size_t names) {
	size_t data_pool_off = offsetof(struct characters, data_pool);
	size_t confa = _Alignof(struct character_entry);
	size_t conf_start = ((data_pool_off + confa - 1) / confa) * confa;
	size_t conf_size = sizeof(struct character_entry) * names;
	size_t conf_end = conf_start + conf_size;
	size_t rbta = rbt_alignment();
	size_t rbt_start = ((conf_end + rbta - 1) / rbta) * rbta;
	size_t rbt_size = rbt_required_size(names);
	size_t rbt_end = rbt_start + rbt_size;
	size_t charsa = _Alignof(struct characters);
	size_t alignment = sizeof(void *);
	alignment = (charsa > alignment) ? charsa : alignment;
	alignment = (confa > alignment) ? confa : alignment;
	alignment = (rbta > alignment) ? rbta : alignment;
	struct characters *res = NULL;
	int r = posix_memalign((void **)&res, alignment, rbt_end);
	if (r != 0) {
		return NULL;
	}
	res->config = (struct character_entry *)(res->data_pool + (conf_start - data_pool_off));
	void *data = (void *)(res->data_pool + (conf_end - data_pool_off));
	size_t data_size = rbt_end - conf_end;
	res->rbt = rbt_init_empty(&data, &data_size, sizeof(res->config[0].name), sizeof(res->config[0]), res->config[0].name, names);
	if (res->rbt == NULL) {
		free(res);
		return NULL;
	}
	res->tot_size = rbt_end;
	return res;
}

void characters_destroy(struct characters *chars) {
	if (chars != NULL) {
		free(chars);
	}
	return;
}

size_t characters_max_names(struct characters *chars) {
	if (chars == NULL) {
		return 0;
	}
	return chars->max_names;
}

int characters_hash(struct characters *chars, const char *name, size_t *hash) {
	if ((chars == NULL) || (name == NULL) || (hash == NULL)) {
		errno = EFAULT;
		return -1;
	}
	char nm[sizeof(chars->config[0].name)];
	memset(nm, 0, sizeof(nm));
	strncpy(nm, name, sizeof(nm));
	if (!rbt_get_free(chars->rbt, hash)) {
		if (!rbt_get_hash(chars->rbt, (void *)nm, hash)) {
			errno = ENOSPC;
			return -1;
		}
		return 0;
	}
	size_t aux = *hash;
	if (!rbt_bind_key(chars->rbt, (void *)nm, hash)) {
		errno = EFAULT;
		return -1;
	}
	if (aux == *hash) {
		memcpy(chars->config[*hash].name, nm, sizeof(nm));
	}
	return 0;
}

int characters_unhash(struct characters *chars, size_t hash) {
	if (chars == NULL) {
		errno = EFAULT;
		return -1;
	}
	if (!rbt_unbind(chars->rbt, hash)) {
		errno = ENOENT;
		return -1;
	}
	return 0;
}

int characters_get_name(struct characters *chars, size_t hash, char *name, size_t name_size) {
	if ((chars == NULL) || (name == NULL) || (name_size <= 0)) {
		errno = EFAULT;
		return -1;
	}
	if (!rbt_is_bound_hash(chars->rbt, hash)) {
		errno = ENOENT;
		return -1;
	}
	if (name_size > sizeof(chars->config[0].name)) {
		name_size = sizeof(chars->config[0].name);
	}
	strncpy(name, chars->config[hash].name, name_size);
	name[name_size - 1] = '\0';
	return 0;
}

int characters_complete(struct characters *chars, const char *name, size_t *level, size_t *hash) {
	if ((chars == NULL) || (hash == NULL) || (name == NULL) || (level == NULL)) {
		errno = EFAULT;
		return -1;
	}
	char nm[sizeof(chars->config[0].name)];
	memset(nm, 0, sizeof(nm));
	strncpy(nm, name, sizeof(nm));
	if (!rbt_get_free(chars->rbt, hash)) {
		if (!rbt_get_hash(chars->rbt, (void *)nm, hash)) {
			errno = ENOSPC;
			return -1;
		}
		return 0;
	}
	size_t aux = *hash;
	if (!rbt_bind_key(chars->rbt, (void *)nm, hash)) {
		errno = EFAULT;
		return -1;
	}
	*level = strnlen(name, sizeof(nm));
	if (aux != *hash) {
		return 0;
	}
	size_t next;
	_Bool t = rbt_get_next_hash(chars->rbt, *hash, &next);
	rbt_unbind(chars->rbt, *hash);
	if (!t) {
		errno = ENOENT;
		return -1;
	}
	if (memcmp(chars->config[*hash].name, chars->config[next].name, *level) != 0) {
		errno = ENOENT;
		return -1;
	}
	*hash = next;
	return 0;
}

int characters_next_complete(struct characters *chars, size_t level, size_t *hash) {
	if ((chars == NULL) || (hash == NULL)) {
		errno = EFAULT;
		return -1;
	}
	size_t next;
	if (!rbt_get_next_hash(chars->rbt, *hash, &next)) {
		errno = ENOENT;
		return -1;
	}
	if (level > sizeof(chars->config[0].name)) {
		level = sizeof(chars->config[0].name);
	}
	if (memcmp(chars->config[*hash].name, chars->config[next].name, level) != 0) {
		errno = ENOENT;
		return -1;
	}
	*hash = next;
	return 0;
}

