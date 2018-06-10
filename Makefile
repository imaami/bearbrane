# Get the absolute path of this makefile's folder.
override DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

######################## DO NOT ADD ANYTHING ABOVE THIS ########################

CC := gcc
CFLAGS := -std=gnu11 -Og -ggdb -march=native -Wall -Wextra

# Build output
ifeq ($(strip $(O)),)
override O := $(DIR)
else
override O := $(O:%/=%)/
endif

# Dependencies
override DEP := $(DIR)dep/

# Sources
override SRC := fcgi.c

# Objects
override OBJ := $(SRC:%.c=$(O)%.o)

# Programs
override EXE := $(O)fcgi

# Output binaries (objects + programs)
override BIN := $(OBJ) $(EXE)

.PHONY: default
default: $(O)fcgi

.PHONY: clean
clean: clean-kcgi
	rm -fr $(BIN)

.PHONY: clean-kcgi
clean-kcgi:
	@[ ! -e "$(DEP)kcgi/Makefile.configure" ] || $(MAKE) -C "$(DEP)kcgi" distclean

override ALL_CFLAGS := -I"$(DEP)kcgi" $(CFLAGS)
override LDFLAGS := -L"$(DEP)kcgi"
override LDLIBS  := -lkcgi -lz

override define __o =
$$(O)$(1:%.c=%.o): $$(DIR)$(1) $$(DIR)$(1:%.c=%.h) $(2)
	@[ -z "$$(O)" -o -e "$$(O)" ] || mkdir -pv "$$(O)"
	$$(CC) -c $$(ALL_CFLAGS) "$$<" -o "$$@"
endef

$(foreach f,$(SRC),$(eval $(call __o,$(f),$(DEP)kcgi/kcgi.h)))

$(DEP)kcgi/kcgi.h:
	git submodule update --init "$(DEP)kcgi"

$(DEP)kcgi/libkcgi.a: $(DEP)kcgi/kcgi.h
	cd "$(DEP)kcgi" && CC="$(CC)" CFLAGS="$(CFLAGS)" ./configure && $(MAKE)

$(O)fcgi: $(OBJ) $(DEP)kcgi/libkcgi.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) $(LDLIBS) -o "$@"
