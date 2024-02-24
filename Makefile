CC            = gcc
CFLAGS        = -pedantic -W -Wall -Wextra \
                -Wconversion -Wswitch-enum \
                -Werror -std=c99 -O0 -g -I.

EXEC          = examples/count-words

all: $(EXEC)

$(EXEC): $(EXEC).c cdata.h Makefile
	$(CC) $(CFLAGS) $(filter %.c %.o %.s,$^) -o $@

clean:
	rm -rf $(EXEC) *.o *.d

.PHONY: all clean
