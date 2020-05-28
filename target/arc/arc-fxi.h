/* Extract functions for the ARC opcodes.
   Copyright (C) 2015-2017 Free Software Foundation, Inc.

   Contributed by Claudiu Zissulescu (claziss@synopsys.com)

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef EXTRACT_LIMM
#define EXTRACT_LIMM
/* mask = 00000000000000000000000000000000.  */
static ATTRIBUTE_UNUSED int
extract_limm (unsigned long long insn ATTRIBUTE_UNUSED, bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  return value;
}
#endif /* EXTRACT_LIMM */

#ifndef EXTRACT_UIMM6_20
#define EXTRACT_UIMM6_20
/* mask = 00000000000000000000111111000000.  */
static long long int
extract_uimm6_20 (unsigned long long insn ATTRIBUTE_UNUSED,
		  bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 6) & 0x003f) << 0;

  return value;
}
#endif /* EXTRACT_UIMM6_20 */

#ifndef EXTRACT_SIMM12_20
#define EXTRACT_SIMM12_20
/* mask = 00000000000000000000111111222222.  */
static long long int
extract_simm12_20 (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 6) & 0x003f) << 0;
  value |= ((insn >> 0) & 0x003f) << 6;

  /* Extend the sign.  */
  int signbit = 1 << (12 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM12_20 */

#ifndef EXTRACT_SIMM3_5_S
#define EXTRACT_SIMM3_5_S
/* mask = 0000011100000000.  */
static ATTRIBUTE_UNUSED int
extract_simm3_5_s (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 8) & 0x0007) << 0;

  /* Extend the sign.  */
  int signbit = 1 << (3 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM3_5_S */

#ifndef EXTRACT_LIMM_S
#define EXTRACT_LIMM_S
/* mask = 0000000000000000.  */
static ATTRIBUTE_UNUSED int
extract_limm_s (unsigned long long insn ATTRIBUTE_UNUSED, bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  return value;
}
#endif /* EXTRACT_LIMM_S */

#ifndef EXTRACT_UIMM7_A32_11_S
#define EXTRACT_UIMM7_A32_11_S
/* mask = 0000000000011111.  */
static long long int
extract_uimm7_a32_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x001f) << 2;

  return value;
}
#endif /* EXTRACT_UIMM7_A32_11_S */

#ifndef EXTRACT_UIMM7_9_S
#define EXTRACT_UIMM7_9_S
/* mask = 0000000001111111.  */
static long long int
extract_uimm7_9_s (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x007f) << 0;

  return value;
}
#endif /* EXTRACT_UIMM7_9_S */

#ifndef EXTRACT_UIMM3_13_S
#define EXTRACT_UIMM3_13_S
/* mask = 0000000000000111.  */
static long long int
extract_uimm3_13_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x0007) << 0;

  return value;
}
#endif /* EXTRACT_UIMM3_13_S */

#ifndef EXTRACT_SIMM11_A32_7_S
#define EXTRACT_SIMM11_A32_7_S
/* mask = 0000000111111111.  */
static long long int
extract_simm11_a32_7_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x01ff) << 2;

  /* Extend the sign.  */
  int signbit = 1 << (11 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM11_A32_7_S */

#ifndef EXTRACT_UIMM6_13_S
#define EXTRACT_UIMM6_13_S
/* mask = 0000000002220111.  */
static long long int
extract_uimm6_13_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x0007) << 0;
  value |= ((insn >> 4) & 0x0007) << 3;

  return value;
}
#endif /* EXTRACT_UIMM6_13_S */

#ifndef EXTRACT_UIMM5_11_S
#define EXTRACT_UIMM5_11_S
/* mask = 0000000000011111.  */
static long long int
extract_uimm5_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x001f) << 0;

  return value;
}
#endif /* EXTRACT_UIMM5_11_S */

#ifndef EXTRACT_SIMM9_A16_8
#define EXTRACT_SIMM9_A16_8
/* mask = 00000000111111102000000000000000.  */
static long long int
extract_simm9_a16_8 (unsigned long long insn ATTRIBUTE_UNUSED,
		     bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 17) & 0x007f) << 1;
  value |= ((insn >> 15) & 0x0001) << 8;

  /* Extend the sign.  */
  int signbit = 1 << (9 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM9_A16_8 */

#ifndef EXTRACT_UIMM6_8
#define EXTRACT_UIMM6_8
/* mask = 00000000000000000000111111000000.  */
static long long int
extract_uimm6_8 (unsigned long long insn ATTRIBUTE_UNUSED,
		 bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 6) & 0x003f) << 0;

  return value;
}
#endif /* EXTRACT_UIMM6_8 */

#ifndef EXTRACT_SIMM21_A16_5
#define EXTRACT_SIMM21_A16_5
/* mask = 00000111111111102222222222000000.  */
static long long int
extract_simm21_a16_5 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 17) & 0x03ff) << 1;
  value |= ((insn >> 6) & 0x03ff) << 11;

  /* Extend the sign.  */
  int signbit = 1 << (21 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM21_A16_5 */

#ifndef EXTRACT_SIMM25_A16_5
#define EXTRACT_SIMM25_A16_5
/* mask = 00000111111111102222222222003333.  */
static long long int
extract_simm25_a16_5 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 17) & 0x03ff) << 1;
  value |= ((insn >> 6) & 0x03ff) << 11;
  value |= ((insn >> 0) & 0x000f) << 21;

  /* Extend the sign.  */
  int signbit = 1 << (25 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM25_A16_5 */

#ifndef EXTRACT_SIMM10_A16_7_S
#define EXTRACT_SIMM10_A16_7_S
/* mask = 0000000111111111.  */
static long long int
extract_simm10_a16_7_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x01ff) << 1;

  /* Extend the sign.  */
  int signbit = 1 << (10 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM10_A16_7_S */

#ifndef EXTRACT_SIMM7_A16_10_S
#define EXTRACT_SIMM7_A16_10_S
/* mask = 0000000000111111.  */
static long long int
extract_simm7_a16_10_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x003f) << 1;

  /* Extend the sign.  */
  int signbit = 1 << (7 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM7_A16_10_S */

#ifndef EXTRACT_SIMM21_A32_5
#define EXTRACT_SIMM21_A32_5
/* mask = 00000111111111002222222222000000.  */
static long long int
extract_simm21_a32_5 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 18) & 0x01ff) << 2;
  value |= ((insn >> 6) & 0x03ff) << 11;

  /* Extend the sign.  */
  int signbit = 1 << (21 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM21_A32_5 */

#ifndef EXTRACT_SIMM25_A32_5
#define EXTRACT_SIMM25_A32_5
/* mask = 00000111111111002222222222003333.  */
static long long int
extract_simm25_a32_5 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 18) & 0x01ff) << 2;
  value |= ((insn >> 6) & 0x03ff) << 11;
  value |= ((insn >> 0) & 0x000f) << 21;

  /* Extend the sign.  */
  int signbit = 1 << (25 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM25_A32_5 */

#ifndef EXTRACT_SIMM13_A32_5_S
#define EXTRACT_SIMM13_A32_5_S
/* mask = 0000011111111111.  */
static long long int
extract_simm13_a32_5_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x07ff) << 2;

  /* Extend the sign.  */
  int signbit = 1 << (13 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM13_A32_5_S */

#ifndef EXTRACT_SIMM8_A16_9_S
#define EXTRACT_SIMM8_A16_9_S
/* mask = 0000000001111111.  */
static long long int
extract_simm8_a16_9_s (unsigned long long insn ATTRIBUTE_UNUSED,
		       bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x007f) << 1;

  /* Extend the sign.  */
  int signbit = 1 << (8 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM8_A16_9_S */

#ifndef EXTRACT_UIMM3_23
#define EXTRACT_UIMM3_23
/* mask = 00000000000000000000000111000000.  */
static long long int
extract_uimm3_23 (unsigned long long insn ATTRIBUTE_UNUSED,
		  bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 6) & 0x0007) << 0;

  return value;
}
#endif /* EXTRACT_UIMM3_23 */

#ifndef EXTRACT_UIMM10_6_S
#define EXTRACT_UIMM10_6_S
/* mask = 0000001111111111.  */
static long long int
extract_uimm10_6_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x03ff) << 0;

  return value;
}
#endif /* EXTRACT_UIMM10_6_S */

#ifndef EXTRACT_UIMM6_11_S
#define EXTRACT_UIMM6_11_S
/* mask = 0000002200011110.  */
static long long int
extract_uimm6_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 1) & 0x000f) << 0;
  value |= ((insn >> 8) & 0x0003) << 4;

  return value;
}
#endif /* EXTRACT_UIMM6_11_S */

#ifndef EXTRACT_SIMM9_8
#define EXTRACT_SIMM9_8
/* mask = 00000000111111112000000000000000.  */
static long long int
extract_simm9_8 (unsigned long long insn ATTRIBUTE_UNUSED,
		 bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 16) & 0x00ff) << 0;
  value |= ((insn >> 15) & 0x0001) << 8;

  /* Extend the sign.  */
  int signbit = 1 << (9 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM9_8 */

#ifndef EXTRACT_UIMM10_A32_8_S
#define EXTRACT_UIMM10_A32_8_S
/* mask = 0000000011111111.  */
static long long int
extract_uimm10_a32_8_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x00ff) << 2;

  return value;
}
#endif /* EXTRACT_UIMM10_A32_8_S */

#ifndef EXTRACT_SIMM9_7_S
#define EXTRACT_SIMM9_7_S
/* mask = 0000000111111111.  */
static long long int
extract_simm9_7_s (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x01ff) << 0;

  /* Extend the sign.  */
  int signbit = 1 << (9 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM9_7_S */

#ifndef EXTRACT_UIMM6_A16_11_S
#define EXTRACT_UIMM6_A16_11_S
/* mask = 0000000000011111.  */
static long long int
extract_uimm6_a16_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x001f) << 1;

  return value;
}
#endif /* EXTRACT_UIMM6_A16_11_S */

#ifndef EXTRACT_UIMM5_A32_11_S
#define EXTRACT_UIMM5_A32_11_S
/* mask = 0000020000011000.  */
static long long int
extract_uimm5_a32_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
			bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 3) & 0x0003) << 2;
  value |= ((insn >> 10) & 0x0001) << 4;

  return value;
}
#endif /* EXTRACT_UIMM5_A32_11_S */

#ifndef EXTRACT_SIMM11_A32_13_S
#define EXTRACT_SIMM11_A32_13_S
/* mask = 0000022222200111.  */
static long long int
extract_simm11_a32_13_s (unsigned long long insn ATTRIBUTE_UNUSED,
			 bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 0) & 0x0007) << 2;
  value |= ((insn >> 5) & 0x003f) << 5;

  /* Extend the sign.  */
  int signbit = 1 << (11 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM11_A32_13_S */

#ifndef EXTRACT_UIMM7_13_S
#define EXTRACT_UIMM7_13_S
/* mask = 0000000022220111.  */
static long long int
extract_uimm7_13_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x0007) << 0;
  value |= ((insn >> 4) & 0x000f) << 3;

  return value;
}
#endif /* EXTRACT_UIMM7_13_S */

#ifndef EXTRACT_UIMM6_A16_21
#define EXTRACT_UIMM6_A16_21
/* mask = 00000000000000000000011111000000.  */
static long long int
extract_uimm6_a16_21 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 6) & 0x001f) << 1;

  return value;
}
#endif /* EXTRACT_UIMM6_A16_21 */

#ifndef EXTRACT_UIMM7_11_S
#define EXTRACT_UIMM7_11_S
/* mask = 0000022200011110.  */
static long long int
extract_uimm7_11_s (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 1) & 0x000f) << 0;
  value |= ((insn >> 8) & 0x0007) << 4;

  return value;
}
#endif /* EXTRACT_UIMM7_11_S */

#ifndef EXTRACT_UIMM7_A16_20
#define EXTRACT_UIMM7_A16_20
/* mask = 00000000000000000000111111000000.  */
static long long int
extract_uimm7_a16_20 (unsigned long long insn ATTRIBUTE_UNUSED,
		      bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 6) & 0x003f) << 1;

  return value;
}
#endif /* EXTRACT_UIMM7_A16_20 */

#ifndef EXTRACT_SIMM13_A16_20
#define EXTRACT_SIMM13_A16_20
/* mask = 00000000000000000000111111222222.  */
static long long int
extract_simm13_a16_20 (unsigned long long insn ATTRIBUTE_UNUSED,
		       bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  int value = 0;

  value |= ((insn >> 6) & 0x003f) << 1;
  value |= ((insn >> 0) & 0x003f) << 7;

  /* Extend the sign.  */
  int signbit = 1 << (13 - 1);
  value = (value ^ signbit) - signbit;

  return value;
}
#endif /* EXTRACT_SIMM13_A16_20 */

#ifndef EXTRACT_UIMM8_8_S
#define EXTRACT_UIMM8_8_S
/* mask = 0000000011111111.  */
static long long int
extract_uimm8_8_s (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 0) & 0x00ff) << 0;

  return value;
}
#endif /* EXTRACT_UIMM8_8_S */

#ifndef EXTRACT_UIMM6_5_S
#define EXTRACT_UIMM6_5_S
/* mask = 0000011111100000.  */
static long long int
extract_uimm6_5_s (unsigned long long insn ATTRIBUTE_UNUSED,
		   bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  value |= ((insn >> 5) & 0x003f) << 0;

  return value;
}
#endif /* EXTRACT_UIMM6_5_S */

#ifndef EXTRACT_UIMM6_AXX_
#define EXTRACT_UIMM6_AXX_
/* mask = 00000000000000000000000000000000.  */
static ATTRIBUTE_UNUSED int
extract_uimm6_axx_ (unsigned long long insn ATTRIBUTE_UNUSED,
		    bfd_boolean * invalid ATTRIBUTE_UNUSED)
{
  unsigned value = 0;

  return value;
}
#endif /* EXTRACT_UIMM6_AXX_ */
