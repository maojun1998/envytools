/*
 * Copyright (C) 2012 Marcin Kościelnicki <koriakin@0x04.net>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NVHW_H
#define NVHW_H

#include <stdint.h>

enum pgraph_type {
	PGRAPH_NV01,
	PGRAPH_NV03,
	PGRAPH_NV04,
	PGRAPH_NV10,
	PGRAPH_NV20,
	PGRAPH_NV40,
	PGRAPH_NV50,
	PGRAPH_NVC0,
};

int pgraph_type(int chipset);

enum {
	NV01_PGRAPH_STATE_DEBUG_0,
	NV01_PGRAPH_STATE_DEBUG_1,
	NV01_PGRAPH_STATE_DEBUG_2,
	NV01_PGRAPH_STATE_INTR,
	NV01_PGRAPH_STATE_INVALID,
	NV01_PGRAPH_STATE_INTR_EN,
	NV01_PGRAPH_STATE_INVALID_EN,
	NV01_PGRAPH_STATE_CTX_SWITCH,
	NV01_PGRAPH_STATE_CTX_CONTROL,
	NV01_PGRAPH_STATE_ICLIP,
	NV01_PGRAPH_STATE_UCLIP_MIN = NV01_PGRAPH_STATE_ICLIP + 2,
	NV01_PGRAPH_STATE_UCLIP_MAX = NV01_PGRAPH_STATE_UCLIP_MIN + 2,
	NV01_PGRAPH_STATE_VTX_X = NV01_PGRAPH_STATE_UCLIP_MAX + 2,
	NV01_PGRAPH_STATE_VTX_Y = NV01_PGRAPH_STATE_VTX_X + 18,
	NV01_PGRAPH_STATE_VTX_BETA = NV01_PGRAPH_STATE_VTX_Y + 18,
	NV01_PGRAPH_STATE_PATTERN_RGB = NV01_PGRAPH_STATE_VTX_BETA + 14,
	NV01_PGRAPH_STATE_PATTERN_A = NV01_PGRAPH_STATE_PATTERN_RGB + 2,
};

struct nv01_pgraph_state {
	uint32_t intr;
	uint32_t invalid;
	uint32_t intr_en;
	uint32_t invalid_en;
	uint32_t ctx_switch;
	uint32_t ctx_control;
	uint32_t iclip[2];
	uint32_t uclip_min[2];
	uint32_t uclip_max[2];
	uint32_t vtx_x[18];
	uint32_t vtx_y[18];
	uint32_t vtx_beta[14];
	uint32_t pattern_rgb[2];
	uint32_t pattern_a[2];
	uint32_t pattern_mono[2];
	uint32_t pattern_shape;
	uint32_t bitmap_color[2];
	uint32_t rop;
	uint32_t plane;
	uint32_t chroma;
	uint32_t beta;
	uint32_t canvas_config;
	uint32_t canvas_min;
	uint32_t canvas_max;
	uint32_t cliprect_min[2];
	uint32_t cliprect_max[2];
	uint32_t cliprect_ctrl;
	uint32_t valid;
	uint32_t source_color;
	uint32_t subdivide;
	uint32_t edgefill;
	uint32_t x_misc;
	uint32_t y_misc;
	uint32_t xy_misc_0;
	uint32_t xy_misc_1;
	uint32_t dma;
	uint32_t notify;
	uint32_t access;
	uint32_t debug[3];
	/* not exactly PGRAPH reg, but important */
	uint32_t pfb_config;
};

#endif