NODE := node

SCRIPT := ../scripts/codegen.js
CONF_TYPES := ../asc/vm_type.json
CONF_OPCODES := ../asc/vm_op.json

all: vm_type.c vm_type.h vm_op.h

vm_type.c: $(SCRIPT) $(CONF_TYPES)
	$(NODE) $(SCRIPT) -t $(CONF_TYPES)
vm_type.h: vm_type.c

vm_op.h: $(SCRIPT) $(CONF_OPCODES)
	$(NODE) $(SCRIPT) -c $(CONF_OPCODES)

clean:
	rm -f vm_type.c vm_type.h vm_op.h
