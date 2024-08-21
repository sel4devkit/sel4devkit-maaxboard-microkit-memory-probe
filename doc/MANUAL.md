# Introduction

This Package forms part of the seL4 Developer Kit. It provides a fully
coordinated build of a small program that reads bytes from a specified memory
location, for a specific size, and presents the contents as a base64 encoded
string.

Its purpose is to provide a simple mechinsim to inspect and confirm basic
properties of the MaaXBoard.

# Usage

Must be invoked inside the following:
* sel4devkit-maaxboard-camkes-docker-dev-env

Show usage instructions:
```
make
```

Build program, setting `ENV_BLOCK_ADDR_HEX` to select the starting address,
and `ENV_BLOCK_SIZE_BYTE_HEX` to select the size:
Build program:
```
make get
make all ENV_BLOCK_ADDR_HEX=123ABC ENV_BLOCK_SIZE_BYTE_HEX=123ABC
```

# Example

The MaaXBoard holds the USB-1 Memory Map from 0x38100000, containing various
items of structured information, including a set of capability descriptions.
These capability descriptions include a four byte Name String, "USB ", one of
which resides at address 0x38100894. 

The program may be built to inspect these four bytes:
```
make get
make all ENV_BLOCK_ADDR_HEX=38100894 ENV_BLOCK_SIZE_BYTE_HEX=4
```

On execution, the following is output:
```
u-boot=> go 0x50000000
## Starting application at 0x50000000 ...
LDR|INFO: altloader for seL4 starting
LDR|INFO: Flags:                0x0000000000000001
             seL4 configured as hypervisor
LDR|INFO: Kernel:      entry:   0x0000008040000000
LDR|INFO: Root server: physmem: 0x000000004024f000 -- 0x0000000040257000
LDR|INFO:              virtmem: 0x000000008a000000 -- 0x000000008a008000
LDR|INFO:              entry  : 0x000000008a000000
LDR|INFO: region: 0x00000000   addr: 0x0000000040000000   size: 0x000000000024c000   offset: 0x0000000000000000   type: 0x000001
LDR|INFO: region: 0x00000001   addr: 0x000000004024f000   size: 0x0000000000007110   offset: 0x000000000024c000   type: 0x000001
LDR|INFO: region: 0x00000002   addr: 0x000000004024c000   size: 0x0000000000000a38   offset: 0x0000000000253110   type: 0x000001
LDR|INFO: region: 0x00000003   addr: 0x000000004024d000   size: 0x00000000000008d4   offset: 0x0000000000253b48   type: 0x000001
LDR|INFO: region: 0x00000004   addr: 0x000000004024e000   size: 0x0000000000000090   offset: 0x000000000025441c   type: 0x000001
LDR|INFO: copying region 0x00000000
LDR|INFO: copying region 0x00000001
LDR|INFO: copying region 0x00000002
LDR|INFO: copying region 0x00000003
LDR|INFO: copying region 0x00000004
LDR|INFO: CurrentEL=EL2
LDR|INFO: Resetting CNTVOFF
LDR|INFO: enabling MMU
LDR|INFO: jumping to kernel
Warning:  gpt_cntfrq 8333333, expected 8000000
Bootstrapping kernel
available phys memory regions: 1
  [40000000..c0000000]
reserved virt address space regions: 3
  [8040000000..804024c000]
  [804024c000..804024f000]
  [804024f000..8040257000]
Booting all finished, dropped to user space
MON|INFO: Microkit Bootstrap
MON|INFO: bootinfo untyped list matches expected list
MON|INFO: Number of bootstrap invocations: 0x00000009
MON|INFO: Number of system invocations:    0x00000028
MON|INFO: completed bootstrap invocations
MON|INFO: completed system invocations
BLOCK_ADDR_HEX:0x0000000038100894
BLOCK_SIZE_BYTE_HEX:0x0000000000000004
BASE64:BEGIN
VVNCIA==
BASE64:END
```

The text between `BASE64:BEGIN` and `BASE64:END` is base64 encoded. It may be
decoded to view its contents:
```
echo "VVNCIA==" | base64 -d | xxd
```

Where the expected output is as follows:
```
00000000: 5553 4220                                USB
```

# Maintenance

Presents detailed technical aspects relevant to understanding and maintaining
this Package.

## Base64

The use of Base64 permits the effective transfer of arbitrary binary data over
a simple ASCII only interface.

The generated Base64 includes a new line at each 81st column. This simplifies
the cut-and-paste of the generated Base64 from a terminal program. While new
line characters are technically not part of the Base64 standard, their usage
is normal, and it is conventional for a Base64 encoder and decoder to silently
ignore these.

## Page Size

Within Microkit, Physical Addresses, Virtual Addresses, and Sizes all need to
be aligned to a Page Size. We chose to exclusively work with the smaller
supported page size (4KiB, or 0x1000 bytes). The build process utilises both
bc (arbitrary precision calculator) and m4 (macro processor) to calculate
aligned memory regions and amend the program code and configuration
accordingly.
