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

// the trap handler (called from s_trap defined in start.s)
void trap(uint cause)
{
    console_puts("trap!!!\n");

    // to do: Detect if its an interrupt or exception
    // see https://docs.riscv.org/reference/isa/priv/supervisor.html#scause
    // HINT: It should be an Illegal instruction exception


}

const char *hello = "Hello World from supervisor mode!\n";

// main kernel function. Here we have in supervisor (kernel) mode.
void kernel_main(void) {
    asm volatile("csrr a0, mhartid");
    console_puts(hello);
    for (;;) {
    }
}
