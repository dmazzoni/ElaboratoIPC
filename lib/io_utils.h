#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "list.h"

list* parse_file(const char *const pathname);
void write_err(const char *const message);

#endif
