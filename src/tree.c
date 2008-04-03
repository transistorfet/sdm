/*
 * Name:	tree.c
 * Description:	Binary Tree
 */

#include <stdlib.h>
#include <string.h>

#include <sdm/tree.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

static void sdm_tree_destroy_subtree(struct sdm_tree *, struct sdm_tree_entry *);

/**
 * Allocate a binary tree generic data.
 */
struct sdm_tree *create_sdm_tree(short bitflags, destroy_t destroy)
{
	struct sdm_tree *env;

	if (!(env = (struct sdm_tree *) memory_alloc(sizeof(struct sdm_tree))))
		return(NULL);
	env->bitflags = bitflags;
	env->destroy = destroy;
	env->traverse_next = NULL;
	env->root = NULL;
	return(env);
}

/**
 * Free resources allocated by the binary tree.
 */
void destroy_sdm_tree(struct sdm_tree *env)
{
	sdm_tree_destroy_subtree(env, env->root);
	memory_free(env);
}


/**
 * Add an entry of "name" to the binary tree.
 */
int sdm_tree_add(struct sdm_tree *env, const char *name, void *data)
{
	int res;
	struct sdm_tree_entry *prev, *entry;

	if (!name || !data || (env->bitflags & SDM_TBF_NO_ADD))
		return(-1);
	/** Search for an existing entry */
	for (entry = env->root, prev = NULL; entry; ) {
		if (!(res = strcmp(name, entry->name)))
			return(-1);
		prev = entry;
		entry = (res < 0) ? entry->left : entry->right;
	}

	if (!(entry = (struct sdm_tree_entry *) memory_alloc(sizeof(struct sdm_tree_entry) + strlen(name) + 1)))
		return(-1);
	entry->name = (char *) (entry + 1);
	strcpy(entry->name, name);
	entry->data = data;

	entry->left = NULL;
	entry->right = NULL;
	entry->parent = prev;
	if (!prev)
		env->root = entry;
	else if (strcmp(name, prev->name) < 0)
		prev->left = entry;
	else
		prev->right = entry;
	return(0);
}

/**
 * Replace the entry's data with "data".
 */
int sdm_tree_replace(struct sdm_tree *env, const char *name, void *data)
{
	int res;
	struct sdm_tree_entry *cur;

	if (!name || !data || (env->bitflags & SDM_TBF_NO_REPLACE))
		return(-1);
	for (cur = env->root; cur; ) {
		if (!(res = strcmp(name, cur->name))) {
			if (env->destroy)
				env->destroy(cur->data);
			cur->data = data;
			return(0);
		}
		cur = (res < 0) ? cur->left : cur->right;
	}
	return(-1);
}

/**
 * Remove an entry of "name" from binary tree
 */
int sdm_tree_remove(struct sdm_tree *env, const char *name)
{
	int res;
	struct sdm_tree_entry *cur, *tmp;

	if (!name || (env->bitflags & SDM_TBF_NO_REMOVE))
		return(-1);
	for (cur = env->root; cur; ) {
		if (!(res = strcmp(name, cur->name))) {
			if (!cur->left && !cur->right) {
				if (!cur->parent)
					env->root = NULL;
				else if (cur->parent->left == cur)
					cur->parent->left = NULL;
				else
					cur->parent->right = NULL;
			}
			else if (cur->left && cur->right) {
				tmp = cur->left;
				while (tmp->right)
					tmp = tmp->right;
				if (cur->left == tmp) {
					cur->left = tmp->left;
					if (tmp->left)
						tmp->left->parent = cur;
				}
				else {
					tmp->parent->right = tmp->left;
					if (tmp->left)
						tmp->left->parent = tmp->parent;
				}
				tmp->left = cur->left;
				tmp->right = cur->right;
				tmp->parent = cur->parent;
				if (cur == env->root)
					env->root = tmp;
			}
			else {
				tmp = cur->left ? cur->left : cur->right;
				if (!cur->parent)
					env->root = tmp;
				else if (cur->parent->left == cur)
					cur->parent->left = tmp;
				else
					cur->parent->right = tmp;
				tmp->parent = cur->parent;
			}
			// TODO this is incorrect but the correct way depends on how you traverse
			if (env->traverse_next == cur)
				env->traverse_next = NULL;
			if (env->destroy)
				env->destroy(cur->data);
			memory_free(cur);
			return(0);
		}
		cur = (res < 0) ? cur->left : cur->right;
	}
	return(-1);
}

/**
 * Find the value bound to name in table.
 */
struct sdm_tree_entry *sdm_tree_find_entry(struct sdm_tree *env, const char *name)
{
	int res;
	struct sdm_tree_entry *cur;

	for (cur = env->root; cur; ) {
		if (!(res = strcmp(name, cur->name)))
			return(cur);
		cur = (res < 0) ? cur->left : cur->right;
	}
	return(NULL);
}

/**
 * Find the value bound to partial name.
 */
struct sdm_tree_entry *sdm_tree_find_entry_partial(struct sdm_tree *env, const char *name)
{
	int res, len;
	struct sdm_tree_entry *cur;

	len = strlen(name);
	for (cur = env->root; cur; ) {
		if (!(res = strncmp(name, cur->name, len)))
			return(cur);
		cur = (res < 0) ? cur->left : cur->right;
	}
	return(NULL);
}

void sdm_tree_traverse_reset(struct sdm_tree *env)
{
	env->traverse_next = env->root;
}

struct sdm_tree_entry *sdm_tree_traverse_next_entry(struct sdm_tree *env)
{
	struct sdm_tree_entry *entry;

	if (!env->traverse_next)
		return(NULL);
	entry = env->traverse_next;
	if (env->traverse_next->left)
		env->traverse_next = env->traverse_next->left;
	else if (env->traverse_next->right)
		env->traverse_next = env->traverse_next->right;
	else {
		for (; env->traverse_next->parent; env->traverse_next = env->traverse_next->parent) {
			if (env->traverse_next->parent->left == env->traverse_next) {
				env->traverse_next = env->traverse_next->parent->right;
				return(entry);
			}
		}
		env->traverse_next = NULL;
	}
	return(entry);
}

/*** Local Functions ***/

static void sdm_tree_destroy_subtree(struct sdm_tree *env, struct sdm_tree_entry *entry)
{
	if (entry->left)
		sdm_tree_destroy_subtree(env, entry->left);
	if (entry->right)
		sdm_tree_destroy_subtree(env, entry->right);
	if (env->destroy)
		env->destroy(entry->data);
	memory_free(entry);
}


