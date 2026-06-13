AS=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy
QEMU=qemu-system-i386
BUILD=build
OBJDIR=$(BUILD)/obj

CFLAGS=-m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -Wall -Wextra -nostdlib -Iinclude
LDFLAGS=-m elf_i386 -T linker.ld

KERNEL_C=main.c print.c kernel_runtime.c idt.c pic.c pit.c process.c shm.c sync.c
KERNEL_ASM=entry.asm gdt.asm idt_load.asm interrupt.asm switch.asm
BANK_DEPOSIT_OBJ=$(OBJDIR)/bank_deposit.o
BANK_DEPOSIT_ELF=$(BUILD)/bank_deposit.elf
BANK_DEPOSIT_BLOB=$(OBJDIR)/bank_deposit_blob.o
BANK_WITHDRAW_OBJ=$(OBJDIR)/bank_withdraw.o
BANK_WITHDRAW_ELF=$(BUILD)/bank_withdraw.elf
BANK_WITHDRAW_BLOB=$(OBJDIR)/bank_withdraw_blob.o

C_OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_C))
ASM_OBJS=$(patsubst %.asm,$(OBJDIR)/%.o,$(KERNEL_ASM))
KERNEL_OBJS=$(C_OBJS) $(ASM_OBJS) $(BANK_DEPOSIT_BLOB) $(BANK_WITHDRAW_BLOB)

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

$(BANK_DEPOSIT_OBJ): bank_deposit.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(BANK_DEPOSIT_ELF): $(BANK_DEPOSIT_OBJ) bank_deposit.ld | $(BUILD)
	$(LD) -m elf_i386 -T bank_deposit.ld -o $@ $(BANK_DEPOSIT_OBJ)

$(BANK_DEPOSIT_BLOB): $(BANK_DEPOSIT_ELF) | $(OBJDIR)
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $< $@

$(BANK_WITHDRAW_OBJ): bank_withdraw.asm | $(OBJDIR)
	$(AS) -f elf32 -o $@ $<

$(BANK_WITHDRAW_ELF): $(BANK_WITHDRAW_OBJ) bank_withdraw.ld | $(BUILD)
	$(LD) -m elf_i386 -T bank_withdraw.ld -o $@ $(BANK_WITHDRAW_OBJ)

$(BANK_WITHDRAW_BLOB): $(BANK_WITHDRAW_ELF) | $(OBJDIR)
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
