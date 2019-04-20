CC := gcc
NODE := node
MAKE := make

CFLAGS := -m32 -Wall -Wextra -std=c11 -g -Og -ftest-coverage -fprofile-arcs
INCLUDE_PATH := ../core

VPATH := ..
SCRIPT := ../scripts/codegen.js
OUTDIR := .out

SOURCES := $(subst ../,,$(wildcard ../core/*.c) $(wildcard ../stdlib/*.c)) \
	core/vm_type.c main.c vm_lib_test.c vm_extern_native.c
OBJECTS := $(addprefix $(OUTDIR)/,$(patsubst %.c,%.o,$(SOURCES)))
EXTERNS := $(wildcard ../stdlib/*.c) vm_lib_test.c

all: pre-build main-build

pre-build:
	@$(MAKE) -C ../core

main-build: vm

vm_extern_native.c: $(SCRIPT) $(EXTERNS)
	$(NODE) $(SCRIPT) $(addprefix -l ,$(EXTERNS))

$(OUTDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(addprefix -I,$(INCLUDE_PATH)) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ $<

vm: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OUTDIR)
	rm -f vm_extern_native.c

-include $(patsubst %.o,%.d,$(OBJECTS))
