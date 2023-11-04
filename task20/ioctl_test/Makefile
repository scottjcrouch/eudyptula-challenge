BIN_NAME=ioctl_test

BUILD_DIR := build

.PHONY:
all: | dirs build/bin/$(BIN_NAME)

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC_FILES = $(call rwildcard,src,*.c)
OBJ_FILES = $(SRC_FILES:src/%.c=build/%.o)
DEP_FILES = $(addsuffix .d,$(OBJ_FILES))

DEPFLAGS = -MMD -MP -MF $@.d

-include $(DEP_FILES)

INCLUDE_DIRS = \
        src/

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

CFLAGS += \
        -Og \
        -g3 \
        -Werror \
        -Wall \
        -Wextra \
        -Wshadow \
        -Wdouble-promotion \
        -Wformat=2 \
        -Wformat-overflow \
        -Wformat-truncation \
        -Wundef \
        -ffunction-sections \
        -fdata-sections \
        -fno-common

LDFLAGS += \
         -Wl,--gc-sections,-Map,$@.map

LDLIBS +=

dirs:
	mkdir -p build
	mkdir -p build/bin

build/%.o: src/%.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

build/bin/$(BIN_NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	- rm -rf build
