#ifndef _VGA_H
#define _VGA_H

#include <kernel.h>

/*
 * VGA Colors
 *
 * Copyright (C) 2010 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 *
 * Reference: 'System BIOS for IBM PCs, compatibles, and EISA computers',
 * second edition, Phoenix press
 */

typedef enum {
	VIDMEM_MODE3 = 0,
	VIDMEM_MODE12 = 1,
	VIDMEM_MODE13 = 2,
	VIDMEM_MODEX = 3
} vidmem_vga_mode_t;

/*
 * VGA color attribute is an 8 bit value, low 4 bits sets
 * foreground color, while the high 4 sets the background.
 */
#define VGA_COLOR(bg, fg)	(((bg) << 4) | (fg))

#define VGA_BLACK		0x0
#define VGA_BLUE		0x1
#define VGA_GREEN		0x2
#define VGA_CYAN		0x3
#define VGA_RED			0x4
#define VGA_MAGNETA		0x5
#define VGA_BROWN		0x6
#define VGA_LIGHT_GRAY		0x7
#define VGA_GRAY		0x8
#define VGA_LIGHT_BLUE		0x9
#define VGA_LIGHT_GREEN		0xa
#define VGA_LIGHT_CYAN		0xb
#define VGA_LIGHT_RED		0xc
#define VGA_LIGHT_MAGNETA	0xd
#define VGA_YELLOW		0xe
#define VGA_WHITE		0xf

/* Max color value, 4 bytes */
#define VGA_COLOR_MAX		0xf

typedef enum {
	NO_REGISTER = 0,
	GRAPHIC_CTRL = 1,
	SEQUENCER = 2,
	ATTRIBUTE_CTRL = 3,
	CRT_CTRL = 4,
	DAC_REGISTERS = 5,
	EXTERNAL_REGISTERS = 6
} register_set_t;

typedef struct {
	register_set_t target;
	uint8_t index;
	uint8_t value;
} vidmem_register_t;

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} vidmem_dac_color_t;

#define DAC_COLORS_NB 256
typedef struct {
	vidmem_dac_color_t colors[DAC_COLORS_NB];
} vidmem_dac_palette_t;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	unsigned char *pixmap;
} vidmem_plane_t;

typedef struct {
	uint16_t width;
	uint16_t height;
	uint8_t bpp; // bytes per pixel

} vidmem_layout_t;

extern vidmem_plane_t vga_logo;

void __no_return vga_worker(void);
uint8_t read_vga_register(register_set_t target, uint8_t index);
void write_vga_register(register_set_t target, uint8_t index, uint8_t value);
void load_dac_palette(vidmem_dac_palette_t *palette);
void load_mode(vidmem_vga_mode_t mode);
void clear_screen(uint8_t color);
void load_palette(void);
void set_mode13(void);
void vga_init(vidmem_vga_mode_t mode);
void render_plane(vidmem_plane_t *plane);

#endif /* _VGA_H */
