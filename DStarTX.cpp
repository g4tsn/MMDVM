/*
 *   Copyright (C) 2009-2016 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Config.h"
#include "Globals.h"
#include "DStarTX.h"

#include "DStarDefines.h"

const uint8_t BIT_SYNC = 0xAAU;

const uint8_t FRAME_SYNC[] = {0xEAU, 0xA6U, 0x00U};

// Generated using gaussfir(0.5, 4, 5) in MATLAB
static q15_t DSTAR_GMSK_FILTER[] = {8, 104, 760, 3158, 7421, 9866, 7421, 3158, 760, 104, 8, 0};
const uint16_t DSTAR_GMSK_FILTER_LEN = 12U;

q15_t DSTAR_1[] = { 1600,  1600,  1600,  1600,  1600};
q15_t DSTAR_0[] = {-1600, -1600, -1600, -1600, -1600};

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

const uint8_t INTERLEAVE_TABLE_TX[] = {
  0x00U, 0x04U, 0x04U, 0x00U, 0x07U, 0x04U, 0x0BU, 0x00U, 0x0EU, 0x04U,
  0x12U, 0x00U, 0x15U, 0x04U, 0x19U, 0x00U, 0x1CU, 0x04U, 0x20U, 0x00U,
  0x23U, 0x04U, 0x27U, 0x00U, 0x2AU, 0x04U, 0x2DU, 0x07U, 0x31U, 0x02U,
  0x34U, 0x05U, 0x38U, 0x00U, 0x3BU, 0x03U, 0x3EU, 0x06U, 0x42U, 0x01U,
  0x45U, 0x04U, 0x48U, 0x07U, 0x4CU, 0x02U, 0x4FU, 0x05U, 0x00U, 0x05U,
  0x04U, 0x01U, 0x07U, 0x05U, 0x0BU, 0x01U, 0x0EU, 0x05U, 0x12U, 0x01U,
  0x15U, 0x05U, 0x19U, 0x01U, 0x1CU, 0x05U, 0x20U, 0x01U, 0x23U, 0x05U,
  0x27U, 0x01U, 0x2AU, 0x05U, 0x2EU, 0x00U, 0x31U, 0x03U, 0x34U, 0x06U,
  0x38U, 0x01U, 0x3BU, 0x04U, 0x3EU, 0x07U, 0x42U, 0x02U, 0x45U, 0x05U,
  0x49U, 0x00U, 0x4CU, 0x03U, 0x4FU, 0x06U, 0x00U, 0x06U, 0x04U, 0x02U,
  0x07U, 0x06U, 0x0BU, 0x02U, 0x0EU, 0x06U, 0x12U, 0x02U, 0x15U, 0x06U,
  0x19U, 0x02U, 0x1CU, 0x06U, 0x20U, 0x02U, 0x23U, 0x06U, 0x27U, 0x02U,
  0x2AU, 0x06U, 0x2EU, 0x01U, 0x31U, 0x04U, 0x34U, 0x07U, 0x38U, 0x02U,
  0x3BU, 0x05U, 0x3FU, 0x00U, 0x42U, 0x03U, 0x45U, 0x06U, 0x49U, 0x01U,
  0x4CU, 0x04U, 0x4FU, 0x07U, 0x00U, 0x07U, 0x04U, 0x03U, 0x07U, 0x07U,
  0x0BU, 0x03U, 0x0EU, 0x07U, 0x12U, 0x03U, 0x15U, 0x07U, 0x19U, 0x03U,
  0x1CU, 0x07U, 0x20U, 0x03U, 0x23U, 0x07U, 0x27U, 0x03U, 0x2AU, 0x07U,
  0x2EU, 0x02U, 0x31U, 0x05U, 0x35U, 0x00U, 0x38U, 0x03U, 0x3BU, 0x06U,
  0x3FU, 0x01U, 0x42U, 0x04U, 0x45U, 0x07U, 0x49U, 0x02U, 0x4CU, 0x05U,
  0x50U, 0x00U, 0x01U, 0x00U, 0x04U, 0x04U, 0x08U, 0x00U, 0x0BU, 0x04U,
  0x0FU, 0x00U, 0x12U, 0x04U, 0x16U, 0x00U, 0x19U, 0x04U, 0x1DU, 0x00U,
  0x20U, 0x04U, 0x24U, 0x00U, 0x27U, 0x04U, 0x2BU, 0x00U, 0x2EU, 0x03U,
  0x31U, 0x06U, 0x35U, 0x01U, 0x38U, 0x04U, 0x3BU, 0x07U, 0x3FU, 0x02U,
  0x42U, 0x05U, 0x46U, 0x00U, 0x49U, 0x03U, 0x4CU, 0x06U, 0x50U, 0x01U,
  0x01U, 0x01U, 0x04U, 0x05U, 0x08U, 0x01U, 0x0BU, 0x05U, 0x0FU, 0x01U,
  0x12U, 0x05U, 0x16U, 0x01U, 0x19U, 0x05U, 0x1DU, 0x01U, 0x20U, 0x05U,
  0x24U, 0x01U, 0x27U, 0x05U, 0x2BU, 0x01U, 0x2EU, 0x04U, 0x31U, 0x07U,
  0x35U, 0x02U, 0x38U, 0x05U, 0x3CU, 0x00U, 0x3FU, 0x03U, 0x42U, 0x06U,
  0x46U, 0x01U, 0x49U, 0x04U, 0x4CU, 0x07U, 0x50U, 0x02U, 0x01U, 0x02U,
  0x04U, 0x06U, 0x08U, 0x02U, 0x0BU, 0x06U, 0x0FU, 0x02U, 0x12U, 0x06U,
  0x16U, 0x02U, 0x19U, 0x06U, 0x1DU, 0x02U, 0x20U, 0x06U, 0x24U, 0x02U,
  0x27U, 0x06U, 0x2BU, 0x02U, 0x2EU, 0x05U, 0x32U, 0x00U, 0x35U, 0x03U,
  0x38U, 0x06U, 0x3CU, 0x01U, 0x3FU, 0x04U, 0x42U, 0x07U, 0x46U, 0x02U,
  0x49U, 0x05U, 0x4DU, 0x00U, 0x50U, 0x03U, 0x01U, 0x03U, 0x04U, 0x07U,
  0x08U, 0x03U, 0x0BU, 0x07U, 0x0FU, 0x03U, 0x12U, 0x07U, 0x16U, 0x03U,
  0x19U, 0x07U, 0x1DU, 0x03U, 0x20U, 0x07U, 0x24U, 0x03U, 0x27U, 0x07U,
  0x2BU, 0x03U, 0x2EU, 0x06U, 0x32U, 0x01U, 0x35U, 0x04U, 0x38U, 0x07U,
  0x3CU, 0x02U, 0x3FU, 0x05U, 0x43U, 0x00U, 0x46U, 0x03U, 0x49U, 0x06U,
  0x4DU, 0x01U, 0x50U, 0x04U, 0x01U, 0x04U, 0x05U, 0x00U, 0x08U, 0x04U,
  0x0CU, 0x00U, 0x0FU, 0x04U, 0x13U, 0x00U, 0x16U, 0x04U, 0x1AU, 0x00U,
  0x1DU, 0x04U, 0x21U, 0x00U, 0x24U, 0x04U, 0x28U, 0x00U, 0x2BU, 0x04U,
  0x2EU, 0x07U, 0x32U, 0x02U, 0x35U, 0x05U, 0x39U, 0x00U, 0x3CU, 0x03U,
  0x3FU, 0x06U, 0x43U, 0x01U, 0x46U, 0x04U, 0x49U, 0x07U, 0x4DU, 0x02U,
  0x50U, 0x05U, 0x01U, 0x05U, 0x05U, 0x01U, 0x08U, 0x05U, 0x0CU, 0x01U,
  0x0FU, 0x05U, 0x13U, 0x01U, 0x16U, 0x05U, 0x1AU, 0x01U, 0x1DU, 0x05U,
  0x21U, 0x01U, 0x24U, 0x05U, 0x28U, 0x01U, 0x2BU, 0x05U, 0x2FU, 0x00U,
  0x32U, 0x03U, 0x35U, 0x06U, 0x39U, 0x01U, 0x3CU, 0x04U, 0x3FU, 0x07U,
  0x43U, 0x02U, 0x46U, 0x05U, 0x4AU, 0x00U, 0x4DU, 0x03U, 0x50U, 0x06U,
  0x01U, 0x06U, 0x05U, 0x02U, 0x08U, 0x06U, 0x0CU, 0x02U, 0x0FU, 0x06U,
  0x13U, 0x02U, 0x16U, 0x06U, 0x1AU, 0x02U, 0x1DU, 0x06U, 0x21U, 0x02U,
  0x24U, 0x06U, 0x28U, 0x02U, 0x2BU, 0x06U, 0x2FU, 0x01U, 0x32U, 0x04U,
  0x35U, 0x07U, 0x39U, 0x02U, 0x3CU, 0x05U, 0x40U, 0x00U, 0x43U, 0x03U,
  0x46U, 0x06U, 0x4AU, 0x01U, 0x4DU, 0x04U, 0x50U, 0x07U, 0x01U, 0x07U,
  0x05U, 0x03U, 0x08U, 0x07U, 0x0CU, 0x03U, 0x0FU, 0x07U, 0x13U, 0x03U,
  0x16U, 0x07U, 0x1AU, 0x03U, 0x1DU, 0x07U, 0x21U, 0x03U, 0x24U, 0x07U,
  0x28U, 0x03U, 0x2BU, 0x07U, 0x2FU, 0x02U, 0x32U, 0x05U, 0x36U, 0x00U,
  0x39U, 0x03U, 0x3CU, 0x06U, 0x40U, 0x01U, 0x43U, 0x04U, 0x46U, 0x07U,
  0x4AU, 0x02U, 0x4DU, 0x05U, 0x51U, 0x00U, 0x02U, 0x00U, 0x05U, 0x04U,
  0x09U, 0x00U, 0x0CU, 0x04U, 0x10U, 0x00U, 0x13U, 0x04U, 0x17U, 0x00U,
  0x1AU, 0x04U, 0x1EU, 0x00U, 0x21U, 0x04U, 0x25U, 0x00U, 0x28U, 0x04U,
  0x2CU, 0x00U, 0x2FU, 0x03U, 0x32U, 0x06U, 0x36U, 0x01U, 0x39U, 0x04U,
  0x3CU, 0x07U, 0x40U, 0x02U, 0x43U, 0x05U, 0x47U, 0x00U, 0x4AU, 0x03U,
  0x4DU, 0x06U, 0x51U, 0x01U, 0x02U, 0x01U, 0x05U, 0x05U, 0x09U, 0x01U,
  0x0CU, 0x05U, 0x10U, 0x01U, 0x13U, 0x05U, 0x17U, 0x01U, 0x1AU, 0x05U,
  0x1EU, 0x01U, 0x21U, 0x05U, 0x25U, 0x01U, 0x28U, 0x05U, 0x2CU, 0x01U,
  0x2FU, 0x04U, 0x32U, 0x07U, 0x36U, 0x02U, 0x39U, 0x05U, 0x3DU, 0x00U,
  0x40U, 0x03U, 0x43U, 0x06U, 0x47U, 0x01U, 0x4AU, 0x04U, 0x4DU, 0x07U,
  0x51U, 0x02U, 0x02U, 0x02U, 0x05U, 0x06U, 0x09U, 0x02U, 0x0CU, 0x06U,
  0x10U, 0x02U, 0x13U, 0x06U, 0x17U, 0x02U, 0x1AU, 0x06U, 0x1EU, 0x02U,
  0x21U, 0x06U, 0x25U, 0x02U, 0x28U, 0x06U, 0x2CU, 0x02U, 0x2FU, 0x05U,
  0x33U, 0x00U, 0x36U, 0x03U, 0x39U, 0x06U, 0x3DU, 0x01U, 0x40U, 0x04U,
  0x43U, 0x07U, 0x47U, 0x02U, 0x4AU, 0x05U, 0x4EU, 0x00U, 0x51U, 0x03U,
  0x02U, 0x03U, 0x05U, 0x07U, 0x09U, 0x03U, 0x0CU, 0x07U, 0x10U, 0x03U,
  0x13U, 0x07U, 0x17U, 0x03U, 0x1AU, 0x07U, 0x1EU, 0x03U, 0x21U, 0x07U,
  0x25U, 0x03U, 0x28U, 0x07U, 0x2CU, 0x03U, 0x2FU, 0x06U, 0x33U, 0x01U,
  0x36U, 0x04U, 0x39U, 0x07U, 0x3DU, 0x02U, 0x40U, 0x05U, 0x44U, 0x00U,
  0x47U, 0x03U, 0x4AU, 0x06U, 0x4EU, 0x01U, 0x51U, 0x04U, 0x02U, 0x04U,
  0x06U, 0x00U, 0x09U, 0x04U, 0x0DU, 0x00U, 0x10U, 0x04U, 0x14U, 0x00U,
  0x17U, 0x04U, 0x1BU, 0x00U, 0x1EU, 0x04U, 0x22U, 0x00U, 0x25U, 0x04U,
  0x29U, 0x00U, 0x2CU, 0x04U, 0x2FU, 0x07U, 0x33U, 0x02U, 0x36U, 0x05U,
  0x3AU, 0x00U, 0x3DU, 0x03U, 0x40U, 0x06U, 0x44U, 0x01U, 0x47U, 0x04U,
  0x4AU, 0x07U, 0x4EU, 0x02U, 0x51U, 0x05U, 0x02U, 0x05U, 0x06U, 0x01U,
  0x09U, 0x05U, 0x0DU, 0x01U, 0x10U, 0x05U, 0x14U, 0x01U, 0x17U, 0x05U,
  0x1BU, 0x01U, 0x1EU, 0x05U, 0x22U, 0x01U, 0x25U, 0x05U, 0x29U, 0x01U,
  0x2CU, 0x05U, 0x30U, 0x00U, 0x33U, 0x03U, 0x36U, 0x06U, 0x3AU, 0x01U, 
  0x3DU, 0x04U, 0x40U, 0x07U, 0x44U, 0x02U, 0x47U, 0x05U, 0x4BU, 0x00U,
  0x4EU, 0x03U, 0x51U, 0x06U, 0x02U, 0x06U, 0x06U, 0x02U, 0x09U, 0x06U,
  0x0DU, 0x02U, 0x10U, 0x06U, 0x14U, 0x02U, 0x17U, 0x06U, 0x1BU, 0x02U,
  0x1EU, 0x06U, 0x22U, 0x02U, 0x25U, 0x06U, 0x29U, 0x02U, 0x2CU, 0x06U,
  0x30U, 0x01U, 0x33U, 0x04U, 0x36U, 0x07U, 0x3AU, 0x02U, 0x3DU, 0x05U,
  0x41U, 0x00U, 0x44U, 0x03U, 0x47U, 0x06U, 0x4BU, 0x01U, 0x4EU, 0x04U,
  0x51U, 0x07U, 0x02U, 0x07U, 0x06U, 0x03U, 0x09U, 0x07U, 0x0DU, 0x03U,
  0x10U, 0x07U, 0x14U, 0x03U, 0x17U, 0x07U, 0x1BU, 0x03U, 0x1EU, 0x07U,
  0x22U, 0x03U, 0x25U, 0x07U, 0x29U, 0x03U, 0x2CU, 0x07U, 0x30U, 0x02U,
  0x33U, 0x05U, 0x37U, 0x00U, 0x3AU, 0x03U, 0x3DU, 0x06U, 0x41U, 0x01U,
  0x44U, 0x04U, 0x47U, 0x07U, 0x4BU, 0x02U, 0x4EU, 0x05U, 0x52U, 0x00U,
  0x03U, 0x00U, 0x06U, 0x04U, 0x0AU, 0x00U, 0x0DU, 0x04U, 0x11U, 0x00U,
  0x14U, 0x04U, 0x18U, 0x00U, 0x1BU, 0x04U, 0x1FU, 0x00U, 0x22U, 0x04U,
  0x26U, 0x00U, 0x29U, 0x04U, 0x2DU, 0x00U, 0x30U, 0x03U, 0x33U, 0x06U,
  0x37U, 0x01U, 0x3AU, 0x04U, 0x3DU, 0x07U, 0x41U, 0x02U, 0x44U, 0x05U,
  0x48U, 0x00U, 0x4BU, 0x03U, 0x4EU, 0x06U, 0x52U, 0x01U, 0x03U, 0x01U,
  0x06U, 0x05U, 0x0AU, 0x01U, 0x0DU, 0x05U, 0x11U, 0x01U, 0x14U, 0x05U,
  0x18U, 0x01U, 0x1BU, 0x05U, 0x1FU, 0x01U, 0x22U, 0x05U, 0x26U, 0x01U,
  0x29U, 0x05U, 0x2DU, 0x01U, 0x30U, 0x04U, 0x33U, 0x07U, 0x37U, 0x02U,
  0x3AU, 0x05U, 0x3EU, 0x00U, 0x41U, 0x03U, 0x44U, 0x06U, 0x48U, 0x01U,
  0x4BU, 0x04U, 0x4EU, 0x07U, 0x52U, 0x02U, 0x03U, 0x02U, 0x06U, 0x06U,
  0x0AU, 0x02U, 0x0DU, 0x06U, 0x11U, 0x02U, 0x14U, 0x06U, 0x18U, 0x02U,
  0x1BU, 0x06U, 0x1FU, 0x02U, 0x22U, 0x06U, 0x26U, 0x02U, 0x29U, 0x06U,
  0x2DU, 0x02U, 0x30U, 0x05U, 0x34U, 0x00U, 0x37U, 0x03U, 0x3AU, 0x06U,
  0x3EU, 0x01U, 0x41U, 0x04U, 0x44U, 0x07U, 0x48U, 0x02U, 0x4BU, 0x05U,
  0x4FU, 0x00U, 0x52U, 0x03U, 0x03U, 0x03U, 0x06U, 0x07U, 0x0AU, 0x03U,
  0x0DU, 0x07U, 0x11U, 0x03U, 0x14U, 0x07U, 0x18U, 0x03U, 0x1BU, 0x07U,
  0x1FU, 0x03U, 0x22U, 0x07U, 0x26U, 0x03U, 0x29U, 0x07U, 0x2DU, 0x03U,
  0x30U, 0x06U, 0x34U, 0x01U, 0x37U, 0x04U, 0x3AU, 0x07U, 0x3EU, 0x02U,
  0x41U, 0x05U, 0x45U, 0x00U, 0x48U, 0x03U, 0x4BU, 0x06U, 0x4FU, 0x01U,
  0x52U, 0x04U, 0x03U, 0x04U, 0x07U, 0x00U, 0x0AU, 0x04U, 0x0EU, 0x00U,
  0x11U, 0x04U, 0x15U, 0x00U, 0x18U, 0x04U, 0x1CU, 0x00U, 0x1FU, 0x04U,
  0x23U, 0x00U, 0x26U, 0x04U, 0x2AU, 0x00U, 0x2DU, 0x04U, 0x30U, 0x07U,
  0x34U, 0x02U, 0x37U, 0x05U, 0x3BU, 0x00U, 0x3EU, 0x03U, 0x41U, 0x06U,
  0x45U, 0x01U, 0x48U, 0x04U, 0x4BU, 0x07U, 0x4FU, 0x02U, 0x52U, 0x05U,
  0x03U, 0x05U, 0x07U, 0x01U, 0x0AU, 0x05U, 0x0EU, 0x01U, 0x11U, 0x05U,
  0x15U, 0x01U, 0x18U, 0x05U, 0x1CU, 0x01U, 0x1FU, 0x05U, 0x23U, 0x01U,
  0x26U, 0x05U, 0x2AU, 0x01U, 0x2DU, 0x05U, 0x31U, 0x00U, 0x34U, 0x03U,
  0x37U, 0x06U, 0x3BU, 0x01U, 0x3EU, 0x04U, 0x41U, 0x07U, 0x45U, 0x02U,
  0x48U, 0x05U, 0x4CU, 0x00U, 0x4FU, 0x03U, 0x52U, 0x06U, 0x03U, 0x06U,
  0x07U, 0x02U, 0x0AU, 0x06U, 0x0EU, 0x02U, 0x11U, 0x06U, 0x15U, 0x02U,
  0x18U, 0x06U, 0x1CU, 0x02U, 0x1FU, 0x06U, 0x23U, 0x02U, 0x26U, 0x06U,
  0x2AU, 0x02U, 0x2DU, 0x06U, 0x31U, 0x01U, 0x34U, 0x04U, 0x37U, 0x07U,
  0x3BU, 0x02U, 0x3EU, 0x05U, 0x42U, 0x00U, 0x45U, 0x03U, 0x48U, 0x06U,
  0x4CU, 0x01U, 0x4FU, 0x04U, 0x52U, 0x07U, 0x03U, 0x07U, 0x07U, 0x03U,
  0x0AU, 0x07U, 0x0EU, 0x03U, 0x11U, 0x07U, 0x15U, 0x03U, 0x18U, 0x07U,
  0x1CU, 0x03U, 0x1FU, 0x07U, 0x23U, 0x03U, 0x26U, 0x07U, 0x2AU, 0x03U
};

const uint8_t SCRAMBLE_TABLE_TX[] = {
  0x00U, 0xF7U, 0x34U, 0x09U, 0x44U, 0x46U, 0xD7U, 0x06U, 0xB3U, 0x72U,
  0xDEU, 0x42U, 0xF5U, 0xA5U, 0xD8U, 0xF1U, 0x87U, 0x7BU, 0x9AU, 0x04U,
  0x22U, 0xA3U, 0x6BU, 0x83U, 0x59U, 0x39U, 0x6FU, 0xA1U, 0xFAU, 0x52U,
  0xECU, 0xF8U, 0xC3U, 0x3DU, 0x4DU, 0x02U, 0x91U, 0xD1U, 0xB5U, 0xC1U,
  0xACU, 0x9CU, 0xB7U, 0x50U, 0x7DU, 0x29U, 0x76U, 0xFCU, 0xE1U, 0x9EU,
  0x26U, 0x81U, 0xC8U, 0xE8U, 0xDAU, 0x60U, 0x56U, 0xCEU, 0x5BU, 0xA8U,
  0xBEU, 0x14U, 0x3BU, 0xFEU, 0x70U, 0x4FU, 0x93U, 0x40U, 0x64U, 0x74U,
  0x6DU, 0x30U, 0x2BU, 0xE7U, 0x2DU, 0x54U, 0x5FU, 0x8AU, 0x1DU, 0x7FU,
  0xB8U, 0xA7U, 0x49U, 0x20U, 0x32U, 0xBAU, 0x36U, 0x98U, 0x95U, 0xF3U,
  0x06U};

const uint8_t DSTAR_HEADER = 0x00U;
const uint8_t DSTAR_DATA   = 0x01U;
const uint8_t DSTAR_EOT    = 0x02U;

CDStarTX::CDStarTX() :
m_buffer(),
m_modFilter(),
m_modState(),
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_txDelay(60U)      // 100ms
{
  ::memset(m_modState, 0x00U, 60U * sizeof(q15_t));

  m_modFilter.numTaps = DSTAR_GMSK_FILTER_LEN;
  m_modFilter.pState  = m_modState;
  m_modFilter.pCoeffs = DSTAR_GMSK_FILTER;
}

void CDStarTX::process()
{
  if (m_buffer.getData() == 0U && m_poLen == 0U)
    return;

  uint8_t type = m_buffer.peek();

  if (type == DSTAR_HEADER && m_poLen == 0U) {
    if (!m_tx) {
      for (uint16_t i = 0U; i < m_txDelay; i++)
        m_poBuffer[m_poLen++] = BIT_SYNC;
    } else {
      // Pop the type byte off
      m_buffer.get();

      uint8_t header[DSTAR_HEADER_LENGTH_BYTES];
      for (uint8_t i = 0U; i < DSTAR_HEADER_LENGTH_BYTES; i++)
        header[i] = m_buffer.get();

      uint8_t buffer[86U];
      txHeader(header, buffer + 2U);

      buffer[0U]  = FRAME_SYNC[0U];
      buffer[1U]  = FRAME_SYNC[1U];
      buffer[2U] |= FRAME_SYNC[2U];
      
      for (uint8_t i = 0U; i < 85U; i++)
        m_poBuffer[m_poLen++] = buffer[i];
    }

    m_poPtr = 0U;
  }
 
  if (type == DSTAR_DATA && m_poLen == 0U) {
    // Pop the type byte off
    m_buffer.get();

    for (uint8_t i = 0U; i < DSTAR_DATA_LENGTH_BYTES; i++)
      m_poBuffer[m_poLen++] = m_buffer.get();

    m_poPtr = 0U;
  }

  if (type == DSTAR_EOT && m_poLen == 0U) {
    // Pop the type byte off
    m_buffer.get();

    for (uint8_t j = 0U; j < 3U; j++) {
      for (uint8_t i = 0U; i < DSTAR_EOT_LENGTH_BYTES; i++)
        m_poBuffer[m_poLen++] = DSTAR_EOT_BYTES[i];
    }
     
    m_poPtr = 0U;
  }

  if (m_poLen > 0U) {
    uint16_t space = io.getSpace();
    
    while (space > (8U * DSTAR_RADIO_BIT_LENGTH) && space < 1000U) {
      uint8_t c = m_poBuffer[m_poPtr++];
      writeByte(c);

      space -= 8U * DSTAR_RADIO_BIT_LENGTH;
      
      if (m_poPtr >= m_poLen) {
        m_poPtr = 0U;
        m_poLen = 0U;
        return;
      }
    }
  }
}

uint8_t CDStarTX::writeHeader(const uint8_t* header, uint8_t length)
{
  if (length != DSTAR_HEADER_LENGTH_BYTES)
    return 4U;

  uint16_t space = m_buffer.getSpace();
  if (space < (DSTAR_HEADER_LENGTH_BYTES + 1U))
    return 5U;

  m_buffer.put(DSTAR_HEADER);

  for (uint8_t i = 0U; i < DSTAR_HEADER_LENGTH_BYTES; i++)
    m_buffer.put(header[i]);
    
  return 0U;
}

uint8_t CDStarTX::writeData(const uint8_t* data, uint8_t length)
{
  if (length != DSTAR_DATA_LENGTH_BYTES)
    return 4U;

  uint16_t space = m_buffer.getSpace();
  if (space < (DSTAR_DATA_LENGTH_BYTES + 1U))
    return 5U;

  m_buffer.put(DSTAR_DATA);

  for (uint8_t i = 0U; i < DSTAR_DATA_LENGTH_BYTES; i++)
    m_buffer.put(data[i]);
    
  return 0U;
}

uint8_t CDStarTX::writeEOT()
{
  uint16_t space = m_buffer.getSpace();
  if (space < 1U)
    return 5U;

  m_buffer.put(DSTAR_EOT);

  return 0U;
}

void CDStarTX::txHeader(const uint8_t* in, uint8_t* out) const
{
  uint8_t intermediate[84U];
  uint32_t i;

  for (i = 0U; i < 83U; i++) {
    intermediate[i] = 0x00U;
    out[i] = 0x00U;
  }

  // Convolve the header
  uint8_t d, d1 = 0U, d2 = 0U, g0, g1;
  uint32_t k = 0U;
  for (i = 0U; i < 42U; i++) {
    for (uint8_t j = 0U; j < 8U; j++) {
      uint8_t mask = (0x01U << j);
      d = 0U;

      if (in[i] & mask)
        d = 1U;

      g0 = (d + d2) & 1;
      g1 = (d + d1 + d2) & 1;
      d2 = d1;
      d1 = d;

      if (g1)
        intermediate[k >> 3] |= BIT_MASK_TABLE[k & 7];
      k++;

      if (g0)
        intermediate[k >> 3] |= BIT_MASK_TABLE[k & 7];
      k++;
    }
  }

  // Interleave the header
  i = 0U;
  while (i < 660U) {
    unsigned char d = intermediate[i >> 3];

    if (d & 0x80U)
      out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
    i++;

    if (d & 0x40U)
      out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
    i++;

    if (d & 0x20U)
      out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
    i++;

    if (d & 0x10U)
      out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
    i++;

    if (i < 660U) {
      if (d & 0x08U)
        out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
        i++;

      if (d & 0x04U)
        out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
      i++;

      if (d & 0x02U)
        out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
      i++;

      if (d & 0x01U)
        out[INTERLEAVE_TABLE_TX[i * 2U]] |= (0x01U << INTERLEAVE_TABLE_TX[i * 2U + 1U]);
      i++;
    }
  }

  // Scramble the header
  for (i = 0U; i < 83U; i++)
    out[i] ^= SCRAMBLE_TABLE_TX[i];
}

void CDStarTX::writeByte(uint8_t c)
{
  q15_t inBuffer[DSTAR_RADIO_BIT_LENGTH * 8U];
  q15_t outBuffer[DSTAR_RADIO_BIT_LENGTH * 8U];

  uint8_t mask = 0x01U;

  q15_t* p = inBuffer;
  for (uint8_t i = 0U; i < 8U; i++, p += DSTAR_RADIO_BIT_LENGTH) {
    if ((c & mask) == mask)
      ::memcpy(p, DSTAR_0, DSTAR_RADIO_BIT_LENGTH * sizeof(q15_t));
    else
      ::memcpy(p, DSTAR_1, DSTAR_RADIO_BIT_LENGTH * sizeof(q15_t));

    mask <<= 1;
  }

  ::arm_fir_fast_q15(&m_modFilter, inBuffer, outBuffer, DSTAR_RADIO_BIT_LENGTH * 8U);

  io.write(outBuffer, DSTAR_RADIO_BIT_LENGTH * 8U);
}

void CDStarTX::setTXDelay(uint8_t delay)
{
  m_txDelay = 60U + uint16_t(delay) * 6U;        // 100ms + tx delay
}

uint16_t CDStarTX::getSpace() const
{
  return m_buffer.getSpace() / (DSTAR_DATA_LENGTH_BYTES + 1U);
}
