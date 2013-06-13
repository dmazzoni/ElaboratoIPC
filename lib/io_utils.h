#ifndef IO_UTILS_H
#define IO_UTILS_H

#include "list.h"

int itoa(int num, char *const buffer, int buf_len);
list* parse_file(const char *const pathname);
void write_results(const char *const pathname, int *results, int length);
void write_to_fd(int fd, const char *const s);

#endif
