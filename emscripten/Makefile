CC := emcc
NODE := node
MAKE := make

CFLAGS := -Wall -Wextra -std=c11 -O3 -Dinline="inline __attribute__((always_inline))" -DNDEBUG
INCLUDE_PATH := ../core

VPATH := ..
SCRIPT := ../scripts/codegen.js
OUTDIR := build

SOURCES := $(subst ../,,$(wildcard ../core/*.c) $(wildcard ../stdlib/*.c)) \
	core/vm_type.c vm_extern_native.c
OBJECTS := $(addprefix $(OUTDIR)/,$(patsubst %.c,%.bc,$(SOURCES)))
EXTERNS := $(wildcard ../stdlib/*.c)

EMOPTS := \
	-s ASSERTIONS=0 \
	-s INVOKE_RUN=0 \
	-s MALLOC=emmalloc \
	-s WARN_UNALIGNED=1 \
	-s ALLOW_TABLE_GROWTH=1 \
	-s ALIASING_FUNCTION_POINTERS=1 \
	-s EMULATED_FUNCTION_POINTERS=2 \
	-s FILESYSTEM=0 \
	-s EXPORTED_FUNCTIONS=@exports.json \
	-s STRICT=1 \
	-s ERROR_ON_MISSING_LIBRARIES=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=AsVM \
	-s DYNAMIC_EXECUTION=0 \
	-s USE_CLOSURE_COMPILER=0 \
	-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
	-s USES_DYNAMIC_ALLOC=0 \
	-s ASM_PRIMITIVE_VARS="['STACKTOP', 'STACK_MAX']" \
	-s MINIMAL_RUNTIME=2 \
	-s ALLOW_MEMORY_GROWTH=0 \
	-s DOUBLE_MODE=0 \
	-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE="['malloc','free']" \
	-s WARN_ON_UNDEFINED_SYMBOLS=0 \
	-s RUNTIME_FUNCS_TO_IMPORT="[]" \
	-s TOTAL_MEMORY=8388608 \
	-s TOTAL_STACK=2097152 \
	-s WASM_MEM_MAX=1073741824 \
	-s EMIT_EMSCRIPTEN_METADATA=1

all: pre-build main-build

pre-build:
	@$(MAKE) -C ../core

main-build: $(OUTDIR)/asvm.wasm $(OUTDIR)/asvm.js

$(OUTDIR)/%.bc: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(addprefix -I,$(INCLUDE_PATH)) -MMD -MF $(patsubst %.bc,%.d,$@) -o $@ $<

vm_extern_native.c: $(SCRIPT) $(EXTERNS)
	$(NODE) $(SCRIPT) $(addprefix -l ,$(EXTERNS))

$(OUTDIR)/asvm.wasm: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(EMOPTS) --js-opts 0 --emit-symbol-map $(OBJECTS) -o $(OUTDIR)/asvm2.js
	mv $(OUTDIR)/asvm2.wasm $(OUTDIR)/asvm.wasm

$(OUTDIR)/asvm.js: asvm.ts
	./node_modules/typescript/bin/tsc

clean:
	rm -rf $(OUTDIR)
	rm -f vm_extern_native.c
