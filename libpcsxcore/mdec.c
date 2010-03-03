/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
* Movie decoder. Based on the FPSE v0.08 Mdec decoder.
*/

#include "psxhw.h"
#include "mdec.h"

// Need to find endianess issue.
#if !defined(__BIGENDIAN__) && !defined(GEKKO) 
#define MAME_MDEC
#endif

#ifndef MAME_MDEC
#define CONST_BITS  8
#define PASS1_BITS  2

#define FIX_1_082392200  (277)
#define FIX_1_414213562  (362)
#define FIX_1_847759065  (473)
#define FIX_2_613125930  (669)

#define MULTIPLY(var,const)  (DESCALE((var) * (const), CONST_BITS))

#define DEQUANTIZE(coef,quantval)  (coef)

//#define DESCALE(x,n)  ((x)>>(n))

#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#define ONE ((s32) 1)
#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)
#define	RANGE(n)	(n)
#endif // MAME_MDEC

#define	DCTSIZE		( 8 )
#define	DCTSIZE2	( DCTSIZE * DCTSIZE )

// mdec status:
#define MDEC_DREQ	0x18000000
#define MDEC_FIFO	0xc0000000
#define MDEC_STP	( 1L << 25 )
#define MDEC_RGB15	( 1L << 27 )
#define MDEC_BUSY	( 1L << 29 )

struct {
	u32 flag:1;
	u32 size;
#ifndef MAME_MDEC
	u32 command;
	u32 status;
	u16 *rl;
#else
	u32 address;
	u32 command0;
	u32 command1;
	u32 status0;
	u32 status1;
	u32 decoded;
#endif
} mdec;

static const u32 zscan[ DCTSIZE2 ] =
{
	0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

// MAME {{{{{{{{{{{{{{{{{
#ifdef MAME_MDEC
#	if defined(GEKKO) || defined(__BIGENDIAN__)
#		define WORD_XOR_LE(a) ((a) ^ 2)
#	else
#		define WORD_XOR_LE(a) ((a) ^ 0)
#	endif

static u16 mdec_output[ 24 * 16 ];

static s32 m_p_n_mdec_quantize_y[ DCTSIZE2 ];
static s32 m_p_n_mdec_quantize_uv[ DCTSIZE2 ];
static s32 m_p_n_mdec_cos[ DCTSIZE2 ];
static s32 m_p_n_mdec_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

static u16 m_p_n_mdec_clamp8[ 256 * 3 ];
static u16 m_p_n_mdec_r5[ 256 * 3 ];
static u16 m_p_n_mdec_g5[ 256 * 3 ];
static u16 m_p_n_mdec_b5[ 256 * 3 ];

static s32 m_p_n_mdec_unpacked[ DCTSIZE2 * 6 * 2 ];

#define MDEC_COS_PRECALC_BITS ( 21 )

static void mdec_cos_precalc( void )
{
	u32 n_x;
	u32 n_y;
	u32 n_u;
	u32 n_v;
	s32 *p_n_precalc;

	p_n_precalc = m_p_n_mdec_cos_precalc;

	for( n_y = 0; n_y < 8; n_y++ )
	{
		for( n_x = 0; n_x < 8; n_x++ )
		{
			for( n_v = 0; n_v < 8; n_v++ )
			{
				for( n_u = 0; n_u < 8; n_u++ )
				{
					*( p_n_precalc++ ) = 
						( ( m_p_n_mdec_cos[ ( n_u * 8 ) + n_x ] * 
						m_p_n_mdec_cos[ ( n_v * 8 ) + n_y ] ) >> ( 30 - MDEC_COS_PRECALC_BITS ) );
				}
			}
		}
	}
}

static void mdec_idct( s32 *p_n_src, s32 *p_n_dst )
{
	u32 n_yx;
	u32 n_vu;
	s32 p_n_z[ 8 ];
	s32 *p_n_data;
	s32 *p_n_precalc;

	p_n_precalc = m_p_n_mdec_cos_precalc;

	for( n_yx = 0; n_yx < DCTSIZE2; n_yx++ )
	{
		p_n_data = p_n_src;
 
		memset( p_n_z, 0, sizeof( p_n_z ) );
	
		for( n_vu = 0; n_vu < DCTSIZE2 / 8; n_vu++ )
		{
			p_n_z[ 0 ] += p_n_data[ 0 ] * p_n_precalc[ 0 ];
			p_n_z[ 1 ] += p_n_data[ 1 ] * p_n_precalc[ 1 ];
			p_n_z[ 2 ] += p_n_data[ 2 ] * p_n_precalc[ 2 ];
			p_n_z[ 3 ] += p_n_data[ 3 ] * p_n_precalc[ 3 ];
			p_n_z[ 4 ] += p_n_data[ 4 ] * p_n_precalc[ 4 ];
			p_n_z[ 5 ] += p_n_data[ 5 ] * p_n_precalc[ 5 ];
			p_n_z[ 6 ] += p_n_data[ 6 ] * p_n_precalc[ 6 ];
			p_n_z[ 7 ] += p_n_data[ 7 ] * p_n_precalc[ 7 ];
			p_n_data += 8;
			p_n_precalc += 8;
		}
		*( p_n_dst++ ) = ( p_n_z[ 0 ] + p_n_z[ 1 ] + p_n_z[ 2 ] + p_n_z[ 3 ] +
			p_n_z[ 4 ] + p_n_z[ 5 ] + p_n_z[ 6 ] + p_n_z[ 7 ] ) >> ( MDEC_COS_PRECALC_BITS + 2 );
	}
}


__inline u16 mdec_unpack_run( u16 n_packed )
{
	return n_packed >> 10;
}

__inline s32 mdec_unpack_val( u16 n_packed )
{
	return ( ( (s32)n_packed ) << 22 ) >> 22;
}

static u32 mdec_unpack( u32 n_address )
{
	u8 n_z;
	s32 n_qscale;
	u16 n_packed;
	u32 n_block;
	s32 *p_n_block;
	s32 p_n_unpacked[ 64 ];
	s32 *p_n_q;
 
	p_n_q = m_p_n_mdec_quantize_uv;
	p_n_block = m_p_n_mdec_unpacked;
 
	for( n_block = 0; n_block < 6; n_block++ )
	{
		memset( p_n_unpacked, 0, sizeof( p_n_unpacked ) );
 
		if( n_block == 2 )
		{
			p_n_q = m_p_n_mdec_quantize_y;
		}
		n_packed = PSXMu16( n_address );
		n_address += 2;
		if( n_packed == 0xfe00 )
		{
			break;
		}
 
		n_qscale = mdec_unpack_run( n_packed );
		p_n_unpacked[ 0 ] = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];
 
		n_z = 0;
		for( ;; )
		{
			n_packed = PSXMu16( n_address );
			n_address += 2;
 
			if( n_packed == 0xfe00 )
			{
				break;
			}
			n_z += mdec_unpack_run( n_packed ) + 1;
			if( n_z > 63 )
			{
				break;
			}
			p_n_unpacked[ zscan[ n_z ] ] = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
		}
		mdec_idct( p_n_unpacked, p_n_block );
		p_n_block += DCTSIZE2;
	}
	return n_address;
}

__inline s32 mdec_cr_to_r( s32 n_cr )
{
	return ( 1435 * n_cr ) >> 10;
}
 
__inline s32 mdec_cr_to_g( s32 n_cr )
{
	return ( -731 * n_cr ) >> 10;
}

__inline s32 mdec_cb_to_g( s32 n_cb )
{
	return ( -351 * n_cb ) >> 10;
}
 
__inline s32 mdec_cb_to_b( s32 n_cb )
{
	return ( 1814 * n_cb ) >> 10;
}

__inline u16 mdec_clamp_r5( s32 n_r )
{
	return m_p_n_mdec_r5[ n_r + 128 + 256 ];
}

__inline u16 mdec_clamp_g5( s32 n_g )
{
	return m_p_n_mdec_g5[ n_g + 128 + 256 ];
}

__inline u16 mdec_clamp_b5( s32 n_b )
{
	return m_p_n_mdec_b5[ n_b + 128 + 256 ];
}

__inline void mdec_makergb15( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u16 n_stp )
{
	mdec_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 0 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 0 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 0 ] + n_b );
 
	mdec_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 1 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 1 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 1 ] + n_b );
}

static u32 mdec_yuv2_to_rgb15( void )
{
	s32 n_r;
	s32 n_g;
	s32 n_b;
	s32 n_cb;
	s32 n_cr;
	s32 *p_n_cb;
	s32 *p_n_cr;
	s32 *p_n_y;
	u32 n_x;
	u32 n_y;
	u32 n_z;
	u16 n_stp;
	int n_address = 0;
 
	if( ( mdec.command0 & MDEC_STP ) != 0 )
	{
		n_stp = 0x8000;
	}
	else
	{
		n_stp = 0x0000;
	}
 
	p_n_cr = &m_p_n_mdec_unpacked[ 0 ];
	p_n_cb = &m_p_n_mdec_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_mdec_unpacked[ DCTSIZE2 * 2 ];
 
	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );
 
				mdec_makergb15( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb15( ( n_address + 32 ), n_r, n_g, n_b, p_n_y + 8, n_stp );
 
				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );
 
				mdec_makergb15( ( n_address + 16 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb15( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );
 
				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 4;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 48;
		}
		p_n_y += DCTSIZE2;
	}
	mdec.decoded = ( 16 * 16 ) / 2;
	return mdec.decoded;
}

__inline u16 mdec_clamp8( s32 n_r )
{
	return m_p_n_mdec_clamp8[ n_r + 128 + 256 ];
}

__inline void mdec_makergb24( u32 n_address, s32 n_r, s32 n_g, s32 n_b, s32 *p_n_y, u32 n_stp )
{
	mdec_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = ( mdec_clamp8( p_n_y[ 0 ] + n_g ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_r );
	mdec_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_r ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_b );
	mdec_output[ WORD_XOR_LE( n_address + 4 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_b ) << 8 ) | mdec_clamp8( p_n_y[ 1 ] + n_g );
}

static u32 mdec_yuv2_to_rgb24( void )
{
	s32 n_r;
	s32 n_g;
	s32 n_b;
	s32 n_cb;
	s32 n_cr;
	s32 *p_n_cb;
	s32 *p_n_cr;
	s32 *p_n_y;
	u32 n_x;
	u32 n_y;
	u32 n_z;
	u32 n_stp;
	int n_address = 0;
 
	if( ( mdec.command0 & MDEC_STP ) != 0 )
	{
		n_stp = 0x80008000;
	}
	else
	{
		n_stp = 0x00000000;
	}
 
	p_n_cr = &m_p_n_mdec_unpacked[ 0 ];
	p_n_cb = &m_p_n_mdec_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_mdec_unpacked[ DCTSIZE2 * 2 ];
 
	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );
 
				mdec_makergb24( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb24( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + 8, n_stp );
 
				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );
 
				mdec_makergb24( ( n_address + 24 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb24( ( n_address + 72 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );
 
				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 6;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 72;
		}
		p_n_y += DCTSIZE2;
	}
	mdec.decoded = ( 24 * 16 ) / 2;
	return mdec.decoded;
}

#else
// }}}}}}}}}}}}}}}}}

static void idct1(int *block)
{
	int val = RANGE(DESCALE(block[0], PASS1_BITS+3));
	int i;
	for(i=0;i<DCTSIZE2;i++) block[i]=val;
}

static void idct(int *block,int k)
{

  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  int z5, z10, z11, z12, z13;
  int *ptr;
  int i;

  if (!k) { idct1(block); return; }

  ptr = block;
  for (i = 0; i< DCTSIZE; i++,ptr++) {
    
    if ((ptr[DCTSIZE*1] | ptr[DCTSIZE*2] | ptr[DCTSIZE*3] |
	 ptr[DCTSIZE*4] | ptr[DCTSIZE*5] | ptr[DCTSIZE*6] |
	 ptr[DCTSIZE*7]) == 0) {
      ptr[DCTSIZE*0] =
      ptr[DCTSIZE*1] = 
      ptr[DCTSIZE*2] = 
      ptr[DCTSIZE*3] = 
      ptr[DCTSIZE*4] = 
      ptr[DCTSIZE*5] = 
      ptr[DCTSIZE*6] = 
      ptr[DCTSIZE*7] = 
      	ptr[DCTSIZE*0];
      
      continue;
    }
    
    z10 = ptr[DCTSIZE*0] + ptr[DCTSIZE*4];
    z11 = ptr[DCTSIZE*0] - ptr[DCTSIZE*4];
    z13 = ptr[DCTSIZE*2] + ptr[DCTSIZE*6];
    z12 = MULTIPLY(ptr[DCTSIZE*2] - ptr[DCTSIZE*6], FIX_1_414213562) - z13;

    tmp0 = z10 + z13;
    tmp3 = z10 - z13;
    tmp1 = z11 + z12;
    tmp2 = z11 - z12;
    
    z13 = ptr[DCTSIZE*5] + ptr[DCTSIZE*3];
    z10 = ptr[DCTSIZE*5] - ptr[DCTSIZE*3];
    z11 = ptr[DCTSIZE*1] + ptr[DCTSIZE*7];
    z12 = ptr[DCTSIZE*1] - ptr[DCTSIZE*7];
	
	tmp7 = z11 + z13;

    z5 = MULTIPLY(z12 + z10, FIX_1_847759065);
    
    tmp6 = MULTIPLY(z10, -FIX_2_613125930) + z5 - tmp7;
    tmp5 = MULTIPLY(z11 - z13, FIX_1_414213562) - tmp6;
    tmp4 = MULTIPLY(z12, FIX_1_082392200) - z5 + tmp5;

    ptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
    ptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
    ptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
    ptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
    ptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
    ptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
    ptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
    ptr[DCTSIZE*3] = (int) (tmp3 - tmp4);

  }
  
  ptr = block;
  for (i = 0; i < DCTSIZE; i++ ,ptr+=DCTSIZE) {
    
    if ((ptr[1] | ptr[2] | ptr[3] | ptr[4] | ptr[5] | ptr[6] |
	 ptr[7]) == 0) {
      ptr[0] =
      ptr[1] = 
      ptr[2] = 
      ptr[3] = 
      ptr[4] = 
      ptr[5] = 
      ptr[6] = 
      ptr[7] = 
      	RANGE(DESCALE(ptr[0], PASS1_BITS+3));

      continue;
    }

    z10 = ptr[0] + ptr[4];
    z11 = ptr[0] - ptr[4];
    z13 = ptr[2] + ptr[6];
    z12 = MULTIPLY(ptr[2] - ptr[6], FIX_1_414213562) - z13;

    tmp0 = z10 + z13;
    tmp3 = z10 - z13;
    tmp1 = z11 + z12;
    tmp2 = z11 - z12;

    z13 = ptr[5] + ptr[3];
    z10 = ptr[5] - ptr[3];
    z11 = ptr[1] + ptr[7];
    z12 = ptr[1] - ptr[7];

    z5 = MULTIPLY(z12 + z10, FIX_1_847759065);
    tmp7 = z11 + z13;
    tmp6 = MULTIPLY(z10, -FIX_2_613125930) + z5 - tmp7;
    tmp5 = MULTIPLY(z11 - z13, FIX_1_414213562) - tmp6;
    tmp4 = MULTIPLY(z12, FIX_1_082392200) - z5 + tmp5;

    ptr[0] = RANGE(DESCALE(tmp0 + tmp7, PASS1_BITS+3));;
    ptr[7] = RANGE(DESCALE(tmp0 - tmp7, PASS1_BITS+3));;
    ptr[1] = RANGE(DESCALE(tmp1 + tmp6, PASS1_BITS+3));;
    ptr[6] = RANGE(DESCALE(tmp1 - tmp6, PASS1_BITS+3));;
    ptr[2] = RANGE(DESCALE(tmp2 + tmp5, PASS1_BITS+3));;
    ptr[5] = RANGE(DESCALE(tmp2 - tmp5, PASS1_BITS+3));;
    ptr[4] = RANGE(DESCALE(tmp3 + tmp4, PASS1_BITS+3));;
    ptr[3] = RANGE(DESCALE(tmp3 - tmp4, PASS1_BITS+3));;

  }

}

static unsigned short* rl2blk(int *blk,unsigned short *mdec_rl);
static void iqtab_init(int *iqtab,unsigned char *iq_y);
static void yuv2rgb24(int *blk,unsigned char *image);
static void yuv2rgb15(int *blk,unsigned short *image);

static int iq_y[DCTSIZE2],iq_uv[DCTSIZE2];
#endif

void mdecInit(void) {
#ifndef MAME_MDEC
	mdec.rl = (u16*)&psxM[0x100000];
	mdec.command = 0;
	mdec.status = 0;
	mdec.flag = 0;
#else
	int n;
	for( n = 0; n < 256; n++ )
	{
		m_p_n_mdec_clamp8[ n ] = 0;
		m_p_n_mdec_clamp8[ n + 256 ] = n;
		m_p_n_mdec_clamp8[ n + 512 ] = 255;
 
		m_p_n_mdec_r5[ n ] = 0;
		m_p_n_mdec_r5[ n + 256 ] = ( n >> 3 );
		m_p_n_mdec_r5[ n + 512 ] = ( 255 >> 3 );
 
		m_p_n_mdec_g5[ n ] = 0;
		m_p_n_mdec_g5[ n + 256 ] = ( n >> 3 ) << 5;
		m_p_n_mdec_g5[ n + 512 ] = ( 255 >> 3 ) << 5;
 
		m_p_n_mdec_b5[ n ] = 0;
		m_p_n_mdec_b5[ n + 256 ] = ( n >> 3 ) << 10;
		m_p_n_mdec_b5[ n + 512 ] = ( 255 >> 3 ) << 10;
	}
	
	mdec.command0 = 0;
	mdec.address = 0;
	mdec.size = 0;
	mdec.command1 = 0;
	mdec.status1 = 0;
#endif

}


void mdecWrite0(u32 data) {
#ifdef CDR_LOG
	CDR_LOG("mdec0 write %lx\n", data);
#endif
#ifndef MAME_MDEC
	mdec.command = data;
	if ((data&0xf5ff0000)==0x30000000) {
		mdec.size = data&0xffff;
	}
#else
	mdec.command0 = data;
#endif
}

void mdecWrite1(u32 data) {
#ifdef CDR_LOG
	CDR_LOG("mdec1 write %lx\n", data);
#endif
#ifndef MAME_MDEC
	mdec.command = data;
	if (data & 0x80000000) { // mdec reset
		mdec.command = 0;
		mdec.status = 0;
		mdec.flag = 0;
	}
#else
	mdec.command1 = data;
#endif
}

u32 mdecRead0(void) {
#ifdef CDR_LOG
	CDR_LOG("mdec0 read %lx\n", mdec.command);
#endif
	return 0; //mdec.command
}

u32 mdecRead1(void) {
#ifdef CDR_LOG
	CDR_LOG("mdec1 read %lx\n", mdec.status);
#endif
	// Most of games call mdecRead1() before psxDma0(). If mdec.status == MDEC_BUSY, they wait and send data -> slowdown.
	// FF9 make extra calls of mdecRead1() before psxDma1() and reset Mdec before call psxDma0(). 
	// If mdec.status == 0 after one of extra calls it start another drawing cycle without sending data to memory -> crash (Firnis)
	if(mdec.flag)
#ifdef MAME_MDEC
		mdec.status1 &= ~MDEC_BUSY;
#else
		mdec.status &= ~MDEC_BUSY;
#endif
	mdec.flag = 1;
#ifdef MAME_MDEC
	return mdec.status1;
#else
	return mdec.status;
#endif
}

void psxDma0(u32 adr, u32 bcr, u32 chcr) {
#ifdef CDR_LOG
	CDR_LOG("DMA0 %lx %lx %lx\n", adr, bcr, chcr);
#endif

	if (chcr!=0x01000201) {
		SysPrintf("chcr != 0x01000201\n");
		return;
	}

 	mdec.flag = 0;
	//int size = (bcr >> 16) * (bcr & 0xffff);
	u32 size = bcr >> 16;
	if( size == 0 )
	{
		size = 0x10000;
	}
	size = ( bcr & 0xffff ) * size;
#ifdef MAME_MDEC
	int n_index;
 
	//SysPrintf( "mdec0_write( %08x, %08x )\n", adr, size );
		switch( mdec.command0 >> 28 )
		{
		case 0x3:
			//SysPrintf( "mdec decode %08x %08x %08x\n", mdec.command0, adr, size );
			mdec.address = adr;
			mdec.size = size * 4;
			mdec.status1 |= MDEC_BUSY;
			break;
		case 0x4:
			//SysPrintf( "mdec quantize table %08x %08x %08x\n", mdec.command0, adr, size );
			n_index = 0;
			while( size > 0 )
			{
				if( n_index < DCTSIZE2 )
				{
					m_p_n_mdec_quantize_y[ n_index + 0 ] = ( PSXMu32(adr ) >> 0 ) & 0xff;
					m_p_n_mdec_quantize_y[ n_index + 1 ] = ( PSXMu32(adr ) >> 8 ) & 0xff;
					m_p_n_mdec_quantize_y[ n_index + 2 ] = ( PSXMu32(adr ) >> 16 ) & 0xff;
					m_p_n_mdec_quantize_y[ n_index + 3 ] = ( PSXMu32(adr ) >> 24 ) & 0xff;
				}
				else if( n_index < DCTSIZE2 * 2 )
				{
					m_p_n_mdec_quantize_uv[ n_index + 0 - DCTSIZE2 ] = ( PSXMu32(adr ) >> 0 ) & 0xff;
					m_p_n_mdec_quantize_uv[ n_index + 1 - DCTSIZE2 ] = ( PSXMu32(adr ) >> 8 ) & 0xff;
					m_p_n_mdec_quantize_uv[ n_index + 2 - DCTSIZE2 ] = ( PSXMu32(adr ) >> 16 ) & 0xff;
					m_p_n_mdec_quantize_uv[ n_index + 3 - DCTSIZE2 ] = ( PSXMu32(adr ) >> 24 ) & 0xff;
				}
				n_index += 4;
				adr += 4;
				size--;
			}
			break;
		case 0x6:
			//SysPrintf( "mdec cosine table %08x %08x %08x\n", mdec.command0, adr, size );
			n_index = 0;
			while( size > 0 )
			{
				m_p_n_mdec_cos[ n_index + 0 ] = (s16)( ( PSXMu32(adr ) >> 0 ) & 0xffff );
				m_p_n_mdec_cos[ n_index + 1 ] = (s16)( ( PSXMu32(adr ) >> 16 ) & 0xffff );
				n_index += 2;
				adr += 4;
				size--;
			}
			mdec_cos_precalc();
			break;
		default:
			SysPrintf( "mdec unknown command %08x %08x %08x\n", mdec.command0, adr, size );
			break;
	}

#else
	switch( mdec.command >> 28 ) {
		case 0x3:	// cmd&0xf5ff0000)==0x30000000
			mdec.status|= MDEC_BUSY;
			mdec.rl = (u16*)PSXM(adr);
			break;

		case 0x4:	// cmd==0x40000001
		{
			u8 *p = (u8*)PSXM(adr);
			iqtab_init(iq_y,p);
			iqtab_init(iq_uv,p+64);
			break;
		}

		case 0x6:	// cmd==0x60000000

			break;

		default: SysPrintf("mdec unknown command\n");
	}
#endif
	HW_DMA0_CHCR &= SWAP32(~0x11000000);
	psxDmaInterrupt(0);

}

void psxDma1(u32 adr, u32 bcr, u32 chcr) {
	int blk[DCTSIZE2*6];
	u16 *image;

#ifdef CDR_LOG
	CDR_LOG("DMA1 %lx %lx %lx (cmd = %lx)\n", adr, bcr, chcr, mdec.command);
#endif
	if (chcr != 0x01000200){
		return;
	}
 	mdec.flag = 0;
	u32 size = bcr >> 16;
	if( size == 0 )
	{
		size = 0x10000;
	}
	size = ( bcr & 0xffff ) * size;

#ifdef MAME_MDEC
	u32 n_this;
	u32 n_nextaddress;
	u32 mdec_offset = 0;
	u32 decoded = 0;

	if( ( mdec.command0 & MDEC_BUSY ) != 0 && mdec.size != 0 )
	{
		while( size > 0 )
		{
			if( decoded == 0 )
			{
				if( (int)mdec.size <= 0 )
				{
					SysPrintf( "ran out of data %08x\n", size );
					mdec.size = 0;
					break;
				}
 
				n_nextaddress = mdec_unpack( mdec.address );
				mdec.size -= n_nextaddress - mdec.address;
				mdec.address = n_nextaddress;
 
				if( ( mdec.command0 & MDEC_RGB15 ) != 0 )
				{
					decoded = mdec_yuv2_to_rgb15();
				}
				else
				{
					decoded = mdec_yuv2_to_rgb24();
				}
				mdec_offset = 0;
			}
 
			n_this = decoded;
			if( n_this > size )
			{
				n_this = size;
			}
			decoded -= n_this;
 
			memcpy( (u8 *)PSXM(adr), (u8 *)mdec_output + mdec_offset, n_this * 4 );
			mdec_offset += n_this * 4;
			adr += n_this * 4;
			size -= n_this;
		}

 		if( (int)mdec.size < 0 )
		{
			SysPrintf( "ran out of data %d\n", mdec.size );
		}
	}
	else
	{
		//SysPrintf( "mdec1_read no conversion :%08x:%08x:\n", mdec.command0, mdec.size );
	}
	//mdec.status1 &= ~MDEC_BUSY;

#else
	image = (u16*)PSXM(adr);

	if (mdec.command & 0x08000000) {
		size = size / ((16 * 16) / 2);
		for ( ; size > 0; size--, image += (16 * 16) ) {
			mdec.rl = rl2blk( blk, mdec.rl );
			yuv2rgb15( blk, image );
		}
	} else {
		size = size / ((24*16)/2);
		for (;size>0;size--,image+=(24*16)) {
			mdec.rl = rl2blk(blk,mdec.rl);
			yuv2rgb24(blk,(u8 *)image);
		}
	}
#endif
	HW_DMA1_CHCR &= SWAP32(~0x11000000);
	psxDmaInterrupt(1);
}
#ifndef MAME_MDEC
#define	RUNOF(a)	((a)>>10)
#define	VALOF(a)	(((int)(a)<<(32-10))>>(32-10))

static int aanscales[DCTSIZE2] = {
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
	  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
	  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
	   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
	   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

static void iqtab_init(int *iqtab,unsigned char *iq_y) {
#define CONST_BITS14 14
#define	IFAST_SCALE_BITS 2
	int i;

	for(i=0;i<DCTSIZE2;i++) {
		iqtab[i] =iq_y[i] *aanscales[zscan[i]]>>(CONST_BITS14-IFAST_SCALE_BITS);
	}
}

#define	NOP	0xfe00
static unsigned short* rl2blk(int *blk,unsigned short *mdec_rl) {
	int i,k,q_scale,rl;
	int *iqtab;

	memset (blk, 0, 6*DCTSIZE2*4);
	iqtab = iq_uv;
	for(i=0;i<6;i++) {	// decode blocks (Cr,Cb,Y1,Y2,Y3,Y4)
		if (i>1) iqtab = iq_y;

		// zigzag transformation
		rl = GETLE16(mdec_rl); mdec_rl++;
		q_scale = RUNOF(rl);
		blk[0] = iqtab[0]*VALOF(rl);
		for(k = 0;;) {
			rl = GETLE16(mdec_rl); mdec_rl++;
			if (rl==NOP) break;
			k += RUNOF(rl)+1;	// skip level zero-coefficients
			if (k > 63) break;
			blk[zscan[k]] = (VALOF(rl) * iqtab[k] * q_scale) / 8; // / 16;
		}

		// idct
		idct(blk,k+1);

		blk+=DCTSIZE2;
	}
	return mdec_rl;
}

#define	MULR(a)		((((int)0x0000059B) * (a)) >> 10)
#define	MULG(a)		((((int)0xFFFFFEA1) * (a)) >> 10)
#define	MULG2(a)	((((int)0xFFFFFD25) * (a)) >> 10)
#define	MULB(a)		((((int)0x00000716) * (a)) >> 10)

#define	MAKERGB15(r, g, b)	( SWAP16((((r) >> 3) << 10)|(((g) >> 3) << 5)|((b) >> 3)) )
#define ROUND(c)	( ((c) < -128) ? 0 : (((c) > (255 - 128)) ? 255 : ((c) + 128)) )

#define RGB15(n, Y) \
	image[n] = MAKERGB15(ROUND(Y + R),ROUND(Y + G),ROUND(Y + B));

#define RGB15BW(n, Y) \
	image[n] = MAKERGB15(ROUND(Y),ROUND(Y),ROUND(Y));

#define RGB24(n, Y) \
	image[n+2] = ROUND(Y + R); \
	image[n+1] = ROUND(Y + G); \
	image[n+0] = ROUND(Y + B);

#define RGB24BW(n, Y) \
	image[n+2] = ROUND(Y); \
	image[n+1] = ROUND(Y); \
	image[n+0] = ROUND(Y);

static void yuv2rgb15(int *blk,unsigned short *image) {
	int x,y;
	int *Yblk = blk+DCTSIZE2*2;
	int Cb,Cr,R,G,B;
	int *Cbblk = blk;
	int *Crblk = blk+DCTSIZE2;

	if (!Config.Mdec)
	for (y=0;y<16;y+=2,Crblk+=4,Cbblk+=4,Yblk+=8,image+=24) {
		if (y==8) Yblk+=DCTSIZE2;
		for (x=0;x<4;x++,image+=2,Crblk++,Cbblk++,Yblk+=2) {
			Cr = *Crblk;
			Cb = *Cbblk;
			R = MULR(Cr);
			G = MULG(Cb) + MULG2(Cr);
			B = MULB(Cb);

			RGB15(0, Yblk[0]);
			RGB15(1, Yblk[1]);
			RGB15(16, Yblk[8]);
			RGB15(17, Yblk[9]);

			Cr = *(Crblk+4);
			Cb = *(Cbblk+4);
			R = MULR(Cr);
			G = MULG(Cb) + MULG2(Cr);
			B = MULB(Cb);

			RGB15(8, Yblk[DCTSIZE2+0]);
			RGB15(9, Yblk[DCTSIZE2+1]);
			RGB15(24, Yblk[DCTSIZE2+8]);
			RGB15(25, Yblk[DCTSIZE2+9]);
		}
	} else
	for (y=0;y<16;y+=2,Yblk+=8,image+=24) {
		if (y==8) Yblk+=DCTSIZE2;
		for (x=0;x<4;x++,image+=2,Yblk+=2) {
			RGB15BW(0, Yblk[0]);
			RGB15BW(1, Yblk[1]);
			RGB15BW(16, Yblk[8]);
			RGB15BW(17, Yblk[9]);

			RGB15BW(8, Yblk[DCTSIZE2+0]);
			RGB15BW(9, Yblk[DCTSIZE2+1]);
			RGB15BW(24, Yblk[DCTSIZE2+8]);
			RGB15BW(25, Yblk[DCTSIZE2+9]);
		}
	}
}

static void yuv2rgb24(int *blk,unsigned char *image) {
	int x,y;
	int *Yblk = blk+DCTSIZE2*2;
	int Cb,Cr,R,G,B;
	int *Cbblk = blk;
	int *Crblk = blk+DCTSIZE2;

	if (!Config.Mdec)
	for (y=0;y<16;y+=2,Crblk+=4,Cbblk+=4,Yblk+=8,image+=24*3) {
		if (y==8) Yblk+=DCTSIZE2;
		for (x=0;x<4;x++,image+=6,Crblk++,Cbblk++,Yblk+=2) {
			Cr = *Crblk;
			Cb = *Cbblk;
			R = MULR(Cr);
			G = MULG(Cb) + MULG2(Cr);
			B = MULB(Cb);

			RGB24(0, Yblk[0]);
			RGB24(1*3, Yblk[1]);
			RGB24(16*3, Yblk[8]);
			RGB24(17*3, Yblk[9]);

			Cr = *(Crblk+4);
			Cb = *(Cbblk+4);
			R = MULR(Cr);
			G = MULG(Cb) + MULG2(Cr);
			B = MULB(Cb);

			RGB24(8*3, Yblk[DCTSIZE2+0]);
			RGB24(9*3, Yblk[DCTSIZE2+1]);
			RGB24(24*3, Yblk[DCTSIZE2+8]);
			RGB24(25*3, Yblk[DCTSIZE2+9]);
		}
	} else
	for (y=0;y<16;y+=2,Yblk+=8,image+=24*3) {
		if (y==8) Yblk+=DCTSIZE2;
		for (x=0;x<4;x++,image+=6,Yblk+=2) {
			RGB24BW(0, Yblk[0]);
			RGB24BW(1*3, Yblk[1]);
			RGB24BW(16*3, Yblk[8]);
			RGB24BW(17*3, Yblk[9]);

			RGB24BW(8*3, Yblk[DCTSIZE2+0]);
			RGB24BW(9*3, Yblk[DCTSIZE2+1]);
			RGB24BW(24*3, Yblk[DCTSIZE2+8]);
			RGB24BW(25*3, Yblk[DCTSIZE2+9]);
		}
	}
}
#endif
int mdecFreeze(gzFile f, int Mode) {
	char Unused[4096];
#ifndef MAME_MDEC
	gzfreeze(&mdec, sizeof(mdec));
	gzfreezel(iq_y);
	gzfreezel(iq_uv);
#else
// TODO:
#endif
	gzfreezel(Unused);

	return 0;
}

