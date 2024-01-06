################################################################################
################################# User config ##################################

CONFIG_FILE := config.mk

# Include a configuration file (if it doesn't exist, there's a rule below that generates one):
include $(CONFIG_FILE)

# If programmer is not specified, use a default value:
PROGRAMMER ?= usbasp

# If target is not specified, use a default value:
TARGET ?= application/main

################################################################################
################################ Variable setup ################################

# Specify root source and build directories:
SOURCE_DIR := source
BUILD_DIR := build

# Compiler flags for generating dependency files (see 'Dependency files' section below):
DEPENDENCY_FLAGS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d

# Other compiler flags:
COMPILER_FLAGS := -Wall -Os -flto -g -mmcu=atmega644p -DF_CPU=12000000

# Find target file:
ALL_TARGET_FILES := $(shell find $(SOURCE_DIR) -type f -name '*.target') # list all target files in the project
TARGET := $(firstword $(TARGET)) # take only the first target
TARGET_FILE := source/$(strip $(TARGET)).target # add full path to specified target file
ifeq (,$(filter $(TARGET_FILE),$(ALL_TARGET_FILES))) # check if specified target file exists
$(error Target file '$(TARGET_FILE)' was not found)
endif

# Generate ELF and ASM file names:
ELF_FILE := $(TARGET_FILE:$(SOURCE_DIR)/%.target=$(BUILD_DIR)/%.elf)
ASM_FILE := $(ELF_FILE:%.elf=%.asm)

# Include the target file, specifying the SOURCE_FILES variable:
include $(TARGET_FILE)
SOURCE_FILES := $(wildcard $(SOURCE_FILES)) # expand any wildcards in SOURCE_FILES

# Generate object and dependecy file names:
OBJECT_FILES := $(SOURCE_FILES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPENDENCY_FILES := $(SOURCE_FILES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.d)

################################################################################
############################# Targets and recipes ##############################

.PHONY: elf
elf: $(ELF_FILE)

.PHONY: asm
asm: $(ASM_FILE)

.PHONY: flash
flash: $(ELF_FILE)
	avrdude -c $(PROGRAMMER) -p m644p -U flash:w:$<

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(ELF_FILE): $(OBJECT_FILES)
	avr-gcc $(COMPILER_FLAGS) -o $@ $^

$(ASM_FILE): $(ELF_FILE)
	avr-objdump -D --section=.data --section=.text --source-comment -m avr5 $< > $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(BUILD_DIR)/%.d
	@mkdir -p $(@D)
	avr-gcc $(DEPENDENCY_FLAGS) $(COMPILER_FLAGS) -I include -c -o $@ $<

$(CONFIG_FILE):
	$(file >  $(CONFIG_FILE),TARGET ?= application/main)
	$(file >> $(CONFIG_FILE),PROGRAMMER ?= usbasp)

################################################################################
############################### Dependency files ###############################

# See https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# for information about how the dependencies are managed.

$(DEPENDENCY_FILES): ;

include $(DEPENDENCY_FILES)

################################################################################