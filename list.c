#include <stdlib.h>
#include "inc/list.h"

typedef struct list_node {
	char *data;
	struct list_node *next;
} list_node;

struct list {
	list_node *head;
	list_node *tail;
	int elements;
};

static list_node* node_construct(char *const s);
static void list_destruct_recursive(list_node *n);

list* list_construct() {
	list *l = (list *) malloc(sizeof(list));
	l->head = NULL;
	l->tail = NULL;
	l->elements = 0;
	return l;
}


void list_destruct(list *l) {
	if (l) {
		list_destruct_recursive(l->head);
		free(l);	
	}
}

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

int list_count(const list *const l) {
	return l ? l->elements : -1;
}

static list_node* node_construct(char *const s) {
	list_node *node = (list_node *) malloc(sizeof(list_node));
	node->data = s;
	node->next = NULL;
	return node;
}

static void list_destruct_recursive(list_node *n) {
	if (n) {
		list_destruct_recursive(n->next);
		free(n->data);
		free(n);
	}
}
