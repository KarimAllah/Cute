#include <malloc.h>
#include <cute/vga.h>

uint64_t vga_size = (0xAFFFF - 0xA0000);
uint64_t vga_start = 0x10000000;

void fill_screen(uint8_t color);
void render_plane(vidmem_plane_t *plane);

#define VGA_WIDTH (0x28 * 2 * 4)

/* busy wait */
#define busy_wait(end) \
{ \
	uint64_t tmp = 1; \
	while(tmp++ < end) \
		asm(""); \
}

void _start() {
	vm_map(0x10000000, (0xAFFFF - 0xA0000), 0xA0000);

	busy_wait(0x5FFFFFF);
	uint8_t color = 0;
	do {
		fill_screen(color);
		busy_wait(0xFFFFF);
		color = (color++) % 255;
	} while(1);
}

void fill_screen(uint8_t color)
{
	uint64_t vga_size = (0xAFFFF - 0xA0000);
	uint64_t vga_start = 0x10000000;
	uint64_t cur_addr;
	for(cur_addr = vga_start; cur_addr < (vga_start + vga_size);cur_addr+=1)
		*((char *)cur_addr) = color;
}

void render_plane(vidmem_plane_t *plane)
{
	uint64_t vga_size = (0xAFFFF - 0xA0000);
	uint64_t vga_start = 0x10000000;

	uint64_t index = 0;
	uint64_t cur_width;
	uint64_t cur_height;
	uint64_t cur_addr;
	uint64_t start_left = vga_start + (plane->x + (plane->y * VGA_WIDTH));
	for (cur_height = 0; cur_height < plane->height; cur_height++)
	{
		cur_addr = start_left;
		for(cur_width = 0; cur_width < plane->width; cur_width++)
		{
			if(cur_addr > (vga_start + vga_size))
				goto exit_loop;
			*((char *)cur_addr++) = plane->pixmap[index++];
		}
		start_left += VGA_WIDTH;
	}
exit_loop:
	return;
}
