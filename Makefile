.SUFFIXES:

CFLAGS ?=
CFLAGS := -g -W -Wall -Wextra -Wmissing-prototypes $(CFLAGS)
DEPFLAGS = -MMD -MP
VERBOSE ?=
DEPFILES =
OBJFILES =
EXEFILES =

all: teststatic testdynamic fwcalc

# $(1): executable name
# $(2): extension
flags_from_ext = $(if $(filter $(2),cc),$($(1)_CXXFLAGS), \
	$(if $(filter $(2),c),$($(1)_CFLAGS), \
	$(if $(filter $(2),S),$($(1)_CFLAGS),)))

# $(1): executable name
# $(2): source name
# $(3): extension
# echo Compiling $$@ from $$^ >&2
define compile_ext=
ifeq ($(VERBOSE),1)
$$(info called compile_ext $(1) $(2) $(3))
endif
$(1)-$$(patsubst %.$(3),%.o,$(2)): $(2)
	$$(CC) -o $$@ \
		$$(CFLAGS) \
		$$(call flags_from_ext,$$(call toupper,$(1)),$(3)) \
		$$(DEPFLAGS) \
		-MF $(1)-$$(patsubst %.$(3),%.d,$(2)) \
		-c $$<
ifeq ($(VERBOSE),1)
$$(info Generated compile $(1)-$$(patsubst %.$(3),%.o,$(2)): $(2))
endif
OBJFILES += $(1)-$$(patsubst %.$(3),%.o,$(2))
DEPFILES += $(1)-$$(patsubst %.$(3),%.d,$(2))
endef

# $(1): name
# $(2): filename
#	echo Linking $$@ from $$^ >&2
define compile_exec=
ifeq ($(VERBOSE),1)
$$(info called compile_exec $(1) $(2))
endif
$$(foreach src,$$(filter %.c,$$($(1)_SRCS)),$$(eval $$(call \
	compile_ext,$(2),$$(src),c)))
$$(foreach src,$$(filter %.S,$$($(1)_SRCS)),$$(eval $$(call \
	compile_ext,$(2),$$(src),S)))

$(2): $$(patsubst %.c,$(2)-%.o,$$(filter %.c,$$($(1)_SRCS))) \
		$$(patsubst %.S,$(2)-%.o,$$(filter %.S,$$($(1)_SRCS)))
	$$(CC) -o $$@ $$(CFLAGS) $$($(1)_CFLAGS) $$^ $$(LDFLAGS) $$($(1)_LDFLAGS)
ifeq ($(VERBOSE),1)
$$(info Generated link $(2): \
	$$(patsubst %.c,$(2)-%.o,$$(filter %.c,$$($(1)_SRCS))) \
	$$(patsubst %.S,$(2)-%.o,$$(filter %.S,$$($(1)_SRCS))))
endif
EXEFILES += $(2)
endef

# $(1): listname
# $(2): source name
# $(3): flag
# $(4): variable
# $(5): operator
define setflagsof=
ifeq ($(VERBOSE),1)
$$(info setting flags: $(1)-$$(patsubst %.c,%.o,$(2)): $(4) $(5) $(3))
endif
$(1)-$$(patsubst %.c,%.o,$(2)): $(4) $(5) $(3)
endef

tolower = $(shell echo $(1) | tr '[:upper:]' '[:lower:]')
toupper = $(shell echo $(1) | tr '[:lower:]' '[:upper:]')

# $(1): listname
# $(2): flag
# $(3): varname
# $(4): operator
define adjvar=
$$(foreach src,$$(filter %.c,$$($$(call toupper,$(1))_SRCS)),$$(eval $$(call \
	setflagsof,$(1),$$(src),$(2),$(3),$(4))))
$(1): $(3) $(4) $(2)
endef

# $(1): listname
# $(2): flags
# $(3): varname
# $(4): operator
define adjvars=
$$(foreach flag,$$(eval $$(call adjvar $(1),$(flag),$(3),$(4))),$(2))
endef

TESTDYNAMIC_SRCS = calc.c freestand.c io.c dynamicmem.c
TESTSTATIC_SRCS = calc.c freestand.c io.c staticmem.c
FWCALC_SRCS = crt0-cortex-m0.S calc.c freestand.c seg7.c keyb.c staticmem.c

LISTS = testdynamic teststatic fwcalc

ARM_CC = arm-none-eabi-gcc
ARM_OBJDUMP = arm-none-eabi-objdump
ARM_M0_QEMU = qemu-system-arm

USE_LIBGCC = 0

ifeq ($(USE_LIBGCC),1)
ARM_LIBGCC = $(shell $(ARM_CC) -mcpu=cortex-m0 -print-libgcc-file-name)
endif

LINKER_SCRIPT_OPTION = -Wl,-Tcortex-m0.ld

FWCALC_CFLAGS = -ffreestanding \
	-Dmemcpy=__builtin_memcpy \
	-nostdlib \
	-mcpu=cortex-m0

FWCALC_LDFLAGS = $(LINKER_SCRIPT_OPTION)

ifeq ($(USE_LIBGCC),1)
FWCALC_CFLAGS += -DBIG_UC=1 \
	-DHAVE_LIBGCC=1
FWCALC_LDFLAGS += $(ARM_LIBGCC)
endif

# $(eval $(call adjvar,fwcalc,-ffreestanding,CFLAGS,+=))
# $(eval $(call adjvar,fwcalc,-Dmemcpy=__builtin_memcpy,CFLAGS,+=))
# $(eval $(call adjvar,fwcalc,-nostdlib,CFLAGS,+=))
# $(eval $(call adjvar,fwcalc,-mcpu=cortex-m0,CFLAGS,+=))
# ifeq ($(USE_LIBGCC),1)
# $(eval $(call adjvar,fwcalc,$$(ARM_LIBGCC),LDFLAGS,+=))
# $(eval $(call adjvar,fwcalc,-DHAVE_LIBGCC=1,CFLAGS,+=))
# $(eval $(call adjvar,fwcalc,-DBIG_UC=1,CFLAGS,+=))
# endif
# $(eval $(call adjvar,fwcalc,$$(LINKER_SCRIPT_OPTION),LDFLAGS,+=))
# $(eval $(call adjvar,testdynamic,-fsanitize=address,CFLAGS,+=))
# $(eval $(call adjvar,teststatic,-fsanitize=address,CFLAGS,+=))

$(eval $(call adjvar,fwcalc,$(ARM_CC),CC,=))

$(foreach list,$(LISTS),$(eval $(call compile_exec,$(call toupper,$(list)),$(list))))

-include $(DEPFILES)

test: teststatic testdynamic test1.expr test2.expr
	./teststatic test1.expr
	./teststatic test2.expr
	./testdynamic test1.expr
	./testdynamic test2.expr

disassemble: fwcalc
	$(ARM_OBJDUMP) -S fwcalc

qemu-debug-m0: fwcalc
	$(ARM_M0_QEMU) -M microbit -cpu cortex-m0 -kernel $^ -s -S

gdb-debug-m0: fwcalc
	gdb-multiarch fwcalc -ex 'target remote :1234' -ex 'layout src'

clean:
	rm -f $(EXEFILES) $(OBJFILES) $(DEPFILES)

.PHONY: all clean test disassemble qemu-debug-m0 gdb-debug-m0
