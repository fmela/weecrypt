SRC=$(wildcard src/*.c)
ASM=$(wildcard asm/*.S)

BUILD_DIR=bin
ST_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.o) $(ASM:asm/%.S=$(BUILD_DIR)/%.o)
PR_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.po) $(ASM:asm/%.S=$(BUILD_DIR)/%.po)
SH_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.So) $(ASM:asm/%.S=$(BUILD_DIR)/%.So)

LIB=weecrypt
ST_LIB=$(BUILD_DIR)/lib$(LIB).a
PR_LIB=$(BUILD_DIR)/lib$(LIB)_p.a
SH_LIB=$(BUILD_DIR)/lib$(LIB).so

CC=$(shell which clang || which gcc)
ifeq ($(CC),)
$(error Neither clang nor gcc found)
endif

# On 64-bit platforms, uncomment this to generate 32-bit code.
#ARCH=-m32

#COPTS=-g3
COPTS=-O2
CFLAGS=-Wall -Wextra -Werror -Wshadow -std=c99 $(COPTS) $(ARCH) -Iinclude
PIC=-DPIC -fPIC
PROF=-pg

CUNIT_PREFIX=$(HOME)/homebrew

INSTALL_PREFIX?=/usr/local
INSTALL=install

TEST_SRC=$(wildcard *.c)
TEST_BIN=$(TEST_SRC:%.c=$(BUILD_DIR)/%)

all: $(BUILD_DIR) $(ST_LIB) $(TEST_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

install: $(ST_LIB) $(SH_LIB)
	$(INSTALL) -o 0 -g 0 -m 644 *.h $(INSTALL_PREFIX)/include
	$(INSTALL) -o 0 -g 0 -m 755 $(ST_LIB) $(INSTALL_PREFIX)/lib
	$(INSTALL) -o 0 -g 0 -m 755 $(SH_LIB) $(INSTALL_PREFIX)/lib
	strip --strip-debug $(INSTALL_PREFIX)/lib/$(SH_LIB) $(INSTALL_PREFIX)/lib/$(ST_LIB)

$(ST_LIB): $(ST_OBJ)
	ar cru $(@) $(ST_OBJ)

$(PR_LIB): $(PR_OBJ)
	ar cru $(@) $(PR_OBJ)

$(SH_LIB): $(SH_OBJ)
	$(CC) -shared -o $(@) $(SH_OBJ)

$(BUILD_DIR)/%.o: asm/%.S
	$(CC) $(ARCH) -Iinclude -c $(<) -o $(@)

$(BUILD_DIR)/%.po: asm/%.S
	$(CC) -Iinclude $(PROF) -c $(<) -o $(@)

$(BUILD_DIR)/%.So: asm/%.S
	$(CC) -Iinclude $(PIC) -c $(<) -o $(@)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $(<) -o $(@)

$(BUILD_DIR)/%.po: src/%.c
	$(CC) $(CFLAGS) $(PROF) -c $(<) -o $(@)

$(BUILD_DIR)/%.So: src/%.c
	$(CC) $(CFLAGS) $(PIC) -c $(<) -o $(@)

$(BUILD_DIR)/unit_tests: unit_tests.c $(ST_LIB)
	$(CC) $(CFLAGS) -I$(CUNIT_PREFIX)/include -L$(CUNIT_PREFIX)/lib -o $(@) $(<) $(ST_LIB) -lncurses -lcunit

$(BUILD_DIR)/%: %.c $(ST_LIB)
	$(CC) $(CFLAGS) -o $(@) $(<) $(ST_LIB)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: analyze
analyze:
	clang $(CFLAGS) --analyze $(SRC)
	rm -f *.plist
