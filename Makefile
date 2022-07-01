define BUILD_OBJ

build/$(1).dep: src/$(1).c
	mkdir -p build/$$(dir $(1))
	gcc -Wall -M -MF $$(@) -MT build/$(1).o $$(^)

include build/$(1).dep

build/$(1).o:
	mkdir -p build/$$(dir $(1))
	gcc -Wall -o $$(@) -c src/$(1).c

endef

INTERFACES := dummy basic simple_colors inout

SOURCES := rbt characters ringbuf entry_parser log_engine interfaces wlog $(addprefix interfaces/,$(INTERFACES))

wlog: $(addprefix build/,$(addsuffix .o, $(SOURCES)))
	gcc -Wall -o wlog $(^)

$(foreach component, $(SOURCES), $(eval $(call BUILD_OBJ,$(component))))

clean:
	rm -Rf build

.PHONY: clean


