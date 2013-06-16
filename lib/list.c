/** @file
	Contains the implementation of a linked list of strings, which 
	is used by the main process to store the operations to compute.<br>
	For details on functions, see @ref list and @ref list_node.
*/

#include <stdlib.h>
#include <string.h>
#include "list.h"


/** 
	@brief Represents a single list node.
	@see list
*/
typedef struct list_node {
	/// Pointer to the stored string.
	char *data;
	
	/// Pointer to the next node in the linked list.
	struct list_node *next;
} list_node;

///	Represents a FIFO linked list.
struct list {
	/// Pointer to the first node.
	list_node *head;
	
	/// Pointer to the last node.
	list_node *tail;
	
	/// Number of nodes in the list.
	int elements;
};

static list_node* node_construct(char *const s);
static void list_destruct_recursive(list_node *n);

/**
	Constructs an empty list.
	@return The created list on success, @c NULL otherwise.
	@memberof list
*/
list* list_construct() {
	list *l = (list *) malloc(sizeof(list));
	if(l) {
		l->head = NULL;
		l->tail = NULL;
		l->elements = 0;
	}
	return l; 
}

/**
	Destructs the list and all its nodes. Node contents are also 
	deallocated.<br>Runs in linear time.
	@param l The list to destruct
	@see list_destruct_recursive
	@memberof list
*/
void list_destruct(list *l) {
	if (l) {
		list_destruct_recursive(l->head);
		free(l);	
	}
}

/**
	Appends a node containing a copy of the passed string to the 
	tail of the specified list. <br>Runs in constant time.
	@param l The linked list
	@param s The string to append
	@return 0 on success, -1 otherwise.
	@see node_construct
	@memberof list
*/
int list_append(list *const l, char *const s) {
	list_node *node = node_construct(s);
	if (!l || !node)
		return -1;
	l->elements += 1;
	if (l->elements == 1)
		l->head = l->tail = node;
	else
		l->tail = l->tail->next = node;
	return 0;
}

/**
	Extracts a node from the head of the specified list and
	returns its content. The extracted node is deallocated.<br>
	Runs in constant time.
	@param l The linked list
	@return A pointer to the extracted string. @c NULL if the list was empty.
	@memberof list
*/
char* list_extract(list *const l) {
	char *res;
	list_node *new_head;
	if (!l || !l->elements)
		return NULL;
	res = l->head->data;
	new_head = l->head->next;
	free(l->head);
	l->elements -= 1;
	l->head = new_head;
	return res;
}

/**
	Returns the number of nodes currently stored in the list.
	@param l The linked list
	@return The number of nodes. -1 if a @c NULL list is passed.
	@memberof list
*/
int list_count(const list *const l) {
	return l ? l->elements : -1;
}

/**
	Constructs a single list node which contains a copy of the passed string.
	<br>The @c next field is initialized to @c NULL.
	@param s The string to copy into the node
	@return The created list node on success, @c NULL otherwise.
	@memberof list_node
*/
static list_node* node_construct(char *const s) {
	int size;
	char *dest;
	if (!s)
		return NULL;
	size = strlen(s) + 1;
	dest = (char *) malloc(size * sizeof(char));
	if (!dest)
		return NULL;
	strcpy(dest, s);
	list_node *node = (list_node *) malloc(sizeof(list_node));
	if (node) {
		node->data = dest;
		node->next = NULL;
	}
	return node;
}

/**
	Auxiliary function that recursively traverses the list, deallocating
	nodes and their content.
	@param n The starting node
	@see list_destruct
	@memberof list
*/
static void list_destruct_recursive(list_node *n) {
	if (n) {
		list_destruct_recursive(n->next);
		free(n->data);
		free(n);
	}
}
