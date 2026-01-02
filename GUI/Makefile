# Use the x86_64-elf cross-compiler as you are on a Mac
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

all: run

kernel.bin: kernel_entry.o kernel.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@

kernel.o: kernel.c
	$(CC) -m32 -ffreestanding -c $< -o $@

mbr.bin: mbr.asm
	nasm $< -f bin -o $@

# CREATE A PADDED DISK IMAGE
os-image.bin: mbr.bin kernel.bin
	# Start with a blank 1MB disk
	dd if=/dev/zero of=os-image.bin bs=512 count=2048
	# Put MBR at Sector 0
	dd if=mbr.bin of=os-image.bin conv=notrunc
	# Put Kernel at Sector 1 (LBA 1)
	dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc

run: os-image.bin
	# -drive tells QEMU it is a Hard Disk on the IDE bus
	qemu-system-i386 -drive format=raw,file=os-image.bin,index=0,if=ide

clean:
	$(RM) *.bin *.o