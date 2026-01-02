CC = x86_64-elf-gcc
LD = x86_64-elf-ld
RM = rm -f

all: run

kernel.bin: kernel_entry.o kernel.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@

kernel.o: kernel.c
	$(CC) -m32 -ffreestanding -c $< -o $@

mbr.bin: mbr.asm
	nasm $< -f bin -o $@

# CREATE AND UPDATE THE DISK IMAGE
os-image.bin: mbr.bin kernel.bin
	# 1. Create a 1MB file ONLY if it doesn't exist
	test -f os-image.bin || dd if=/dev/zero of=os-image.bin bs=512 count=2048
	# 2. Update MBR (Sector 0) without truncating the file
	dd if=mbr.bin of=os-image.bin conv=notrunc
	# 3. Update Kernel (Starting at Sector 1) without truncating
	dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc

run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin,index=0,if=ide,media=disk

clean:
	$(RM) *.bin *.o
# Note: 'clean' does NOT remove os-image.bin so your data stays safe.
# Run 'rm os-image.bin' manually if you want a total reset.