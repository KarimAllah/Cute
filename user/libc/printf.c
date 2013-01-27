/*
 * printf()-like methods: vsnprintf(), etc
 *
 * Copyright (C) 2009-2010 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#include <stdarg.h>			/* provided by GCC */
#include <string.h>
#include <syscall.h>

/*
 * We cannot use assert() for below printk() code as
 * the assert code istelf internally calls printk().
 */
#define printk_assert(condition)						\
	do {									\
		if ((!(condition)))					\
			printk_panic("!(" #condition ")");			\
	} while (0);


static void printk_panic(const char *str)
{
	while(1){}
}

#define PRINTK_MAX_RADIX	16

/*
 * Convert given unsigned long integer (@num) to ascii using
 * desired radix. Return the number of ascii chars printed.
 * @size: output buffer size
 */
static int ultoa(unsigned long num, char *buf, int size, unsigned radix)
{
	int ret, digits;
	char digit[PRINTK_MAX_RADIX + 1] = "0123456789abcdef";

	printk_assert(radix > 2 && radix <= PRINTK_MAX_RADIX);

	digits = 0;
	if (num == 0)
		digits++;

	for (typeof(num) c = num; c != 0; c /= radix)
                digits++;

	ret = digits;

	printk_assert(digits > 0);
	printk_assert(digits <= size);
	for (; digits != 0; digits--) {
		buf[digits - 1] = digit[num % radix];
		num = num / radix;
	}

	return ret;
}

/*
 * Convert given signed long integer (@num) to ascii using
 * desired radix. Return the number of ascii chars printed.
 * @size: output buffer size
 */
static int ltoa(signed long num, char *buf, int size, int radix)
{
	printk_assert(radix > 2 && radix <= PRINTK_MAX_RADIX);

	if (num < 0) {
		/* Make room for the '-' */
		printk_assert(size >= 2);

		num *= -1;
		buf[0] = '-';

		return ultoa(num, buf+1, size-1, radix) + 1;
	}

	return ultoa(num, buf, size, radix);
}

/*
 * Definitions for parsing printk arguments. Each argument
 * is described by its descriptor (argdesc) structure.
 */
enum printf_arglen {
	INT = 0,
	LONG,
};
enum printf_argtype {
	NONE = 0,
	SIGNED,
	UNSIGNED,
	STRING,
	CHAR,
	PERCENT,
};
struct printf_argdesc {
	int radix;
	enum printf_arglen len;
	enum printf_argtype type;
};

/*
 * Parse given printf argument expression (@fmt) and save
 * the results to argument descriptor @desc.
 *
 * Input is in in the form: %ld, %d, %x, %lx, etc.
 *
 * Return @fmt after bypassing the '%' expression.
 * FIXME: Better only return printf-expression # of chars.
 */
static const char *parse_arg(const char *fmt, struct printf_argdesc *desc)
{
	int complete;

	printk_assert(*fmt == '%');

	complete = 0;
	desc->len = INT;
	desc->type = NONE;
	desc->radix = 10;

	while (*++fmt) {
		switch (*fmt) {
		case 'l':
			desc->len = LONG;
			break;
		case 'd':
			desc->type = SIGNED, complete = 1;
			goto out;
		case 'u':
			desc->type = UNSIGNED, complete = 1;
			goto out;
		case 'x':
			desc->type = UNSIGNED, desc->radix = 16, complete = 1;
			goto out;
		case 's':
			desc->type = STRING, complete = 1;
			goto out;
		case 'c':
			desc->type = CHAR, complete = 1;
			goto out;
		case '%':
			desc->type = PERCENT, complete = 1;
			goto out;
		default:
			/* Unknown mark: complete by definition */
			desc->type = NONE;
			complete = 1;
			goto out;
		}
	}

out:
	if (complete != 1)
		printk_panic("Unknown/incomplete expression");

	/* Bypass last expression char */
	if (*fmt != 0)
		fmt++;

	return fmt;
}

/*
 * Print to @buf the printk argument stored in the untyped
 * @va_list with the the help of type info from the argument
 * descriptor @desc. @size: output buffer size
 */
static int print_arg(char *buf, int size, struct printf_argdesc *desc,
		     va_list args)
{
	long num;
	unsigned long unum;
	const char *str;
	unsigned char ch;
	int len;

	len = 0;
	printk_assert(size > 0);

	switch (desc->type) {
	case SIGNED:
		if (desc->len == LONG)
			num = va_arg(args, long);
		else
			num = va_arg(args, int);
		len = ltoa(num, buf, size, desc->radix);
		break;
	case UNSIGNED:
		if (desc->len == LONG)
			unum = va_arg(args, unsigned long);
		else
			unum = va_arg(args, unsigned int);
		len = ultoa(unum, buf, size, desc->radix);
		break;
	case STRING:
		str = va_arg(args, char *);
		if (!str)
			str = "<*NULL*>";
		len = strlen(str);
		len = min(size, len);
		strncpy(buf, str, len);
		break;
	case CHAR:
		ch = (unsigned char)va_arg(args, int);
		*buf++ = ch;
		len = 1;
		break;
	case PERCENT:
		*buf++ = '%';
		len = 1;
		break;
	default:
		break;
		/* No-op */
	}

	return len;
}

/*
 * Formt given printf-like string (@fmt) and store the result
 * within at most @size bytes. This version does *NOT* append
 * a NULL to output buffer @buf; it's for internal use only.
 */
int vsnprintf(char *buf, int size, const char *fmt, va_list args)
{
	struct printf_argdesc desc = { 0 };
	char *str;
	int len;

	if (size < 1)
		return 0;

	str = buf;
	while (*fmt) {
		while (*fmt != 0 && *fmt != '%' && size != 0) {
			*str++ = *fmt++;
			--size;
		}

		/* Mission complete */
		if (*fmt == 0 || size == 0)
			break;

		printk_assert(*fmt == '%');
		fmt = parse_arg(fmt, &desc);

		len = print_arg(str, size, &desc, args);
		str += len;
		size -= len;
	}

	printk_assert(str >= buf);
	return str - buf;
}

static char kbuf[1024];
void printf(const char *fmt, ...)
{

	kbuf[0] = 0;
	va_list args;
	int n;

	/* NOTE! This will deadlock if the code enclosed
	 * by this lock triggered exceptions: the default
	 * exception handlers already call printk() */
	va_start(args, fmt);
	n = vsnprintf(kbuf, sizeof(kbuf), fmt, args);
	va_end(args);

	syscall(SYSCALL_PRINTF, kbuf);
}
