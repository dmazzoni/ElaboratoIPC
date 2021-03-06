CC:= gcc
LD:= gcc
CFLAGS:= -c -Wall -Ilib

PROC_LIBS:= lib/io_utils.c lib/ipc_utils.c
MAIN_LIBS:= $(PROC_LIBS) lib/list.c

PROC_OBJS:= $(PROC_LIBS:.c=.o)
MAIN_OBJS:= $(MAIN_LIBS:.c=.o)

PROC_HEADERS:= $(PROC_LIBS:.c=.h) lib/operation.h
MAIN_HEADERS:= $(MAIN_LIBS:.c=.h) lib/operation.h

all: main.x processor.x

main.x: main.o $(MAIN_OBJS)
	@echo Linking $@
	@$(LD) -o $@ $^
	
processor.x: processor.o $(PROC_OBJS)
	@echo Linking $@
	@$(LD) $^ -o $@
	
main.o: main.c $(MAIN_HEADERS)
	@echo $@
	@$(CC) $(CFLAGS) $< -o $@
	
processor.o: processor.c $(PROC_HEADERS)
	@echo $@
	@$(CC) $(CFLAGS) $< -o $@
	
lib/i%.o: lib/i%.c lib/i*.h
	@echo $@
	@$(CC) $(CFLAGS) $< -o $@
	
lib/list.o: lib/list.c lib/list.h
	@echo $@
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -f *.o lib/*.o *.x

.PHONY: all clean
