SRC=$(wildcard src/*.c)
ASM=$(wildcard asm/*.S)

BUILD_DIR=bin
STATIC_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.o) $(ASM:asm/%.S=$(BUILD_DIR)/%.o)
PROFILE_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.po) $(ASM:asm/%.S=$(BUILD_DIR)/%.po)
SHARED_OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.So) $(ASM:asm/%.S=$(BUILD_DIR)/%.So)

LIB=weecrypt
STATIC_LIB=$(BUILD_DIR)/lib$(LIB).a
PROFILE_LIB=$(BUILD_DIR)/lib$(LIB)_p.a
SHARED_LIB=$(BUILD_DIR)/lib$(LIB).so

CC:=$(shell which clang || which gcc)
ifeq ($(CC),)
$(error Neither clang nor gcc found)
endif

# On 64-bit platforms, uncomment this to generate 32-bit code.
#ARCH=-m32

#COPTS=-g
COPTS=-O2
CFLAGS=-Wall -Wextra -Werror -Wshadow -std=c99 $(COPTS) $(ARCH) -Iinclude
CXXFLAGS=-Wall -Wextra -Werror -Wshadow $(COPTS) $(ARCH) -Iinclude
PIC=-DPIC -fPIC
PROF=-pg

CUNIT_PREFIX=$(HOME)/homebrew

INSTALL_PREFIX?=/usr/local
INSTALL=install

TEST_SRC=$(wildcard *.c *.cc)
TEST_BIN=$(TEST_SRC:%.c=$(BUILD_DIR)/%) $(TEST_SRC:%.cc=$(BUILD_DIR)/%)

all: $(BUILD_DIR) $(STATIC_LIB) $(TEST_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

install: $(STATIC_LIB) $(SHARED_LIB)
	$(INSTALL) -o 0 -g 0 -m 644 *.h $(INSTALL_PREFIX)/include
	$(INSTALL) -o 0 -g 0 -m 755 $(STATIC_LIB) $(INSTALL_PREFIX)/lib
	$(INSTALL) -o 0 -g 0 -m 755 $(SHARED_LIB) $(INSTALL_PREFIX)/lib
	strip --strip-debug $(INSTALL_PREFIX)/lib/$(SHARED_LIB) $(INSTALL_PREFIX)/lib/$(STATIC_LIB)

$(STATIC_LIB): $(STATIC_OBJ)
	ar cru $(@) $(STATIC_OBJ)

$(PROFILE_LIB): $(PROFILE_OBJ)
	ar cru $(@) $(PROFILE_OBJ)

$(SHARED_LIB): $(SHARED_OBJ)
	$(CC) -shared -o $(@) $(SHARED_OBJ)

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

$(BUILD_DIR)/unit_tests: unit_tests.c $(STATIC_LIB)
	$(CC) $(CFLAGS) -I$(CUNIT_PREFIX)/include -L$(CUNIT_PREFIX)/lib -o $(@) $(<) $(STATIC_LIB) -lcunit

$(BUILD_DIR)/%: %.c $(STATIC_LIB)
	$(CC) $(CFLAGS) -o $(@) $(<) $(STATIC_LIB)

$(BUILD_DIR)/%: %.cc $(STATIC_LIB)
	$(CC) $(CXXFLAGS) -o $(@) $(<) $(STATIC_LIB)

.PHONY: clean
clean:
	if test -d $(BUILD_DIR); then rm -r $(BUILD_DIR); fi

.PHONY: analyze
analyze:
	@for x in $(SRC) $(TEST_SRC); do \
		echo Analyzing $$x ...; \
		clang --analyze $(CFLAGS) -I$(CUNIT_PREFIX)/include $$x -o /dev/null; \
	done
