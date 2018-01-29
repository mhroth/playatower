/**
 * Copyright (c) 2017-2018, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <arm_neon.h>
#include <math.h>

#include "PixelBuffer.hpp"

// out = in**2.8
static const uint8_t APA102_GAMMA[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

static inline float _clamp(float x) {
  return fminf(1.0f, fmaxf(0.0f, x));
}

PixelBuffer::PixelBuffer(uint32_t numLeds, uint8_t global) :
    numLeds(numLeds), global(global) {

  // RGB buffer. Order is global, blue, green, red (same as APA-102 datastream)
  rgb = (float *) malloc(4 * numLeds * sizeof(float));
  assert(rgb != nullptr);

  // prepare SPI data
  // https://cpldcpu.com/2014/11/30/understanding-the-apa102-superled/
  // NOTE(mhroth): because getSpiBytes() processes 4 LEDS at a time (i.e. 16-byte output),
  // and so that no memory outside of spi_data will be overwritten,
  // ((4*numLeds) + numSpiTrailerBytes) must be positive mulitple of 16.
  numSpiTrailerBytes = (numLeds >> 4) + 1;
  numSpiBytes = 4 + (4*numLeds) + numSpiTrailerBytes;
  // ensure that it is the next largest multiple-of-16 (if necessary)
  numSpiBytesTotal = (numSpiBytes + 15) & ~0xF;
  spi_data = (uint8_t *) malloc(numSpiBytesTotal * sizeof(uint8_t));
  assert(spi_data != nullptr);

  // reset all buffers
  clear();
}

PixelBuffer::~PixelBuffer() {
  free(rgb);
  free(spi_data);
}

float PixelBuffer::getAmperes() {
  float a = 0.0f;
  const int N = 3 * numLeds;
  for (int i = 0; i < N; ++i) {
    a += _clamp(rgb[i]);
  }
  a *= 0.02f; // 20mA per color LED
  a *= (global/31.0f); // apply global brightness setting
  return a;
}

void PixelBuffer::clear() {
  // reset RGB buffer
  memset(rgb, 0, 4*numLeds*sizeof(float));

  // reset SPI buffer
  memset(spi_data, 0, numSpiBytesTotal); // leading zeros
}

// https://gist.github.com/paulkaplan/5184275
// http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
static void __kelvin_to_rgb(float kelvin, float *r, float *g, float *b) {
  // TODO(mhroth): update the constants to scale to [0,1], not [0,255]
  float temp = kelvin / 100.0f;
  float _r, _g, _b;
  if (temp <= 66) {
    _r = 255.0f;
    _g = 99.4708025861f * logf(temp) - 161.1195681661f;
    _b = (temp <= 19.0f) ? 0.0f : 138.5177312231f * logf(temp-10.0f) - 305.0447927307f;
  } else {
    _r = 329.698727446f * powf(temp-60.0f, -0.1332047592f);
    _g = 288.1221695283f * powf(temp-60.0f, -0.0755148492f);
    _b = 255.0f;
  }

  *r = fmaxf(0.0f, fminf(1.0f, _r/255.0f));
  *g = fmaxf(0.0f, fminf(1.0f, _g/255.0f));
  *b = fmaxf(0.0f, fminf(1.0f, _b/255.0f));
}

void PixelBuffer::fill_rgb(float r, float g, float b) {
  for (int i = 0; i < numLeds; i++) {
    set_pixel_rgb_blend(i, r, g, b);
  }
}

void PixelBuffer::apply_gain(float f) {
  for (int i = 0, j = 0; i < numLeds; ++i, j+=4) {
    float32x4_t x = vld1q_f32(rgb+j);
    x = vmulq_n_f32(x, f);
    vst1q_f32(rgb+j, x);
  }
/*
  const int n = numLeds * 4;
  for (int i = 0; i < n; ++i) {
    rgb[i] *= f;
  }
*/
}

uint8_t *PixelBuffer::getSpiBytes(float n) {
  const float32x4_t CLAMP_ONE = vdupq_n_f32(1.0f);
  const float32x4_t CLAMP_ZERO = vdupq_n_f32(0.0f);

  // nightshift
  float rw, gw, bw;
  __kelvin_to_rgb(6600.0f*(1.0f-n), &rw, &gw, &bw);
  const float32x4_t ns = (float32x4_t) {
      0.0f, // global
      bw,   // blue
      gw,   // green
      rw    // red
  };
  const uint8_t G = 0xE0 | global;
  const uint8x16_t GLOBAL = (uint8x16_t) {G,0,0,0,G,0,0,0,G,0,0,0,G,0,0,0};
  for (int i = 0, j = 0; i < numLeds; i+=4, j+=16) {
    float32x4_t x = vld1q_f32(rgb+j);
    x = vmaxq_f32(x, CLAMP_ZERO); // clamp to [0,1]
    x = vminq_f32(x, CLAMP_ONE);
    x = vmulq_f32(x, ns); // apply nightshift
    // apply gamma adjustment (based on lookup table)
    // NOTE(mhroth): original LUT is roughly x**2.8 (==14/5). In this case we calculate x**3 as it is much
    // easier and faster. The deviation is at most 7 steps darker.
    x = vmulq_f32(x, vmulq_f32(x, x));
    x = vmulq_n_f32(x, 255.0f);
    uint16x4_t x_u16 = vmovn_u32(vcvtq_u32_f32(x));

    float32x4_t y = vld1q_f32(rgb+j+4);
    y = vmaxq_f32(vminq_f32(y, CLAMP_ONE), CLAMP_ZERO);
    y = vmulq_f32(y, ns);
    y = vmulq_f32(y, vmulq_f32(y, y));
    y = vmulq_n_f32(y, 255.0f);
    uint16x4_t y_u16 = vmovn_u32(vcvtq_u32_f32(y));

    uint8x8_t z_u8 = vmovn_u16(vcombine_u16(x_u16, y_u16));

    float32x4_t a = vld1q_f32(rgb+j+8);
    a = vmaxq_f32(vminq_f32(a, CLAMP_ONE), CLAMP_ZERO);
    a = vmulq_f32(a, ns);
    a = vmulq_f32(a, vmulq_f32(a, a));
    a = vmulq_n_f32(a, 255.0f);
    uint16x4_t a_u16 = vmovn_u32(vcvtq_u32_f32(a));

    float32x4_t b = vld1q_f32(rgb+j+12);
    b = vmaxq_f32(vminq_f32(b, CLAMP_ONE), CLAMP_ZERO);
    b = vmulq_f32(b, ns);
    b = vmulq_f32(b, vmulq_f32(b, b));
    b = vmulq_n_f32(b, 255.0f);
    uint16x4_t b_u16 = vmovn_u32(vcvtq_u32_f32(b));

    uint8x8_t c_u8 = vmovn_u16(vcombine_u16(a_u16, b_u16));

    uint8x16_t d_u8 = vorrq_u8(vcombine_u8(z_u8, c_u8), GLOBAL); // add global value into bytestream

    // write to the spi_data buffer
    vst1q_u8(spi_data+4+j, d_u8);
  }

  // clear trailing bytes, as above loop may have overwriten some
  memset(spi_data + (4*(numLeds+1)), 0xFF, numSpiTrailerBytes);

/*
  const float n_b = 1.0f - n*0.75f;
  const float n_g = 1.0f - n*0.60f;
  for (int i = 0; i < numLeds; ++i) {
    int j = (i+1)*4;
    spi_data[j+0] = 0xE0 | global;
    spi_data[j+1] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*4+1]) * n_b * 255.0f)]; // blue
    spi_data[j+2] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*4+2]) * n_g * 255.0f)]; // green
    spi_data[j+3] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*4+3]) * 255.0f)]; // red
  }
*/
  return spi_data;
}

void PixelBuffer::set_pixel_rgb_blend(int i, float r, float g, float b, float a, BlendMode mode) {
  assert(i >= 0 && i < numLeds);
  const int j = 4 * i;

  switch (mode) {
    default:
    case SET: {
      rgb[j+3] = r; rgb[j+2] = g; rgb[j+1] = b;
      break;
    }
    case ADD: {
      const float z = 1.0f - a;
      rgb[j+3] = a*r + z*rgb[j+3]; rgb[j+2] = a*g + z*rgb[j+2]; rgb[j+1] = a*b + z*rgb[j+1];
      break;
    }
    case ACCUMULATE: {
      rgb[j+3] += a*r; rgb[j+2] += a*g; rgb[j+1] += a*b;
      break;
    }
    case DIFFERENCE: {
      set_pixel_rgb_blend(i, fabsf(r-rgb[j+3]), fabsf(g-rgb[j+2]), fabsf(b-rgb[j+1]), a, BlendMode::ADD);
      break;
    }
    case MULTIPLY: {
      set_pixel_rgb_blend(i, rgb[j+3]*r, rgb[j+2]*g, rgb[j+1]*g, a, BlendMode::ADD);
      break;
    }
    case SCREEN: {
      rgb[j+3] = 1.0f-(r*rgb[j+3]); rgb[j+2] = 1.0f-(g*rgb[j+2]); rgb[j+1] = 1.0f-(b*rgb[j+1]);
      break;
    }
  }
}

// http://www.rapidtables.com/convert/color/hsl-to-rgb.htm
void PixelBuffer::set_pixel_hsl_blend(int i, float h, float s, float l, float a, BlendMode mode) {
  // wrap hue around [0,360]
  if (h < 0.0f) h += 360.0f;
  else if (h > 360.0f) h -= 360.0f;
  s = fmaxf(0.0f, fminf(1.0f, s));
  l = fmaxf(0.0f, fminf(1.0f, l));

  float C = (1.0f - fabsf(2.0f*l-1.0f)) * s;
  float X = ((((int) (h/60.0f)) % 2) == 0) ? C : 0.0f;
  float m = l - C*0.5f;

  float r, g, b;
  if (h < 60.0f) {
    r = C+m; g = X+m; b = 0.0f+m;
  } else if (h < 120.0f) {
    r = X+m; g = C+m; b = 0.0f+m;
  } else if (h < 180.0f) {
    r = 0.0f+m; g = C+m; b = X+m;
  } else if (h < 240.0f) {
    r = 0.0f+m; g = X+m; b = C+m;
  } else if (h < 300.0f) {
    r = X+m; g = 0.0f+m; b = C+m;
  } else {
    r = C+m; g = 0.0f+m; b = X+m;
  }

  set_pixel_rgb_blend(i, r, g, b, a, mode);
}
