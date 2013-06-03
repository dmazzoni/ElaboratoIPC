#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "list.h"

list* parse_file(int fd);
void write_err(const char *const message);

#endif