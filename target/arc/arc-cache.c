/*
 * QEMU ARC CPU
 *
 * Copyright (c) 2019 Synopsys, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "cpu.h"
#include "arc-regs.h"
#include "arc-cache.h"

void arc_cache_aux_set(struct arc_aux_reg_detail *aux_reg_detail,
                       uint32_t val, void *data)
{

    CPUARCState *env = (CPUARCState *) data;
    struct arc_cache *cache = &env->cache;

    switch (aux_reg_detail->id) {
    case AUX_ID_ic_ivic:
    case AUX_ID_ic_ivil:
    case AUX_ID_dc_ivdc:
    case AUX_ID_dc_ivdl:
    case AUX_ID_dc_flsh:
    case AUX_ID_dc_fldl:
    case AUX_ID_dc_startr:
       /* Do nothing as we don't simulate cache memories */
       break;

    case AUX_ID_ic_ctrl:
        cache->ic_disabled = val & 1;
        break;

    case AUX_ID_ic_ivir:
        cache->ic_ivir = val & 0xffffff00;
        break;

    case AUX_ID_ic_endr:
        cache->ic_endr = val & 0xffffff00;
        break;

    case AUX_ID_ic_ptag:
        cache->ic_ptag = val;
        break;

    case AUX_ID_ic_ptag_hi:
        cache->ic_ptag_hi = val & 0xff;
        break;

    case AUX_ID_dc_ctrl:
        /* DC - Disable Cache: Enables/Disables the cache: 0 - Enabled, 1 - Disabled */
        cache->dc_disabled = val & 1;
        /* IM - Invalidate Mode: Selects the invalidate type */
        cache->dc_inv_mode = (val >> 6) & 1;
        break;

    case AUX_ID_dc_endr:
        cache->dc_endr = val & 0xffffff00;
        break;

    case AUX_ID_dc_ptag_hi:
        cache->dc_ptag_hi = val & 0xff;
        break;

    default:
        hw_error("%s@%d: Attempt to write read-only register 0x%02x!\n",
                 __func__, __LINE__, (unsigned int)aux_reg_detail->id);
        break;
    }

    return;
}

uint32_t arc_cache_aux_get(struct arc_aux_reg_detail *aux_reg_detail,
                           void *data)
{
    CPUARCState *env = (CPUARCState *) data;
    struct arc_cache *cache = &env->cache;
    uint32_t reg = 0;

    switch (aux_reg_detail->id) {
    case AUX_ID_i_cache_build:
        reg = (0 << 22) | /* D - indicates that IC is disabled on reset */
              (2 << 20) | /* FL - Feature level: 10b - line lock, invalidate, advanced debug features */
              (3 << 16) | /* BSize - indicates the cache block size in bytes: 0011b - 64 bytes */
              (7 << 12) | /* Cache capacity: 0111b - 64 Kbytes */
              (2 << 8)  | /* Cache Associativiy: 0010b - Four-way set associative */
              (4 << 0);   /* Version number: 4 - ARCv2 */
        break;

    case AUX_ID_ic_ctrl:
        reg = cache->ic_disabled & 1;
        break;

    case AUX_ID_ic_ivir:
        reg = cache->ic_ivir;
        break;

    case AUX_ID_ic_endr:
        reg = cache->ic_endr;
        break;

    case AUX_ID_ic_ptag:
        reg = cache->ic_ptag;
        break;

    case AUX_ID_ic_ptag_hi:
        reg = cache->ic_ptag_hi;
        break;

    case AUX_ID_d_cache_build:
        reg = (2 << 20) | /* FL - Feature level: 10b - line lock, invalidate, advanced debug features */
              (2 << 16) | /* BSize - indicates the cache block size in bytes: 0010b - 64 bytes */
              (7 << 12) | /* Cache capacity: 0111b - 64 Kbytes */
              (1 << 8)  | /* Cache Associativiy: 0001b - Two-way set associative */
              (4 << 0);   /* Version number: 4 - ARCv2 with fixed number of cycles */
        break;

    case AUX_ID_dc_ctrl:
             /* DC - Disable Cache: Enables/Disables the cache: 0 - Enabled, 1 - Disabled */
       reg = (cache->dc_disabled & 1) << 0 |
             /* SB - Success Bit: Success of last cache operation: 1 - succeded (immediately) */
             (1 << 2) |
             /* IM - Invalidate Mode: Selects the invalidate type */
             (cache->dc_inv_mode & 1) << 6;
        break;

    case AUX_ID_dc_endr:
        reg = cache->dc_endr;
        break;

    case AUX_ID_dc_ptag_hi:
        reg = cache->dc_ptag_hi;
        break;

    default:
        hw_error("%s@%d: Attempt to read write-only register 0x%02x!\n",
                 __func__, __LINE__, (unsigned int)aux_reg_detail->id);
        break;
    }

    return reg;
}

