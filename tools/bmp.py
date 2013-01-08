#!/usr/bin/env python
import os
import sys
import Image

def convert(filename):
	img = Image.open(filename)
	width, height = img.size
	pixels = []
	for y in range(height):
		for x in range(width):
			r, g, b = img.getpixel((x, y))
			pixels.append(int((r + g + b) / 3))
			
	return (pixels, width, height)

TEMPLATE = """static unsigned char %s[] = {
%s
};
static uint32_t logo_width = %s;
static uint32_t logo_height = %s;
"""

if __name__ == "__main__":
	source_image = sys.argv[1]
	raw_pixels, width, height = convert(source_image)
	out_str = hex(raw_pixels[0])
	index = 1
	for pixel in raw_pixels[1:]:
		if not (index % 10):
			out_str += os.linesep 
		out_str += ", %s" % hex(pixel) 
		index += 1

	print TEMPLATE % ("logo_bitmap", out_str, width, height)