/*
 * Name:	tree.h
 * Description:	Binary Tree Header
 */


#ifndef _SDM_TREE_H
#define _SDM_TREE_H

#include <sdm/globals.h>

#define SDM_TBF_NO_ADD			0x0001			/** No entries can be added */
#define SDM_TBF_NO_REPLACE		0x0002			/** No entries can be replaced */
#define SDM_TBF_NO_REMOVE		0x0004			/** No entries can be removed */
#define SDM_TBF_CONSTANT		0x0006			/** No entries can be replaced or removed */

struct sdm_tree_entry {
	char *name;
	void *data;
	struct sdm_tree_entry *parent;
	struct sdm_tree_entry *left;
	struct sdm_tree_entry *right;
};

struct sdm_tree {
	int bitflags;
	destroy_t destroy;
	struct sdm_tree_entry *traverse_next;
	struct sdm_tree_entry *root;
};

struct sdm_tree *create_sdm_tree(short, destroy_t);
void destroy_sdm_tree(struct sdm_tree *);

int sdm_tree_add(struct sdm_tree *, const char *, void *);
int sdm_tree_replace(struct sdm_tree *, const char *, void *);
int sdm_tree_remove(struct sdm_tree *, const char *);

struct sdm_tree_entry *sdm_tree_find_entry(struct sdm_tree *, const char *);
struct sdm_tree_entry *sdm_tree_find_entry_partial(struct sdm_tree *, const char *);

void sdm_tree_traverse_reset(struct sdm_tree *);
struct sdm_tree_entry *sdm_tree_traverse_next_entry(struct sdm_tree *);


static inline void *sdm_tree_find(struct sdm_tree *tree, const char *name) {
	struct sdm_tree_entry *entry;

	if (!(entry = sdm_tree_find_entry(tree, name)))
		return(NULL);
	return(entry->data);
}

static inline void *sdm_tree_find_partial(struct sdm_tree *tree, const char *name) {
	struct sdm_tree_entry *entry;

	if (!(entry = sdm_tree_find_entry_partial(tree, name)))
		return(NULL);
	return(entry->data);
}

static inline void *sdm_tree_traverse_next(struct sdm_tree *tree) {
	struct sdm_tree_entry *entry;

	if (!(entry = sdm_tree_traverse_next_entry(tree)))
		return(NULL);
	return(entry->data);
}

#endif

