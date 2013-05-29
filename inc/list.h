#ifndef LIST_H
#define LIST_H

struct list;
typedef struct list list;

list *list_construct();
void list_destruct(list *l);
int list_append(list *const l, char *const s);
char *list_extract(list *const l);
int list_count(const list *const l);

#endif
