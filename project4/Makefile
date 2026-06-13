AS=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy
QEMU=qemu-system-i386
BUILD=build
OBJDIR=$(BUILD)/obj

CFLAGS=-m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -Wall -Wextra -nostdlib -Iinclude
LDFLAGS=-m elf_i386 -T linker.ld

KERNEL_C=main.c print.c kernel_runtime.c idt.c pic.c pit.c process.c
KERNEL_ASM=entry.asm gdt.asm idt_load.asm interrupt.asm switch.asm
USER_PROGRAM_OBJ=$(OBJDIR)/user_program.o
USER_PROGRAM_ELF=$(BUILD)/user_program.elf
USER_PROGRAM_BLOB=$(OBJDIR)/user_program_blob.o
USER_PROGRAM2_OBJ=$(OBJDIR)/user_program2.o
USER_PROGRAM2_ELF=$(BUILD)/user_program2.elf
USER_PROGRAM2_BLOB=$(OBJDIR)/user_program2_blob.o

C_OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_C))
ASM_OBJS=$(patsubst %.asm,$(OBJDIR)/%.o,$(KERNEL_ASM))
KERNEL_OBJS=$(C_OBJS) $(ASM_OBJS) $(USER_PROGRAM_BLOB) $(USER_PROGRAM2_BLOB)

BOOT_BIN=$(BUILD)/boot.bin
KERNEL_ELF=$(BUILD)/kernel.elf
KERNEL_BIN=$(BUILD)/kernel.bin
OS_IMAGE=$(BUILD)/os-image.bin

all: $(OS_IMAGE)

$(BUILD):
	mkdir -p $(BUILD)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BOOT_BIN): boot.asm | $(BUILD)
	$(AS) -f bin -o $@ $<

$(USER_PROGRAM_OBJ): user_program.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(USER_PROGRAM_ELF): $(USER_PROGRAM_OBJ) user_program.ld | $(BUILD)
	$(LD) -m elf_i386 -T user_program.ld -o $@ $(USER_PROGRAM_OBJ)

$(USER_PROGRAM_BLOB): $(USER_PROGRAM_ELF) | $(OBJDIR)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $< $@

$(USER_PROGRAM2_OBJ): user_program2.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(USER_PROGRAM2_ELF): $(USER_PROGRAM2_OBJ) user_program2.ld | $(BUILD)
	$(LD) -m elf_i386 -T user_program2.ld -o $@ $(USER_PROGRAM2_OBJ)

$(USER_PROGRAM2_BLOB): $(USER_PROGRAM2_ELF) | $(OBJDIR)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $< $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(OBJDIR)/%.o: %.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(KERNEL_ELF): $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$@ bs=512 count=33 2>/dev/null
	dd if=$(BOOT_BIN) of=$@ bs=512 conv=notrunc 2>/dev/null
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

run-debug: $(OS_IMAGE)
	$(QEMU) -no-reboot -no-shutdown -d int -drive format=raw,file=$(OS_IMAGE)

clean:
	rm -rf $(BUILD)

.PHONY: all run run-debug clean
