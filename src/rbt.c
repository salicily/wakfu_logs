#include <stddef.h>
#include <stdint.h>
#include <string.h>

const size_t not_a_hash = SIZE_MAX;

/* We have two kind of nodes:
 * - free nodes which are red and whose parent is not a node.
 * - used nodes which do not verify this property.
 *
 * On a free node, we use a doubly linked list using
 * lesser as previous element and next as next element.
 *
 * On a used node, we use a doubly linked binary search tree,
 * which is not allowed to have one node and its parent both red.
 * Furthermore, we require that each path has the same number
 * of black nodes. Leaves count as black nodes.
 */
struct node {
	size_t parent;
	size_t child[2];
	size_t previous;
	size_t next;
	_Bool is_black;
};

struct rbt {
	size_t key_size;
	size_t cell_size;
	const char *first_key;
	size_t max_slots;
	size_t first_free;
	size_t black_depth;
	size_t root;
	size_t least;
	size_t greatest;
	struct node slots[];
};

#if 0
#include <stdio.h>
#include <stdlib.h>
static _Bool debug_rbt(const struct rbt *rbt) {
	if (rbt == NULL) {
		return 0;
	}
	if (rbt->max_slots > not_a_hash) {
		return 0;
	}
	size_t remaining_slots = rbt->max_slots;
	size_t prev;
	size_t current;
	_Bool parent_was_black;
	size_t black_depth;
	int dir; /* 0: from parent, 1: from lesser, 2: from greater */

	if (rbt->key_size > rbt->cell_size) {
		dprintf(2, "Key size is greater than cell size\n");
		return 0;
	}

	/* Check free list */
	dprintf(2, "\nChecking free list\n");
	dir = 1;
	prev = not_a_hash;
	current = rbt->first_free;
	while (current != not_a_hash) {
		dprintf(2, " %zu", current);
		if (remaining_slots <= 0) {
			dprintf(2, "\nInvalid: cycling\n");
			return 0;
		}
		if (current >= rbt->max_slots) {
			dprintf(2, "\nInvalid: geq than bound of %zu\n", rbt->max_slots);
			return 0;
		}
		if (rbt->slots[current].child[0] != not_a_hash) {
			dprintf(2, "\nInvalid: child[0] is %zu\n", rbt->slots[current].child[0]);
			return 0;
		}
		if (rbt->slots[current].child[1] != not_a_hash) {
			dprintf(2, "\nInvalid: child[1] is %zu\n", rbt->slots[current].child[1]);
			return 0;
		}
		if (rbt->slots[current].previous != prev) {
			dprintf(2, "\nInvalid: previous is %zu\n", rbt->slots[current].previous);
			return 0;
		}
		if (rbt->slots[current].parent != not_a_hash) {
			dprintf(2, "\nInvalid: has %zu as a parent\n", rbt->slots[current].parent);
			return 0;
		}
		if (rbt->slots[current].is_black) {
			dprintf(2, "\nInvalid: is not red\n");
			return 0;
		}
		--remaining_slots;
		prev = current;
		current = rbt->slots[current].next;
	}

	/* Check the nodes */
	dprintf(2, "\nChecking the nodes\n");
	dir = 0;
	prev = not_a_hash;
	current = rbt->root;
	parent_was_black = 0;
	black_depth = 0;
	size_t xprev = not_a_hash;
	size_t xnext = rbt->least;
	while (current != not_a_hash) {
		dprintf(2, "%zu:%c(%d,%c)", current, rbt->slots[current].is_black ? 'N' : 'R', dir, parent_was_black ? 'N' : 'R');
		if (current >= rbt->max_slots) {
			dprintf(2, "\nInvalid: geq than bound of %zu\n", rbt->max_slots);
			return 0;
		}
		switch (dir) {
			case 0: {
				if (rbt->slots[current].parent != prev) {
					dprintf(2, "\nInvalid: double link broken (parent is %zu, expecting %zu)\n", rbt->slots[current].parent, prev);
					return 0;
				}
				if (rbt->slots[current].is_black) {
					++black_depth;
				} else {
					if (!parent_was_black) {
						dprintf(2, "\nInvalid: two red consecutive nodes\n");
						return 0;
					}
				}
				parent_was_black = rbt->slots[current].is_black;
				if (remaining_slots <= 0) {
					dprintf(2, "\nInvalid: cycling\n");
					return 0;
				}
				--remaining_slots;
				prev = current;
				current = rbt->slots[current].child[0];
				if (current != not_a_hash) {
					if (rbt->slots[prev].child[1] == current) {
						dprintf(2, "\nInvalid: two children are the same\n");
						return 0;
					}
					dir = 0;
					continue;
				}
				if (black_depth != rbt->black_depth) {
					dprintf(2, "\nInvalid: Unbalanced tree (%zu, expected %zu)\n", black_depth, rbt->black_depth);
					return 0;
				}
				dir = 1;
				current = prev;
				prev = not_a_hash;
				continue;
			}
			case 1: {
				parent_was_black = rbt->slots[current].is_black;
				if (xnext != current) {
					dprintf(2, "\nIncorrectly doubly linked at %zu (expected %zu)\n", current, xnext);
					return 0;
				}
				if (xprev != rbt->slots[current].previous) {
					dprintf(2, "\nIncorrectly doubly linked at %zu (prev is %zu, expected %zu)\n", current, rbt->slots[current].previous, xprev);
					return 0;
				}
				xprev = current;
				xnext = rbt->slots[current].next;
				prev = current;
				current = rbt->slots[current].child[1];
				if (current != not_a_hash) {
					dir = 0;
					continue;
				}
				if (black_depth != rbt->black_depth) {
					dprintf(2, "\nInvalid: Unbalanced tree (%zu)\n", black_depth);
					return 0;
				}
				dir = 2;
				current = prev;
				prev = not_a_hash;
				continue;
			}
			case 2: {
				parent_was_black = rbt->slots[current].is_black;
				if (rbt->slots[current].is_black) {
					--black_depth;
				}
				prev = current;
				current = rbt->slots[current].parent;
				if (current != not_a_hash) {
					dir = (rbt->slots[current].child[0] == prev) ? 1 : 2;
				}
				continue;
			}
		}
	}
	if (xnext != not_a_hash) {
		dprintf(2, "\nUnfinished list\n");
		return 0;
	}
	if (xprev != rbt->greatest) {
		dprintf(2, "\nNot ending with greatest\n");
		return 0;
	}
	/* Check no leak */
	if (remaining_slots > 0) {
		dprintf(2, "\nNodes are leaking\n");
		return 0;
	}
	dprintf(2, "\nRBT fully checked\n");
	return 1;
}
#define DEBUG_RBT(rbt) do { if (!debug_rbt(rbt)) { exit(-1); } } while (0)
#else
#define DEBUG_RBT(rbt)
#endif

size_t rbt_alignment(void) {
	return _Alignof(struct rbt);
}

size_t rbt_required_size(size_t keys) {
	return sizeof(struct rbt) + keys * sizeof(struct node);
}

struct rbt *rbt_init_empty(void **data, size_t *data_size, size_t key_size, size_t cell_size, void *first_key, size_t keys) {
	if ((data == NULL) || (*data == NULL) || (data_size == NULL)) {
		return NULL;
	}
	if (key_size > cell_size) {
		return NULL;
	}
	size_t rbt_align = rbt_alignment();
	size_t prepad = (uintptr_t)*data % rbt_align;
	size_t pad = prepad ? rbt_align - prepad : 0;
	if (*data_size < pad) {
		return NULL;
	}
	size_t reqsize = rbt_required_size(keys);
	if ((*data_size - pad) < reqsize) {
		return NULL;
	}
	uintptr_t rbt_start = (uintptr_t)*data + pad;
	*data = (void *)(rbt_start + reqsize);
	*data_size = *data_size - pad - reqsize;
	struct rbt *rbt = (struct rbt *)rbt_start;

	rbt->key_size = key_size;
	rbt->cell_size = cell_size;
	rbt->first_key = first_key;
	rbt->max_slots = keys;
	rbt->first_free = 0;
	rbt->black_depth = 0;
	rbt->root = not_a_hash;
	rbt->least = not_a_hash;
	rbt->greatest = not_a_hash;
	for (size_t i = 0; i < keys; ++i) {
		rbt->slots[i].parent = not_a_hash;
		rbt->slots[i].is_black = 0;
		rbt->slots[i].child[0] = not_a_hash;
		rbt->slots[i].child[1] = not_a_hash;
		rbt->slots[i].previous = i - 1;
		rbt->slots[i].next = i + 1;
	}
	if (keys > 0) {
		rbt->slots[0].previous = not_a_hash;
		rbt->slots[keys - 1].next = not_a_hash;
	}
	return rbt;
}

_Bool rbt_get_free(const struct rbt *rbt, size_t *hash) {
	if (rbt == NULL) {
		return 0;
	}
	if (hash != NULL) {
		*hash = rbt->first_free;
	}
	return 1;
}

/* Auxiliary function, assuming rbt is a valid tree, parent is not NULL */
static int find_rbt_node(const struct rbt *rbt, const void *key, size_t *node_or_parent) {
	size_t index = rbt->root;
	*node_or_parent = not_a_hash;
	int c = -1;
	while (index != not_a_hash) {
		c = memcmp(key, rbt->first_key + index * rbt->cell_size, rbt->key_size);
		*node_or_parent = index;
		if (c == 0) {
			return 0;
		}
		index = rbt->slots[index].child[(c < 0) ? 0 : 1];
	}
	return c;
}

static size_t get_parent(const struct rbt *rbt, size_t node, _Bool *node_greater, size_t *brother) {
	size_t parent = rbt->slots[node].parent;
	if (parent != not_a_hash) {
		*node_greater = (rbt->slots[parent].child[0] != node);
		*brother = rbt->slots[parent].child[!*node_greater];
	}
	return parent;
}

static void rotate(struct rbt *rbt, size_t node, _Bool swap_with_greater) {
	size_t parent = rbt->slots[node].parent;
	size_t sub = rbt->slots[node].child[swap_with_greater];
	size_t subsub = rbt->slots[sub].child[!swap_with_greater];
	if (subsub != not_a_hash) {
		rbt->slots[subsub].parent = node;
	}
	rbt->slots[node].child[swap_with_greater] = subsub;
	if (parent != not_a_hash) {
		size_t lesser = rbt->slots[parent].child[0];
		rbt->slots[parent].child[lesser != node] = sub;
	} else {
		rbt->root = sub;
	}
	rbt->slots[sub].parent = parent;
	rbt->slots[node].parent = sub;
	rbt->slots[sub].child[!swap_with_greater] = node;
	return;
}

/* This function is to be called when:
 * - [node] is a red node belonging to [rbt],
 * - all paths to leaves from root have the same number of black nodes in [rbt],
 * - all red nodes, with maybe the exception of [node] in [rbt] have a black parent,
 * - the root of [rbt], if it is not [node] is black.
 *
 * If not sure that these properties hold, then DO NOT call this function.
 * This function fixes the third property, and ensures that the root is black,
 * making it a valid RBT.
 */
static void insertion_repair_rbt(struct rbt *rbt, size_t node) {
loop:
	/* Check if at root */
	_Bool node_greater;
	size_t brother;
	size_t parent = get_parent(rbt, node, &node_greater, &brother);
	if (parent == not_a_hash) {
		rbt->slots[node].is_black = 1;
		++rbt->black_depth;
		return;
	}
	/* Check if parent is black */
	if (rbt->slots[parent].is_black) {
		return;
	}
	/* Check if parent at root */
	_Bool parent_greater;
	size_t uncle;
	size_t gparent = get_parent(rbt, parent, &parent_greater, &uncle);
	if (gparent == not_a_hash) {
		rbt->slots[parent].is_black = 1;
		return;
	}
	/* Check if uncle is red */
	if ((uncle != not_a_hash) && !rbt->slots[uncle].is_black) {
		rbt->slots[parent].is_black = 1;
		rbt->slots[uncle].is_black = 1;
		rbt->slots[gparent].is_black = 0;
		node = gparent;
		goto loop;
	}
	/* Rotate if required */
	if (node_greater != parent_greater) {
		rotate(rbt, parent, node_greater);
		rbt->slots[node].parent = gparent;
		rbt->slots[gparent].child[parent_greater] = node;
		parent = node;
	}
	rotate(rbt, gparent, parent_greater);
	rbt->slots[gparent].is_black = 0;
	rbt->slots[parent].is_black = 1;
	return;
}

/* This function is to be called when:
 * - [node] is a black node belonging to [rbt] whose subtree is at a 1 missing black depth.
 *
 * If not sure that these properties hold, then DO NOT call this function.
 */
static void deletion_repair_rbt(struct rbt *rbt, size_t node) {
loop:
	/* Check if node is red */
	if (!rbt->slots[node].is_black) {
		rbt->slots[node].is_black = 1;
		return;
	}
	/* Check if at root */
	_Bool node_greater;
	size_t brother;
	size_t parent = get_parent(rbt, node, &node_greater, &brother);
	if (parent == not_a_hash) {
		--rbt->black_depth;
		return;
	}
	/* Check if brother is red */
	size_t nephew;
	size_t grand_nephew;
	if (!rbt->slots[brother].is_black) {
		rbt->slots[brother].is_black = 1;
		nephew = rbt->slots[brother].child[node_greater];
		grand_nephew = rbt->slots[nephew].child[node_greater];
		if ((grand_nephew != not_a_hash) && !rbt->slots[grand_nephew].is_black) {
			rotate(rbt, nephew, node_greater);
			nephew = grand_nephew;
		} else {
			rbt->slots[parent].is_black = 0;
		}
		rotate(rbt, parent, !node_greater);
		rotate(rbt, parent, !node_greater);
		return;
	}
	/* Look at nephew */
	nephew = rbt->slots[brother].child[node_greater];
	if ((nephew != not_a_hash) && !rbt->slots[nephew].is_black) {
		rbt->slots[nephew].is_black = rbt->slots[parent].is_black;
		rbt->slots[parent].is_black = 1;
		rotate(rbt, brother, node_greater);
		brother = nephew;
	} else {
		size_t xnephew = rbt->slots[brother].child[!node_greater];
		if ((xnephew == not_a_hash) || rbt->slots[xnephew].is_black) {
			rbt->slots[brother].is_black = 0;
			node = parent;
			goto loop;
		}
		rbt->slots[xnephew].is_black = rbt->slots[parent].is_black;
	}
	rotate(rbt, parent, !node_greater);
	return;
}

_Bool rbt_is_free_hash(const struct rbt *rbt, size_t hash) {
	if (rbt == NULL) {
		return 0;
	}
	if (hash >= rbt->max_slots) {
		return 0;
	}
	if (rbt->slots[hash].is_black) {
		return 0;
	}
	if (rbt->slots[hash].parent != not_a_hash) {
		return 0;
	}
	return 1;
}

_Bool rbt_is_bound_hash(const struct rbt *rbt, size_t hash) {
	if (rbt == NULL) {
		return 0;
	}
	if (hash >= rbt->max_slots) {
		return 0;
	}
	if (rbt->slots[hash].parent != not_a_hash) {
		return 1;
	}
	if (rbt->slots[hash].is_black) {
		return 1;
	}
	return 0;
}

_Bool rbt_get_least(const struct rbt *rbt, size_t *hash) {
	if (rbt == NULL) {
		return 0;
	}
	if (rbt->least == not_a_hash) {
		return 0;
	}
	if (hash != NULL) {
		*hash = rbt->least;
	}
	return 1;
}

_Bool rbt_get_greatest(const struct rbt *rbt, size_t *hash) {
	if (rbt == NULL) {
		return 0;
	}
	if (rbt->greatest == not_a_hash) {
		return 0;
	}
	if (hash != NULL) {
		*hash = rbt->greatest;
	}
	return 1;
}

_Bool rbt_get_hash(const struct rbt *rbt, const void *key, size_t *hash) {
	size_t nop;
	int c = find_rbt_node(rbt, key, &nop);
	if (c != 0) {
		return 0;
	}
	if (hash != NULL) {
		*hash = nop;
	}
	return 1;
}

_Bool rbt_get_next_hash(const struct rbt *rbt, size_t hash, size_t *next) {
	_Bool valid = rbt_is_bound_hash(rbt, hash);
	if (!valid) {
		return 0;
	}
	if (rbt->slots[hash].next == not_a_hash) {
		return 0;
	}
	if (next != NULL) {
		*next = rbt->slots[hash].next;
	}
	return 1;
}

_Bool rbt_get_previous_hash(const struct rbt *rbt, size_t hash, size_t *previous) {
	_Bool valid = rbt_is_bound_hash(rbt, hash);
	if (!valid) {
		return 0;
	}
	if (rbt->slots[hash].previous == not_a_hash) {
		return 0;
	}
	if (previous != NULL) {
		*previous = rbt->slots[hash].previous;
	}
	return 1;
}

_Bool rbt_bind_key(struct rbt *rbt, const void *key, size_t *hash) {
	if ((rbt == NULL) || (hash == NULL)) {
		return 0;
	}
	size_t node_or_parent;
	int r = find_rbt_node(rbt, key, &node_or_parent);
	if (r == 0) {
		/* Already here! */
		*hash = node_or_parent;
		return 1;
	}
	if (!rbt_is_free_hash(rbt, *hash)) {
		return 0;
	}

	size_t prev = rbt->slots[*hash].previous;
	size_t next = rbt->slots[*hash].next;
	if (prev == not_a_hash) {
		rbt->first_free = next;
	} else {
		rbt->slots[prev].next = next;
	}
	if (next != not_a_hash) {
		rbt->slots[next].previous = prev;
	}

	rbt->slots[*hash].parent = node_or_parent;
	rbt->slots[*hash].child[0] = not_a_hash;
	rbt->slots[*hash].child[1] = not_a_hash;
	rbt->slots[*hash].is_black = 0;
	if (node_or_parent != not_a_hash) {
		if (r > 0) {
			rbt->slots[*hash].previous = node_or_parent;
			rbt->slots[*hash].next = rbt->slots[node_or_parent].next;
			if (rbt->slots[*hash].next != not_a_hash) {
				rbt->slots[rbt->slots[*hash].next].previous = *hash;
			} else {
				rbt->greatest = *hash;
			}
			rbt->slots[node_or_parent].next = *hash;
			rbt->slots[node_or_parent].child[1] = *hash;
		} else {
			rbt->slots[*hash].next = node_or_parent;
			rbt->slots[*hash].previous = rbt->slots[node_or_parent].previous;
			if (rbt->slots[*hash].previous != not_a_hash) {
				rbt->slots[rbt->slots[*hash].previous].next = *hash;
			} else {
				rbt->least = *hash;
			}
			rbt->slots[node_or_parent].previous = *hash;
			rbt->slots[node_or_parent].child[0] = *hash;
		}
	} else {
		rbt->slots[*hash].previous = not_a_hash;
		rbt->slots[*hash].next = not_a_hash;
		rbt->root = *hash;
		rbt->least = *hash;
		rbt->greatest = *hash;
	}

	insertion_repair_rbt(rbt, *hash);
	DEBUG_RBT(rbt);
	return 1;
}

_Bool rbt_unbind(struct rbt *rbt, size_t hash) {
	_Bool valid = rbt_is_bound_hash(rbt, hash);
	if (!valid) {
		return 0;
	}
	size_t aux;
	size_t tmp;
	aux = rbt->slots[hash].next;
	if (aux == not_a_hash) {
		rbt->greatest = rbt->slots[hash].previous;
	} else {
		rbt->slots[aux].previous = rbt->slots[hash].previous;
	}
	aux = rbt->slots[hash].previous;
	if (aux == not_a_hash) {
		rbt->least = rbt->slots[hash].next;
	} else {
		rbt->slots[aux].next = rbt->slots[hash].next;
	}
	if (rbt->slots[hash].child[0] != not_a_hash) {
		struct node n = rbt->slots[aux];
		rbt->slots[aux] = rbt->slots[hash];
		rbt->slots[hash] = n;
		rbt->slots[aux].next = n.next;
		rbt->slots[aux].previous = n.previous;
		tmp = rbt->slots[aux].parent;
		if (tmp != not_a_hash) {
			if (rbt->slots[tmp].child[0] == hash) {
				rbt->slots[tmp].child[0] = aux;
			} else {
				rbt->slots[tmp].child[1] = aux;
			}
		} else {
			rbt->root = aux;
		}
		tmp = rbt->slots[aux].child[1];
		if (tmp != not_a_hash) {
			rbt->slots[tmp].parent = aux;
		}
		tmp = rbt->slots[hash].child[0];
		if (tmp != not_a_hash) {
			rbt->slots[tmp].parent = hash;
		}
		tmp = rbt->slots[aux].child[0];
		if (tmp != aux) {
			rbt->slots[tmp].parent = aux;
			rbt->slots[rbt->slots[hash].parent].child[1] = hash;
		} else {
			rbt->slots[aux].child[0] = hash;
			rbt->slots[hash].parent = aux;
		}
	}
	tmp = rbt->slots[hash].parent;
	aux = rbt->slots[hash].child[0];
	if (aux == not_a_hash) {
		aux = rbt->slots[hash].child[1];
	}
	if (aux != not_a_hash) {
		rbt->slots[aux].is_black = 1;
		rbt->slots[aux].parent = tmp;
	} else {
		if (rbt->slots[hash].is_black) {
			deletion_repair_rbt(rbt, hash);
		}
	}
	if (tmp == not_a_hash) {
		rbt->root = aux;
	} else {
		if (rbt->slots[tmp].child[0] == hash) {
			rbt->slots[tmp].child[0] = aux;
		} else {
			rbt->slots[tmp].child[1] = aux;
		}
	}
	/* Free hash! */
	rbt->slots[hash].child[0] = not_a_hash;
	rbt->slots[hash].child[1] = not_a_hash;
	rbt->slots[hash].parent = not_a_hash;
	rbt->slots[hash].previous = not_a_hash;
	rbt->slots[hash].next = rbt->first_free;
	rbt->slots[hash].is_black = 0;
	if (rbt->first_free != not_a_hash) {
		rbt->slots[rbt->first_free].previous = hash;
	}
	rbt->first_free = hash;
	DEBUG_RBT(rbt);
	return 1;
}
