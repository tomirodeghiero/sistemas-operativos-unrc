typedef unsigned char uint8;
typedef unsigned int  uint32;
typedef unsigned int  uint;
typedef unsigned long uint64;
typedef unsigned int  size_t;

/****************************************************************************
 * Universal Asynchronous Receiver-Transmitter (UART) device controller     *
 * UART_THR address port: Transmitter Holding Register
 * UART_LSR address port: Line Status Register
 * UART_STATUS_EMPTY: LSR bit 6 Transmitter empty
 ****************************************************************************/
#define UART        0x10000000
#define UART_THR    (uint8*)(UART+0x00)
#define UART_LSR    (uint8*)(UART+0x05)
#define UART_STATUS_EMPTY 0x40
#define SCAUSE_INTERRUPT_MASK (1U << 31)
#define SCAUSE_CODE_MASK      0x7fffffffU
#define SCAUSE_ILLEGAL_INSTRUCTION 2U

// get mmio register address (byte)
#define mmio_reg_b(r) ((volatile unsigned char *)(r))

int console_putc(char ch) {
    // wait for UART transmitter register empty
	while ((*mmio_reg_b(UART_LSR) & UART_STATUS_EMPTY) == 0)
        ;
    // write character to UART THR to start transmission
	return *mmio_reg_b(UART_THR) = ch;
}

// write string to console
void console_puts(const char *s) {
	while (*s)
        console_putc(*s++);
}

void console_putu(uint32 value) {
    char buf[11];
    int i = 0;

    if (value == 0) {
        console_putc('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        console_putc(buf[--i]);
    }
}

// the trap handler (called from s_trap defined in start.s)
void trap(uint cause)
{
    console_puts("trap!!!\n");

    // Detect trap type and cause from scause:
    // bit 31: 1=interrupt, 0=exception
    // bits 30..0: exception/interrupt code
    uint is_interrupt = (cause & SCAUSE_INTERRUPT_MASK) != 0;
    uint code = cause & SCAUSE_CODE_MASK;

    if (is_interrupt) {
        console_puts("trap type: interrupt\n");
        console_puts("interrupt code: ");
        console_putu(code);
        console_putc('\n');
    } else {
        console_puts("trap type: exception\n");
        console_puts("exception code: ");
        console_putu(code);
        console_putc('\n');

        if (code == SCAUSE_ILLEGAL_INSTRUCTION) {
            console_puts("exception detail: Illegal Instruction\n");
        }
    }

    // In this step we stop execution after reporting the trap.
    for (;;) {
    }

}

const char *hello = "Hello World from supervisor mode!\n";

// main kernel function. Here we have in supervisor (kernel) mode.
void kernel_main(void) {
    asm volatile("csrr a0, mhartid");
    console_puts(hello);
    for (;;) {
    }
}
