################################################################################
# Makefile
################################################################################

#===========================================================
# Check
#===========================================================
ifndef FORCE
    EXP_INFO := sel4devkit-maaxboard-microkit-docker-dev-env 1 *
    CHK_PATH_FILE := /check.mk
    ifeq ($(wildcard ${CHK_PATH_FILE}),)
        HALT := TRUE
    else
        include ${CHK_PATH_FILE}
    endif
    ifdef HALT
        $(error Expected Environment Not Found: ${EXP_INFO})
    endif
endif

#===========================================================
# Configure
#===========================================================
ifeq ($(strip $(ENV_BLOCK_ADDR_HEX)),)
# IMX8MDQLQRM: 11.1.3.1.1 DWC_usb3 Memory map
# Base Address for USB1: 0x38100000
# Final offset is 0xCC3C of size 32 bits, so total size of: 0xCC40
ENV_BLOCK_ADDR_HEX := 38100000
endif

ifeq ($(strip $(ENV_BLOCK_SIZE_BYTE_HEX)),)
ENV_BLOCK_SIZE_BYTE_HEX := CC40
endif

#===========================================================
# Layout
#===========================================================
DEP_PATH := dep
SRC_PATH := src
TMP_PATH := tmp
OUT_PATH := out

DEP_MKT_PATH := ${DEP_PATH}/microkit

#===========================================================
# Usage
#===========================================================
.PHONY: usage
usage: 
	@echo "usage: make <target> [ENV_BLOCK_ADDR_HEX=VALUE]"
	@echo "                     [ENV_BLOCK_SIZE_BYTE_HEX=VALUE]"
	@echo "                     [FORCE=TRUE]"
	@echo ""
	@echo "<target> is one off:"
	@echo "get"
	@echo "all"
	@echo "clean"
	@echo "reset"

#===========================================================
# Target
#===========================================================
CPU := cortex-a53
TCH := aarch64-linux-gnu
CC := ${TCH}-gcc
LD := ${TCH}-ld
AS := ${TCH}-as

PAGE_SIZE_BYTE_HEX := 1000

MKT_BOARD := maaxboard
MKT_CONFIG := debug

MKT_SDK_PATH := ${DEP_MKT_PATH}/out/microkit-sdk-1.4.1
MKT_PATH_FILE := ${MKT_SDK_PATH}/bin/microkit
MKT_RTM_PATH_FILE := ${MKT_SDK_PATH}/board/${MKT_BOARD}/${MKT_CONFIG}

CC_OPS := \
    -c \
    -mcpu=${CPU} \
    -ffreestanding \
    -nostdlib \
    -mstrict-align \
    -g3 \
    -O3 \
    -Wall \
    -Wno-unused-function \
    -Werror \
    -I ${TMP_PATH} \
    -I ${MKT_RTM_PATH_FILE}/include \

LD_OPS := \
    -L $(MKT_RTM_PATH_FILE)/lib \
    -lmicrokit \
    -Tmicrokit.ld \

.PHONY: get
get: dep-get

.PHONY: dep-get
dep-get:
	make -C ${DEP_MKT_PATH} get

.PHONY: all
all: dep-all ${OUT_PATH}/program.img

.PHONY: dep-all
dep-all:
	make -C ${DEP_MKT_PATH} all

${TMP_PATH}:
	mkdir ${TMP_PATH}

${OUT_PATH}:
	mkdir ${OUT_PATH}

.PHONY: align
align: | ${TMP_PATH}
	m4 --define=MAC_PAR_ALIGN_HEX="$(PAGE_SIZE_BYTE_HEX)" \
	   --define=MAC_PAR_INPUT_HEX="$(ENV_BLOCK_ADDR_HEX)" \
	   ${SRC_PATH}/align_sub.bc.in > ${TMP_PATH}/align_block_addr_hex.bc
	m4 --define=MAC_PAR_ALIGN_HEX="$(PAGE_SIZE_BYTE_HEX)" \
	   --define=MAC_PAR_INPUT_HEX="$(ENV_BLOCK_SIZE_BYTE_HEX)" \
	   ${SRC_PATH}/align_add.bc.in > ${TMP_PATH}/align_block_size_byte_hex.bc

.PHONY: offset
offset: align | ${TMP_PATH}
	m4 --define=MAC_PAR_TARGET_HEX=$(ENV_BLOCK_ADDR_HEX) \
	   --define=MAC_PAR_ACTUAL_HEX=$(shell bc --quiet ${TMP_PATH}/align_block_addr_hex.bc) \
	   ${SRC_PATH}/offset.bc.in > ${TMP_PATH}/offset_size_byte_hex.bc

${TMP_PATH}/config.m4: align offset | ${TMP_PATH}
	m4 --define=MAC_PAR_BLOCK_ADDR_HEX="$(ENV_BLOCK_ADDR_HEX)" \
	   --define=MAC_PAR_BLOCK_SIZE_BYTE_HEX="$(ENV_BLOCK_SIZE_BYTE_HEX)" \
	   --define=MAC_PAR_ALIGN_BLOCK_ADDR_HEX="$(shell bc --quiet ${TMP_PATH}/align_block_addr_hex.bc)" \
	   --define=MAC_PAR_ALIGN_BLOCK_SIZE_BYTE_HEX="$(shell bc --quiet ${TMP_PATH}/align_block_size_byte_hex.bc)" \
	   --define=MAC_PAR_OFFSET_SIZE_BYTE_HEX="$(shell bc --quiet ${TMP_PATH}/offset_size_byte_hex.bc)" \
	   ${SRC_PATH}/config.m4.in > ${TMP_PATH}/config.m4

${TMP_PATH}/program.system: ${SRC_PATH}/program.system.in ${TMP_PATH}/config.m4 
	m4 --include ${TMP_PATH} $< > $@

${TMP_PATH}/probe.h: ${SRC_PATH}/probe.h.in ${TMP_PATH}/config.m4 
	m4 --include ${TMP_PATH} $< > $@

${TMP_PATH}/probe.o: ${SRC_PATH}/probe.c ${TMP_PATH}/probe.h 
	${CC} ${CC_OPS} $< -o $@

${TMP_PATH}/probe.elf: ${TMP_PATH}/probe.o
	${LD} $^ ${LD_OPS} -o $@

${OUT_PATH}/program.img: ${MKT_PATH_FILE} ${TMP_PATH}/program.system ${TMP_PATH}/probe.elf | ${OUT_PATH}
	${MKT_PATH_FILE} ${TMP_PATH}/program.system --search-path ${TMP_PATH} --board ${MKT_BOARD} --config ${MKT_CONFIG} --output ${OUT_PATH}/program.img --report ${OUT_PATH}/report.txt

.PHONY: clean
clean:
	make -C ${DEP_MKT_PATH} clean
	rm -rf ${TMP_PATH}
	rm -rf ${OUT_PATH}

################################################################################
# End of file
################################################################################
