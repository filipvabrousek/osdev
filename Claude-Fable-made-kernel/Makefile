# H7 OS -- barebones kernel with a draggable-window GUI.
# Toolchain: nasm + i686-elf-gcc cross compiler + qemu-system-i386.

AS      = nasm
CC      = i686-elf-gcc
LD      = i686-elf-ld
OBJCOPY = i686-elf-objcopy
QEMU    = qemu-system-i386

CFLAGS = -std=gnu11 -O2 -Wall -Wextra \
         -ffreestanding -fno-builtin -fno-stack-protector -fno-pic \
         -fno-asynchronous-unwind-tables \
         -mno-sse -mno-mmx -msoft-float

OBJS = build/entry.o build/main.o build/idt.o build/ps2.o build/gfx.o build/wm.o

# image layout (512-byte sectors): stage1=1, stage2=8, kernel=256
IMG_SECTORS = 265

all: build/os.img

build:
	mkdir -p build

build/stage1.bin: boot/stage1.asm | build
	$(AS) -f bin -o $@ $<

build/stage2.bin: boot/stage2.asm | build
	$(AS) -f bin -o $@ $<

build/entry.o: kernel/entry.asm | build
	$(AS) -f elf32 -o $@ $<

build/%.o: kernel/%.c kernel/kernel.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/kernel.elf: linker.ld $(OBJS)
	$(LD) -T linker.ld -o $@ $(OBJS)

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@test $$(stat -f%z $@) -le 131072 || { echo "kernel.bin > 128 KiB: bump KERNEL_SECTORS"; exit 1; }

build/os.img: build/stage1.bin build/stage2.bin build/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=$(IMG_SECTORS) 2>/dev/null
	dd if=build/stage1.bin of=$@ conv=notrunc 2>/dev/null
	dd if=build/stage2.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=build/kernel.bin of=$@ bs=512 seek=9 conv=notrunc 2>/dev/null
	@echo "build/os.img ready ($$(stat -f%z $@) bytes)"

run: build/os.img
	$(QEMU) -drive format=raw,file=build/os.img -vga std -m 64

# headless smoke test: boots the image, drags a window via QMP,
# saves screenshots to build/shot*.png
test: build/os.img
	python3 tools/qemu_test.py

clean:
	rm -rf build

.PHONY: all run test clean
