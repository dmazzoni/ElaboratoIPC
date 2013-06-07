#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "list.h"

int itoa(int num, char *const buffer, int buf_len);
list* parse_file(const char *const pathname);
void write_err(const char *const message);

#endif
