CC := clang
NODE := node

OUTDIR := .out

GENERATED_SOURCE := ../core/vm_externs.c ../core/vm_type.c
GENERATED_HEADERS := ../core/vm_op.h ../core/vm_type.h

SCRIPT := ../scripts/codegen.js
SCRIPT_INPUTS := ../scripts/vm_op.json ../scripts/vm_type.json $(wildcard ../stdlib/*.c)

CFLAGS := -Wall -Wextra -std=c11 -g -Og
INCLUDE_PATH := ../core

VPATH := ..

.DEFAULT_GOAL := all

SOURCES := $(subst ../,,$(wildcard ../core/*.c) $(wildcard ../stdlib/*.c) $(GENERATED_SOURCE)) main.c
OBJECTS := $(addprefix $(OUTDIR)/,$(patsubst %.c,%.o,$(SOURCES)))

all: $(GENERATED_SOURCE) $(GENERATED_HEADERS) vm

$(GENERATED_SOURCE) $(GENERATED_HEADERS): $(SCRIPT) $(SCRIPT_INPUTS)
	$(NODE) $(SCRIPT)

$(OUTDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(addprefix -I,$(INCLUDE_PATH)) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ $<

vm: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OUTDIR)
	rm -f vm

-include $(patsubst %.o,%.d,$(OBJECTS))
