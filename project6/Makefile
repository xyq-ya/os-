AS=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy
QEMU=qemu-system-i386
BUILD=build
OBJDIR=$(BUILD)/obj

CFLAGS=-m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -Wall -Wextra -nostdlib -Iinclude
LDFLAGS=-m elf_i386 -T linker.ld

KERNEL_C=main.c print.c kernel_runtime.c idt.c pic.c pit.c process.c shm.c sync.c disk.c fat32.c
KERNEL_ASM=entry.asm gdt.asm idt_load.asm interrupt.asm switch.asm
BANK_DEPOSIT_OBJ=$(OBJDIR)/bank_deposit.o
BANK_DEPOSIT_ELF=$(BUILD)/bank_deposit.elf
BANK_WITHDRAW_OBJ=$(OBJDIR)/bank_withdraw.o
BANK_WITHDRAW_ELF=$(BUILD)/bank_withdraw.elf

C_OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_C))
ASM_OBJS=$(patsubst %.asm,$(OBJDIR)/%.o,$(KERNEL_ASM))
KERNEL_OBJS=$(C_OBJS) $(ASM_OBJS)

BOOT_BIN=$(BUILD)/boot.bin
KERNEL_ELF=$(BUILD)/kernel.elf
KERNEL_BIN=$(BUILD)/kernel.bin
OS_IMAGE=$(BUILD)/os-image.bin
FAT32_IMAGE=$(BUILD)/fat32.img

all: $(OS_IMAGE) $(FAT32_IMAGE)

$(BUILD):
	mkdir -p $(BUILD)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(KERNEL_ELF): $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(BOOT_BIN): boot.asm $(KERNEL_BIN) | $(BUILD)
	@ksectors=$$(stat -c%s $(KERNEL_BIN)); \
	ksectors=$$(( ($$ksectors + 511) / 512 )); \
	$(AS) -f bin -o $@ $< -DKERNEL_SECTORS=$$ksectors

$(BANK_DEPOSIT_OBJ): bank_deposit.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(BANK_DEPOSIT_ELF): $(BANK_DEPOSIT_OBJ) bank_deposit.ld | $(BUILD)
	$(LD) -m elf_i386 -T bank_deposit.ld -o $@ $(BANK_DEPOSIT_OBJ)

$(BANK_WITHDRAW_OBJ): bank_withdraw.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(BANK_WITHDRAW_ELF): $(BANK_WITHDRAW_OBJ) bank_withdraw.ld | $(BUILD)
	$(LD) -m elf_i386 -T bank_withdraw.ld -o $@ $(BANK_WITHDRAW_OBJ)

$(FAT32_IMAGE): $(BANK_DEPOSIT_ELF) $(BANK_WITHDRAW_ELF) tools/mkfat32.py
	python3 tools/mkfat32.py

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(OBJDIR)/%.o: %.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@ksectors=$$(stat -c%s $(KERNEL_BIN)); \
	ksectors=$$(( ($$ksectors + 511) / 512 )); \
	dd if=/dev/zero of=$@ bs=512 count=$$((ksectors + 1)) 2>/dev/null; \
	dd if=$(BOOT_BIN) of=$@ bs=512 conv=notrunc 2>/dev/null; \
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null

run: $(OS_IMAGE) $(FAT32_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE),index=0,media=disk \
	        -drive format=raw,file=$(FAT32_IMAGE),index=1,media=disk

run-debug: $(OS_IMAGE) $(FAT32_IMAGE)
	$(QEMU) -no-reboot -no-shutdown -d int \
	        -drive format=raw,file=$(OS_IMAGE),index=0,media=disk \
	        -drive format=raw,file=$(FAT32_IMAGE),index=1,media=disk

clean:
	rm -rf $(BUILD)

.PHONY: all run run-debug clean
