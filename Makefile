CC            = gcc
CFLAGS        = -pedantic -Wall -Wextra -Werror -std=c99
DEBUG_FLAGS   = -O0 -g -DDEBUG
RELEASE_FLAGS = -O2 -flto
EXEC          = count

RELEASE_DIR   = Release
RELEASE_EXEC  = $(RELEASE_DIR)/$(EXEC)
RELEASE_OBJS  = $(addprefix $(RELEASE_DIR)/, $(OBJS))
RELEASE_DEPS  = $(addprefix $(RELEASE_DIR)/, $(DEPS))
DEBUG_DIR     = Debug
DEBUG_EXEC    = $(DEBUG_DIR)/$(EXEC)
DEBUG_OBJS    = $(addprefix $(DEBUG_DIR)/, $(OBJS))
DEBUG_DEPS    = $(addprefix $(DEBUG_DIR)/, $(DEPS))

all: release

release: $(RELEASE_EXEC)

debug: $(DEBUG_EXEC)

$(RELEASE_EXEC): main.c dynamic_array.h Makefile | $(RELEASE_DIR)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(filter %.c %.o %.s,$^) -o $@

$(DEBUG_EXEC): main.c dynamic_array.h Makefile | $(DEBUG_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(filter %.c %.o %.s,$^) -o $@

$(RELEASE_DIR) $(DEBUG_DIR):
	mkdir $@

clean:
	rm -rf $(RELEASE_DIR) $(DEBUG_DIR) *.o *.d

.PHONY: all clean release debug
