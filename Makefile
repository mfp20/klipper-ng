#
# Author: Anichang
# Date  : 2020-01-01

OUT = bin/
PLATFORMS = linux generic328p melzi mksgenl

# --- END OF CONFIGURABLES ---

ifndef OUT
	OUT = bin/
endif

ifndef PLATFORMS
	PLATFORMS = linux generic328p melzi mksgenl
endif

COMMON_VARS = OUT=../$(OUT) PLATFORMS='$(PLATFORMS)'

# targets

linux: firmware-linux host-linux

firmware-linux:
	@echo "- $@"
	make $(COMMON_VARS) BOARD=linux -C firmware linux
	@echo $<" built!"

firmware-generic328p:
	@echo "- $@"
	make $(COMMON_VARS) BOARD=generic328p -C firmware generic328p
	@echo $<" built!"

firmware-melzi:
	@echo "- $@"
	make $(COMMON_VARS) BOARD=melzi -C firmware melzi
	@echo $<" built!"

firmware-mksgenl:
	@echo "- $@"
	make $(COMMON_VARS) BOARD=mksgenl -C firmware mksgenl
	@echo $<" built!"

host-linux:
	@echo "- $@ (TODO!)"
	@make $(COMMON_VARS) BOARD=linux -C host linux
	@echo $<" built!"

all: $(addprefix firmware-,$(PLATFORMS)) host-linux

clean: 
	@make -C firmware clean
	@make -C host clean

