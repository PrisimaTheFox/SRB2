// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2016 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_local.h
/// \brief Refresh (R_*) module, global header. All the rendering/drawing stuff is here

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Screen size related parameters.
#include "doomdef.h"

// Binary Angles, sine/cosine/atan lookups.
#include "tables.h"

// this one holds the max vid sizes and standard aspect
#include "screen.h"

#include "m_bbox.h"

#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_sky.h"
#include "r_data.h"
#include "r_things.h"
#include "r_draw.h"

extern drawseg_t *firstseg;

void SplitScreen_OnChange(void);

//LUASPRITE
#define LSF_10TRANS 0x1
#define LSF_20TRANS 0x2
#define LSF_30TRANS 0x4
#define LSF_40TRANS 0x8
#define LSF_50TRANS 0x10
#define LSF_60TRANS 0x20
#define LSF_70TRANS 0x40
#define LSF_80TRANS 0x80
#define LSF_90TRANS 0x100
#define LSF_VFLIP   0x200	//Flip it upside down

typedef struct {
	const char *patchname;				//sprite's patch name
	UINT16 width;						//sprite width
	UINT16 height;						//sprite height
	INT16 offsetx;						//X offset
	INT16 offsety;						//Y offset
	UINT8 data[UINT32_MAX/UINT8_MAX];	//Mobj's pixel data
	UINT32 flags;						//Transparency and flip
	fixed_t scalex;						//X scale
	fixed_t scaley;						//Y scale
	mobj_t* mo;							//Mobj of the sprite
} luasprite_t;

#endif // __R_LOCAL__
