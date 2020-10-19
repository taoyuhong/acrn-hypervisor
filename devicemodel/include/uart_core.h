/*-
 * Copyright (c) 2013 Neel Natu <neel@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _UART_CORE_H_
#define	_UART_CORE_H_


#define	UART_IO_BAR_SIZE	8

enum uart_be_type {
        UART_BE_INVALID = 0,
        UART_BE_STDIO,
        UART_BE_TTY,
        UART_BE_SOCK
};

struct fifo {
        uint8_t *buf;
        int     rindex;         /* index to read from */
        int     windex;         /* index to write to */
        int     num;            /* number of characters in the fifo */
        int     size;           /* size of the fifo */
};

struct uart_backend {
        /*
         * UART_BE_STDIO: fd = STDIN_FILENO
         * UART_BE_TTY: fd = open(tty)
         * UART_BE_SOCK: fd = file descriptor of listen socket
         */
        int                     fd;
        struct mevent           *evp;

        /*
         * UART_BE_STDIO: fd2 = STDOUT_FILENO
         * UART_BE_TTY: fd2 = fd = open(tty)
         * UART_BE_SOCK: fd2 = file descriptor of connected socket
         */
        int                     fd2;
        struct mevent           *evp2;

        enum uart_be_type       be_type;
        bool                    opened;
};

typedef void (*uart_intr_func_t)(void *arg);

struct uart_vdev {
	pthread_mutex_t mtx;	/* protects all elements */
	uint8_t data;		/* Data register (R/W) */
	uint8_t ier;		/* Interrupt enable register (R/W) */
	uint8_t lcr;		/* Line control register (R/W) */
	uint8_t mcr;		/* Modem control register (R/W) */
	uint8_t lsr;		/* Line status register (R/W) */
	uint8_t msr;		/* Modem status register (R/W) */
	uint8_t fcr;		/* FIFO control register (W) */
	uint8_t scr;		/* Scratch register (R/W) */

	uint8_t dll;		/* Baudrate divisor latch LSB */
	uint8_t dlh;		/* Baudrate divisor latch MSB */

	struct fifo rxfifo;
	struct uart_backend be;

	bool thre_int_pending;	/* THRE interrupt pending */

	void *arg;
	int rxfifo_size;
	uart_intr_func_t intr_assert;
	uart_intr_func_t intr_deassert;
	bool is_hv_land;
};

int	uart_legacy_alloc(int unit, int *ioaddr, int *irq);
void	uart_legacy_dealloc(int which);
uint8_t	uart_read(struct uart_vdev *uart, int offset);
void	uart_write(struct uart_vdev *uart, int offset, uint8_t value);
struct	uart_vdev*
	uart_set_backend(uart_intr_func_t intr_assert, uart_intr_func_t intr_deassert,
		void *arg, const char *opts);
void	uart_release_backend(struct uart_vdev *uart, const char *opts);
#endif
