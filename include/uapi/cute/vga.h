#ifndef __VGA_H
#define __VGA_H

#include <stdint.h>

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	unsigned char *pixmap;
} vidmem_plane_t;

#endif
