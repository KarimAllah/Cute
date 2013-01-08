#include <io.h>
#include <vm.h>
#include <vga.h>
#include <sched.h>
#include <kernel.h>
#include <kmalloc.h>

#include "logo.h"

vidmem_plane_t vga_logo;
vm_memory_range_t vga_map;

#define DAC_WRITE_COLOR_IDX 0x3C8
#define DAC_READ_COLOR_IDX 0x3C7
#define DAC_RW_COLOR_DATA 0x3C9

void load_palette(void)
{
	uint16_t cur_color_idx = 0;

	outb(0x0, DAC_WRITE_COLOR_IDX);
	for(;cur_color_idx<=255;cur_color_idx++)
	{
		outb(cur_color_idx, DAC_RW_COLOR_DATA);
		outb(cur_color_idx, DAC_RW_COLOR_DATA);
		outb(cur_color_idx, DAC_RW_COLOR_DATA);
	}
}

/* Graphics registers */
#define GCTRL_SR 0x0 // Set/Reset register
#define GCTRL_SR_MASK 0x4
#define GCTRL_SR_SHIFT 0

#define GCTRL_ESR 0x1 // Enable Set/Reset register
#define GCTRL_ESR_MASK 0x4
#define GCTRL_ESR_SHIFT 0

#define GCTRL_CC 0x2 // Color compare register
#define GCTRL_CC_MASK 0x4
#define GCTRL_CC_SHIFT 0x4

#define GCTRL_DR 0x3 // Data rotate register
#define GCTRL_DR_LO_MASK 0x18 // Logical Operation
#define GCTRL_DR_LO_SHIFT 3
#define GCTRL_DR_RC_MASK 0x7 // Rotate count
#define GCTRL_DR_RC_SHIT 0

#define GCTRL_RMS 0x4 // Read map select register
#define GCTRL_RMS_MASK 0x3
#define GCTRL_RMS_SHIFT 0x3

#define GCTRL_GM 0x5
#define GCTRL_GM_SHIFT256_MASK 0x40
#define GCTRL_GM_SHIFT256_SHIFT 6
#define GCTRL_GM_SHIFTREG_MASK 0x20 // SHIFT Reg
#define GCTRL_GM_SHIFTREG_SHIFT 5
#define GCTRL_GM_HOSTOE_MASK 0x10 // HOST O/E
#define GCTRL_GM_HOSTOE_SHIFT 4
#define GCTRL_GM_RM_MASK 0x8 // Read mode
#define GCTRL_GM_RM_SHIFT 3
#define GCTRL_GM_WM_MASK 0x3 // Write mode
#define GCTRL_GM_WM_SHIFT 0

#define GCTRL_MG 0x6 // Miscellaneous graphics register
#define GCTRL_MG_MMS_MASK 0xC
#define GCTRL_MG_MMS_SHIFT 2
#define GCTRL_MG_CHAINOE_MASK 0x2
#define GCTRL_MG_CHAINOE_SHIFT 0
#define GCTRL_MG_ALPHA_MASK 0x1
#define GCTRL_MG_ALPHA_SHIFT 0

#define GCTRL_CDC 0x7 // Color don't care register
#define GCTRL_CDC_MASK 0xF
#define GCTRL_CDC_SHIFT 0

#define GCTRL_BM 0x8 // Bit mask register

#define gctrl_read(index) \
	(outb(index, 0x3CE),inb(0x3CF))
#define gctrl_write(index, value) \
{ \
	outb((index), 0x3CE); \
	outb((value), 0x3CF); \
}

/* Sequencer registers */
#define SEQ_RESET 0x0
#define SEQ_RESET_AR_MASK 0x1
#define SEQ_RESET_AR_SHIFT 0
#define SEQ_RESET_SR_MASK 0x2
#define SEQ_RESET_SR_SHIFT 1

#define SEQ_CLOCKING_MODE 0x1
#define SEQ_CLOCKING_MODE_SD_MASK 0x20
#define SEQ_CLOCKING_MODE_SD_SHIFT 5
#define SEQ_CLOCKING_MODE_S4_MASK 0x10
#define SEQ_CLOCKING_MODE_S4_SHIFT 4
#define SEQ_CLOCKING_MODE_DCR_MASK 0x08
#define SEQ_CLOCKING_MODE_DCR_SHIFT 3
#define SEQ_CLOCKING_MODE_SLR_MASK 0x04
#define SEQ_CLOCKING_MODE_SLR_SHIFT 2
#define SEQ_CLOCKING_MODE_98DM_MASK 0x1
#define SEQ_CLOCKING_MODE_98DM_SHIFT 0

#define SEQ_MAP_MASK 0x2
#define SEQ_MAP_MASK_MPWE_MASK 0x0F
#define SEQ_MAP_MASK_MPWE_SHIFT 0

#define SEQ_CHARACTER_MAP_SELECT 0x3

#define SEQ_MEMORY_MODE 0x4
#define SEQ_MEMORY_MODE_CHAIN4_MASK 0x08
#define SEQ_MEMORY_MODE_CHAIN4_SHIFT 3
#define SEQ_MEMORY_MODE_OEDIS_MASK 0x04
#define SEQ_MEMORY_MODE_OEDIS_SHIFT 0x04
#define SEQ_MEMORY_MODE_EMEM_MASK 0x02
#define SEQ_MEMORY_MODE_EMEM_SHIFT 0x02

#define seq_read(index) \
	(outb(index, 0x3C4),inb(0x3C4))
#define seq_write(index, value) \
{ \
	outb(index, 0x3C4); \
	outb(value, 0x3C5); \
}

/* Attribute registers */
#define ATTR_ADDRESS 0x0

#define ATTR_AMC 0x10 // Attribute mode control
#define ATTR_AMC_P54S_MASK 0x80
#define ATTR_AMC_P54S_SHIFT 7
#define ATTR_AMC_8BIT_MASK 0x40
#define ATTR_AMC_8BIT_SHIFT 6
#define ATTR_AMC_PPM_MASK 0x20
#define ATTR_AMC_PPM_SHIFT 5
#define ATTR_AMC_BLINK_MASK 0x08
#define ATTR_AMC_BLINK_SHIFT 3
#define ATTR_AMC_LGE_MASK 0x04
#define ATTR_AMC_LGE_SHIFT 2
#define ATTR_AMC_MONO_MASK 0x02
#define ATTR_AMC_MONO_SHIFT 1
#define ATTR_AMC_ATGE_MASK 0x01
#define ATTR_AMC_ATGE_SHIFT 0x01

#define ATTR_OVERSCAN_COLOR 0x11

#define ATTR_CPE 0x12 // Color plane enable

#define ATTR_HPP 0x13 // Horizontal pixel panning
#define ATTR_HPP_PSC_MASK 0xF // Pixel shift count
#define ATTR_HPP_PSC_SHIFT 0

#define ATTR_COLOR_SELECT 0x14
#define ATTR_COLOR_SELECT_76_MASK 0x06
#define ATTR_COLOR_SELECT_76_SHIFT 2
#define ATTR_COLOR_SELECT_54_MASK 0x02
#define ATTR_COLOR_SELECT_54_SHIFT 0

#define attr_read(index) \
	(inb(0x3DA), \
	outb(index, 0x3C0), \
	inb(0x3C1))
#define attr_write(index, value) \
{ \
	inb(0x3DA); \
	outb(index, 0x3C0); \
	outb(value, 0x3C0); \
}

#define attr_direct_write(value) \
{ \
	inb(0x3DA); \
	outb(value, 0x3C0); \
}

/* DAC registers */
#define DAC_STATE_REGISTER 0x0


/* CRT Controller */
#define CRTC_HT 0x0 // Horizontal total

#define CRTC_EHD 0x1 // End horizontal display

#define CRTC_SHB 0x2 // Start horizontal blanking

#define CRTC_EHB 0x3 // End horizontal blanking
#define CRTC_EHB_MASK 0x1F
#define CRTC_EHB_SHIFT 0
#define CRTC_EHB_EVRA_MASK 0x80
#define CRTC_EHB_EVRA_SHIFT 7
#define CRTC_EHB_DES_MASK 0x60 // Display enable skew
#define CRTC_EHB_DES_SHIFT 5

#define CRTC_SHR 0x4 // Start horizontal retrace

#define CRTC_EHR 0x5 // End horizontal retrace
#define CRTC_EHR_MASK 0x1F
#define CRTC_EHR_SHIFT 0
#define CRTC_EHR_EHB5_MASK 0x80
#define CRTC_EHR_EHB5_SHIFT 7
#define CRTC_EHR_HRS_MASK 0x60 // Horizontal retrace skew

#define CRTC_VT 0x6 // Vertical total register

#define CRTC_OVERFLOW 0x7 // Overflow register

#define CRTC_PRS 0x8 // Preset row scan register

#define CRTC_MSL 0x9 // Maximum scan line register

#define CRTC_VRS 0x10 // Vertical retrace start register

#define CRTC_VRE 0x11 // Vertical retrace end register

#define CRTC_VDE 0x12 // Vertical display end register

#define CRTC_OFFSET 0x13 // Offset register

#define CRTC_UL 0x14 // Underline location register

#define CRTC_SVB 0x15 // Start vertical blank register

#define CRTC_EVB 0x16 // End vertical blank register

#define CRTC_MC 0x17 // Mode control register

#define crtc_read(index) \
	(outb(index, 0x3D4), \
	inb(0x3D5))
#define crtc_write(index, value) \
{ \
	outb(index, 0x3D4); \
	outb(value, 0x3D5); \
}

/* External register */
#define MISC_OUTPUT_REGISTER 0x0
#define MISC_OUTPUT_IOAS_MASK 0x1 // IO address select
#define MISC_OUTPUT_IOAS_SHIFT 0
#define MISC_OUTPUT_RE_MASK 0x2 // Ram enable
#define MISC_OUTPUT_RE_SHIFT 1
#define MISC_OUTPUT_CS_MASK 0x0C // clock select
#define MISC_OUTPUT_CS_SHIFT 2
#define MISC_OUTPUT_OEP_MASK 0x20 // Odd/Even page select
#define MISC_OUTPUT_OEP_SHIFT 5
#define MISC_OUTPUT_HSYNCP_MASK 0x40 // Horizontal sync polarity
#define MISC_OUTPUT_HSYNCP_SHIFT 6
#define MISC_OUTPUT_VSYNCP_MASK 0x80 // Vertical sync polarity
#define MISC_OUTPUT_VSYNCP_SHIFT 7

#define FEATURE_CONTROL_REGISTER 0x1
#define INPUT_STATUS0_REGISTER 0x2
#define INPUT_STATUS1_REGISTER 0x3

#define misc_output_write(value) outb(value, 0x3C2)
#define misc_output_read() inb(0x3CC)

#define feature_control_write(value) outb(value, 0x3DA)
#define feature_control_read() inb(0x3CA)

#define input_status0_read() inb(0x3C2)

#define input_status1_read() inb(0x3DA)

#define load_dac_color(index, red, green, blue) \
{ \
	outb(index, 0x3C8); \
	outb(red, 0x3C9); \
	outb(green, 0x3C9); \
	outb(blue, 0x3C9); \
}

#define read_dac_color(index, red, green, blue) \
{ \
	outb(index, 0x3C7); \
	red = inb(index, 0x3C9); \
	green = inb(index, 0x3C9); \
	blue = inb(index, 0x3C9); \
}

#define dac_state_read() (inb(0x3C7) & 0x1)

static vidmem_register_t vga_modes_registers[4][32] =
{
		[VIDMEM_MODE3] = {
				{ATTRIBUTE_CTRL, ATTR_AMC, 0x0C},
				{ATTRIBUTE_CTRL, ATTR_OVERSCAN_COLOR, 0xFF},
				{ATTRIBUTE_CTRL, ATTR_CPE, 0x0F},
				{ATTRIBUTE_CTRL, ATTR_HPP, 0x08},
				{ATTRIBUTE_CTRL, ATTR_COLOR_SELECT, 0x00},
				{EXTERNAL_REGISTERS, MISC_OUTPUT_REGISTER, 0x67},
				{SEQUENCER, SEQ_CLOCKING_MODE, 0x00},
				{SEQUENCER, SEQ_MAP_MASK, 0x0F},
				{SEQUENCER, SEQ_CHARACTER_MAP_SELECT, 0x00},
				{SEQUENCER, SEQ_MEMORY_MODE, 0x07},
				{GRAPHIC_CTRL, GCTRL_GM, 0x10},
				{GRAPHIC_CTRL, GCTRL_MG, 0x0E},
				{CRT_CTRL, CRTC_VRE, 0x80},
				{CRT_CTRL, CRTC_HT, 0x5F},
				{CRT_CTRL, CRTC_EHD, 0x4F},
				{CRT_CTRL, CRTC_SHB, 0x50},
				{CRT_CTRL, CRTC_EHB, 0x82},
				{CRT_CTRL, CRTC_SHR, 0x55},
				{CRT_CTRL, CRTC_EHR, 0x81},
				{CRT_CTRL, CRTC_VT, 0xBF},
				{CRT_CTRL, CRTC_OVERFLOW, 0x1F},
				{CRT_CTRL, CRTC_PRS, 0x00},
				{CRT_CTRL, CRTC_MSL, 0x4F},
				{CRT_CTRL, CRTC_VRS, 0x9C},
				{CRT_CTRL, CRTC_VRE, 0x8E},
				{CRT_CTRL, CRTC_VDE, 0x8F},
				{CRT_CTRL, CRTC_OFFSET, 0x28},
				{CRT_CTRL, CRTC_UL, 0x1F},
				{CRT_CTRL, CRTC_SVB, 0x96},
				{CRT_CTRL, CRTC_EVB, 0xB9},
				{CRT_CTRL, CRTC_MC, 0xA3},
				{NO_REGISTER, 0, 0}
		},
		[VIDMEM_MODE12] = {
				{ATTRIBUTE_CTRL, ATTR_AMC, 0x01},
				{ATTRIBUTE_CTRL, ATTR_OVERSCAN_COLOR, 0xFF},
				{ATTRIBUTE_CTRL, ATTR_CPE, 0x0F},
				{ATTRIBUTE_CTRL, ATTR_HPP, 0x00},
				{ATTRIBUTE_CTRL, ATTR_COLOR_SELECT, 0x00},
				{EXTERNAL_REGISTERS, MISC_OUTPUT_REGISTER, 0xE3},
				{SEQUENCER, SEQ_CLOCKING_MODE, 0x01},
				{SEQUENCER, SEQ_MAP_MASK, 0x0F},
				{SEQUENCER, SEQ_CHARACTER_MAP_SELECT, 0x00},
				{SEQUENCER, SEQ_MEMORY_MODE, 0x02},
				{GRAPHIC_CTRL, GCTRL_GM, 0x00},
				{GRAPHIC_CTRL, GCTRL_MG, 0x05},
				{CRT_CTRL, CRTC_VRE, 0x80},
				{CRT_CTRL, CRTC_HT, 0x5F},
				{CRT_CTRL, CRTC_EHD, 0x4F},
				{CRT_CTRL, CRTC_SHB, 0x50},
				{CRT_CTRL, CRTC_EHB, 0x82},
				{CRT_CTRL, CRTC_SHR, 0x54},
				{CRT_CTRL, CRTC_EHR, 0x80},
				{CRT_CTRL, CRTC_VT, 0x0B},
				{CRT_CTRL, CRTC_OVERFLOW, 0x3E},
				{CRT_CTRL, CRTC_PRS, 0x00},
				{CRT_CTRL, CRTC_MSL, 0x40},
				{CRT_CTRL, CRTC_VRS, 0xEA},
				{CRT_CTRL, CRTC_VRE, 0x8C},
				{CRT_CTRL, CRTC_VDE, 0xDF},
				{CRT_CTRL, CRTC_OFFSET, 0x28},
				{CRT_CTRL, CRTC_UL, 0x00},
				{CRT_CTRL, CRTC_SVB, 0xE7},
				{CRT_CTRL, CRTC_EVB, 0x04},
				{CRT_CTRL, CRTC_MC, 0xE3},
				{NO_REGISTER, 0, 0}
		},
		[VIDMEM_MODE13] = {
				{ATTRIBUTE_CTRL, ATTR_AMC, 0x41},
				{ATTRIBUTE_CTRL, ATTR_OVERSCAN_COLOR, 0xFF},
				{ATTRIBUTE_CTRL, ATTR_CPE, 0x0F},
				{ATTRIBUTE_CTRL, ATTR_HPP, 0x00},
				{ATTRIBUTE_CTRL, ATTR_COLOR_SELECT, 0x00},
				{EXTERNAL_REGISTERS, MISC_OUTPUT_REGISTER, 0x63},
				{CRT_CTRL, CRTC_VRE, 0x80},
				{SEQUENCER, SEQ_CLOCKING_MODE, 0x01},
				{SEQUENCER, SEQ_MAP_MASK, 0x0F},
				{SEQUENCER, SEQ_CHARACTER_MAP_SELECT, 0x00},
				{SEQUENCER, SEQ_MEMORY_MODE, 0x0E},
				{GRAPHIC_CTRL, GCTRL_GM, 0x40},
				{GRAPHIC_CTRL, GCTRL_MG, 0x05},
				{CRT_CTRL, CRTC_HT, 0x5F},
				{CRT_CTRL, CRTC_EHD, 0x4F},
				{CRT_CTRL, CRTC_SHB, 0x50},
				{CRT_CTRL, CRTC_EHB, 0x82},
				{CRT_CTRL, CRTC_SHR, 0x54},
				{CRT_CTRL, CRTC_EHR, 0x80},
				{CRT_CTRL, CRTC_VT, 0xBF},
				{CRT_CTRL, CRTC_OVERFLOW, 0x1F},
				{CRT_CTRL, CRTC_PRS, 0x00},
				{CRT_CTRL, CRTC_MSL, 0x41},
				{CRT_CTRL, CRTC_VRS, 0x9C},
				{CRT_CTRL, CRTC_VRE, 0x8E},
				{CRT_CTRL, CRTC_VDE, 0x8F},
				{CRT_CTRL, CRTC_OFFSET, 0x28},
				{CRT_CTRL, CRTC_UL, 0x40},
				{CRT_CTRL, CRTC_SVB, 0x96},
				{CRT_CTRL, CRTC_EVB, 0xB9},
				{CRT_CTRL, CRTC_MC, 0xA3},
				{NO_REGISTER, 0, 0}
		},
		[VIDMEM_MODEX] = {
				{ATTRIBUTE_CTRL, ATTR_AMC, 0x01},
				{ATTRIBUTE_CTRL, ATTR_OVERSCAN_COLOR, 0xFF},
				{ATTRIBUTE_CTRL, ATTR_CPE, 0x0F},
				{ATTRIBUTE_CTRL, ATTR_HPP, 0x00},
				{ATTRIBUTE_CTRL, ATTR_COLOR_SELECT, 0x00},
				{EXTERNAL_REGISTERS, MISC_OUTPUT_REGISTER, 0xE3},
				{SEQUENCER, SEQ_CLOCKING_MODE, 0x01},
				{SEQUENCER, SEQ_MAP_MASK, 0x0F},
				{SEQUENCER, SEQ_CHARACTER_MAP_SELECT, 0x00},
				{SEQUENCER, SEQ_MEMORY_MODE, 0x02},
				{GRAPHIC_CTRL, GCTRL_GM, 0x00},
				{GRAPHIC_CTRL, GCTRL_MG, 0x05},
				{CRT_CTRL, CRTC_VRE, 0x80},
				{CRT_CTRL, CRTC_HT, 0x5F},
				{CRT_CTRL, CRTC_EHD, 0x4F},
				{CRT_CTRL, CRTC_SHB, 0x50},
				{CRT_CTRL, CRTC_EHB, 0x82},
				{CRT_CTRL, CRTC_SHR, 0x54},
				{CRT_CTRL, CRTC_EHR, 0x80},
				{CRT_CTRL, CRTC_VT, 0x0B},
				{CRT_CTRL, CRTC_OVERFLOW, 0x3E},
				{CRT_CTRL, CRTC_PRS, 0x00},
				{CRT_CTRL, CRTC_MSL, 0x40},
				{CRT_CTRL, CRTC_VRS, 0xEA},
				{CRT_CTRL, CRTC_VRE, 0x8C},
				{CRT_CTRL, CRTC_VDE, 0xDF},
				{CRT_CTRL, CRTC_OFFSET, 0x28},
				{CRT_CTRL, CRTC_UL, 0x00},
				{CRT_CTRL, CRTC_SVB, 0xE7},
				{CRT_CTRL, CRTC_EVB, 0x04},
				{CRT_CTRL, CRTC_MC, 0xE3},
				{NO_REGISTER, 0, 0}
		}
};

void load_dac_palette(vidmem_dac_palette_t *palette)
{
	int index;
	for(index = 0; index < DAC_COLORS_NB; index++)
		load_dac_color(index, palette->colors[index].red, palette->colors[index].green, palette->colors[index].blue);
}

uint8_t read_vga_register(register_set_t target, uint8_t index)
{
	switch(target)
	{
	case GRAPHIC_CTRL:
		return gctrl_read(index);
		break;
	case SEQUENCER:
		return seq_read(index);
		break;
	case ATTRIBUTE_CTRL:
		return attr_read(index);
		break;
	case CRT_CTRL:
		return crtc_read(index);
		break;
	case EXTERNAL_REGISTERS:
		switch(index)
		{
		case MISC_OUTPUT_REGISTER:
			return misc_output_read();
			break;
		case FEATURE_CONTROL_REGISTER:
			return feature_control_read();
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}

void write_vga_register(register_set_t target, uint8_t index, uint8_t value)
{
	switch(target)
	{
	case GRAPHIC_CTRL:
		gctrl_write(index, value);
		break;
	case SEQUENCER:
		seq_write(index, value);
		break;
	case ATTRIBUTE_CTRL:
		attr_write(index, value);
		break;
	case CRT_CTRL:
		crtc_write(index, value);
		break;
	case EXTERNAL_REGISTERS:
		switch(index)
		{
		case MISC_OUTPUT_REGISTER:
			misc_output_write(value);
			break;
		case FEATURE_CONTROL_REGISTER:
			feature_control_write(value);
			break;
		}
		break;
	default:
		break;
	}
}

void __no_return vga_worker()
{
	do {
		busy_wait(0xFFFFFF);
		clear_screen(0xFF);
		busy_wait(0xFFFFFF);
		clear_screen(0x00);
		busy_wait(0xFFFFFF);
		render_plane(&vga_logo);
	} while(1);
}

void load_mode(vidmem_vga_mode_t mode)
{
	vidmem_register_t *reg = &vga_modes_registers[mode][0];
	while(reg->target != NO_REGISTER)
	{
		write_vga_register(reg->target, reg->index, reg->value);
		reg++;
	}

	//outb(0x1, 0x1CE + 0x4); // Enable bpp
	//outb(8, 0x1CE + 0x3); // Set bpp value
	//outb(8, 0x1CE); // Set bpp value

	attr_direct_write(0x00); // set load operation for attribute controller.
	load_palette();
	attr_direct_write(0x20); // set normal operation for attribute controller.

	switch (mode) {
		case VIDMEM_MODE3:
		case VIDMEM_MODE12:
		case VIDMEM_MODE13:
			vga_map.start = 0xA0000;
			vga_map.size = 0xAFFFF - 0xA0000;
			break;
		case VIDMEM_MODEX:
			vga_map.start = 0xB8000;
			vga_map.size = 0xBFFFF - 0xB8000;
			break;
		default:
			break;
	}
	vga_map.start = (uint64_t)vm_kmap(vga_map.start, vga_map.size);

	clear_screen(0x00);

	vga_logo.x = 0;
	vga_logo.y = 0;
	vga_logo.width = logo_width;
	vga_logo.height = logo_height;
	vga_logo.pixmap = logo_bitmap;
}

void clear_screen(uint8_t color)
{
	uint64_t cur_addr;
	for(cur_addr = vga_map.start; cur_addr < (vga_map.start + vga_map.size);cur_addr+=1)
		*((char *)cur_addr) = color;
}

#define VGA_WIDTH (0x28 * 2 * 4)
void render_plane(vidmem_plane_t *plane)
{
	uint64_t index = 0;
	uint64_t cur_width;
	uint64_t cur_height;
	uint64_t cur_addr;
	uint64_t start_left = vga_map.start + (plane->x + (plane->y * VGA_WIDTH));
	for (cur_height = 0; cur_height < plane->height; cur_height++)
	{
		cur_addr = start_left;
		for(cur_width = 0; cur_width < plane->width; cur_width++)
		{
			if(cur_addr > (vga_map.start + vga_map.size))
				goto exit_loop;
			*((char *)cur_addr++) = plane->pixmap[index++];
		}
		start_left += VGA_WIDTH;
	}
exit_loop:
	return;
}

/*
 * Initialization of VGA
 */
void vga_init(vidmem_vga_mode_t vga_mode)
{
	load_mode(vga_mode);
	struct proc *vga_proc = kthread_create(vga_worker);
	thread_start(vga_proc);
}
