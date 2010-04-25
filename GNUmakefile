SRC=$(wildcard src/*.c)
ASM=$(wildcard asm/*.S)

BUILDIR=bin
ST_OBJ=$(SRC:src/%.c=$(BUILDIR)/%.o) $(ASM:asm/%.S=$(BUILDIR)/%.o)
PR_OBJ=$(SRC:src/%.c=$(BUILDIR)/%.po) $(ASM:asm/%.S=$(BUILDIR)/%.po)
SH_OBJ=$(SRC:src/%.c=$(BUILDIR)/%.So) $(ASM:asm/%.S=$(BUILDIR)/%.So)

DEPDIR=.depend
DEP=$(SRC:src/%.c=$(DEPDIR)/%.dep) $(ASM:asm/%.S=$(DEPDIR)/%.dep)

LIB=weecrypt
ST_LIB=$(BUILDIR)/lib$(LIB).a
PR_LIB=$(BUILDIR)/lib$(LIB)_p.a
SH_LIB=$(BUILDIR)/lib$(LIB).so

OS=$(shell uname)
ifeq ($(OS),Darwin)
	CC=/Developer/usr/bin/clang
else
	CC=gcc
endif
COPTS=-O -g3 -m32
ASFLAGS=-m32
#COPTS=-O2 -march=core2 -fomit-frame-pointer -ftracer -DNDEBUG
CFLAGS=-Wall -W $(COPTS) -Iinclude
PIC=-DPIC -fPIC
PROF=-pg

PREFIX?=/usr/local
INCDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
INSTALL=install

TEST_SRC=$(wildcard *.c)
TEST_BIN=$(TEST_SRC:%.c=$(BUILDIR)/%)

all: $(ST_LIB) $(TEST_BIN)

install: $(ST_LIB) $(SH_LIB)
	$(INSTALL) -o 0 -g 0 -m 644 *.h $(INCDIR)
	$(INSTALL) -o 0 -g 0 -m 755 $(ST_LIB) $(LIBDIR)
	$(INSTALL) -o 0 -g 0 -m 755 $(SH_LIB) $(LIBDIR)
	strip --strip-debug $(LIBDIR)/$(SH_LIB) $(LIBDIR)/$(ST_LIB)

$(ST_LIB): $(ST_OBJ)
	ar cru $(@) $(ST_OBJ)

$(PR_LIB): $(PR_OBJ)
	ar cru $(@) $(PR_OBJ)

$(SH_LIB): $(SH_OBJ)
	$(CC) -shared -o $(@) $(SH_OBJ)

$(BUILDIR)/%.o: asm/%.S
	$(CC) $(ASFLAGS) -Iinclude -c $(<) -o $(@)

$(BUILDIR)/%.po: asm/%.S
	$(CC) $(ASFLAGS) -Iinclude $(PROF) -c $(<) -o $(@)

$(BUILDIR)/%.So: asm/%.S
	$(CC) -Iinclude $(PIC) -c $(<) -o $(@)

$(BUILDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $(<) -o $(@)

$(BUILDIR)/%.po: src/%.c
	$(CC) $(CFLAGS) $(PROF) -c $(<) -o $(@)

$(BUILDIR)/%.So: src/%.c
	$(CC) $(CFLAGS) $(PIC) -c $(<) -o $(@)

$(BUILDIR)/%: %.c $(ST_LIB)
	$(CC) $(CFLAGS) -o $(@) $(<) $(ST_LIB)

.PHONY : clean
clean :
	rm -f $(ST_LIB) $(PR_LIB) $(SH_LIB)
	rm -f $(ST_OBJ) $(PR_OBJ) $(SH_OBJ)
	rm -f $(TEST_BIN)
	rm -rf $(DEPDIR)

$(DEPDIR)/%.dep : %.c
	@$(SHELL) -c '[ -d $(DEPDIR) ] || mkdir $(DEPDIR)'
	@$(SHELL) -ec 'echo -n "Rebuilding dependencies for $< - "; \
		$(CC) -M $(CFLAGS) $< > $@; \
		sed "s/^$(<:%.c=%.o)/& $(<:%.c=%.po) $(<:%.c=%.So)/" $@ > $(DEPDIR)/$(<:%.c=%.dep2); \
		mv -f $(DEPDIR)/$(<:%.c=%.dep2) $@; \
		echo ok.'

$(DEPDIR)/%.dep : %.S
	@$(SHELL) -c '[ -d $(DEPDIR) ] || mkdir $(DEPDIR)'
	@$(SHELL) -ec 'echo -n "Rebuilding dependencies for $< - "; \
		$(CC) -M $(CFLAGS) $< > $@; \
		sed "s/^$(<:%.S=%.o)/& $(<:%.S=%.po) $(<:%.S=%.So)/" $@ > $(DEPDIR)/$(<:%.S=%.dep2); \
		mv -f $(DEPDIR)/$(<:%.S=%.dep2) $@; \
		echo ok.'

-include $(DEP)
