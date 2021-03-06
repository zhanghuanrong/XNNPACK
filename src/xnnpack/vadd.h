// Copyright (c) Facebook, Inc. and its affiliates.
// All rights reserved.
//
// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <xnnpack/params.h>
#include <xnnpack/common.h>

#ifdef __cplusplus
extern "C" {
#endif


#define DECLARE_QU8_VADD_MINMAX_UKERNEL_FUNCTION(fn_name) \
  XNN_INTERNAL void fn_name(                              \
      size_t n,                                           \
      const uint8_t* a,                                   \
      const uint8_t* b,                                   \
      uint8_t* y,                                         \
      const union xnn_qu8_add_params* params);

DECLARE_QU8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qu8_vadd_minmax_ukernel__neon)
DECLARE_QU8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qu8_vadd_minmax_ukernel__scalar)
DECLARE_QU8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qu8_vadd_minmax_ukernel__sse2)


#define DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(fn_name) \
  XNN_INTERNAL void fn_name(                              \
      size_t n,                                           \
      const int8_t* input_x,                              \
      const int8_t* input_y,                              \
      int8_t* output,                                     \
      const union xnn_qs8_add_params* params);

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__neon_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__neon_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__neon_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__neon_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse2_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse2_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse2_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse2_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__sse41_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__xop_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__xop_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__xop_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__xop_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx2_mul32_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx2_mul32_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx2_mul32_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__avx2_mul32_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__wasmsimd_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__wasmsimd_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__wasmsimd_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vadd_minmax_ukernel__wasmsimd_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__neon_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__neon_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__neon_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__neon_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse2_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse2_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse2_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse2_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul16_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul16_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul16_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul16_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__sse41_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__xop_mul32_ld32_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__xop_mul32_ld32_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__xop_mul32_ld32_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__xop_mul32_ld32_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx2_mul32_ld64_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx2_mul32_ld64_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx2_mul32_ld64_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__avx2_mul32_ld64_x32)

DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__wasmsimd_x8)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__wasmsimd_x16)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__wasmsimd_x24)
DECLARE_QS8_VADD_MINMAX_UKERNEL_FUNCTION(xnn_qs8_vaddc_minmax_ukernel__wasmsimd_x32)


#ifdef __cplusplus
}  // extern "C"
#endif
