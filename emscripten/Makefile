CC := emcc
NODE := node
MAKE := make

CFLAGS := -Wall -Wextra -std=c11 -O3 -g4 -DNDEBUG
INCLUDE_PATH := ../core

VPATH := ..
SCRIPT := ../scripts/codegen.js
OUTDIR := build

SOURCES := \
	$(subst ../,,\
		$(wildcard ../core/*.c) \
		$(wildcard ../stdlib/*.c) \
	) \
	vm_extern_native.c
OBJECTS := $(addprefix $(OUTDIR)/,$(patsubst %.c,%.bc,$(SOURCES)))
EXTERNS := $(wildcard ../stdlib/*.c)

EMOPTS := \
	-s ASSERTIONS=0 \
	-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
	-s WARN_ON_UNDEFINED_SYMBOLS=0 \
	-s STRICT=1 \
	-s INVOKE_RUN=0 \
	-s MALLOC=emmalloc \
	-s EXPORTED_FUNCTIONS=@exports.json \
	-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE="['malloc','free']" \
	-s TOTAL_MEMORY=8388608 \
	-s TOTAL_STACK=2097152 \
	-s WASM_MEM_MAX=0

all: pre-build main-build

pre-build:
	@$(MAKE) -C ../core

main-build: $(OUTDIR)/asvm.wasm $(OUTDIR)/asvm.wat $(OUTDIR)/asvm.js

wasm: pre-build $(OUTDIR)/asvm.wasm $(OUTDIR)/asvm.wat

$(OUTDIR)/%.bc: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(addprefix -I,$(INCLUDE_PATH)) -MMD -MF $(patsubst %.bc,%.d,$@) -o $@ $<

vm_extern_native.c: $(SCRIPT) $(EXTERNS)
	$(NODE) $(SCRIPT) $(addprefix -l ,$(EXTERNS))

%.wat: %.wasm
	$(EMSDK)/upstream/bin/wasm2wat $< -o $@

$(OUTDIR)/asvm.wasm: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(EMOPTS) --llvm-lto 3 $(OBJECTS) -o $(OUTDIR)/asvm.wasm --source-map-base /
	$(EMSDK)/upstream/bin/wasm2wat $(OUTDIR)/asvm.wasm -o $(OUTDIR)/asvm.wat

$(OUTDIR)/asvm.js: asvm.ts
	./node_modules/typescript/bin/tsc

clean:
	rm -rf $(OUTDIR)
	rm -f vm_extern_native.c

-include $(patsubst %.bc,%.d,$(OBJECTS))