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

PixelBuffer::PixelBuffer(uint32_t numLeds) {
  m_numLeds = numLeds;
  m_global = 1.0f;
  m_ampLimit = INFINITY;

  // RGB buffer. Order is global, blue, green, red (same as APA-102 datastream)
  // NOTE(mhroth): prepareAndGetSpiBytes() functions on the basis of 4 0RGB pixels at a time.
  // m_rgb must therefore be a multiple of 16 bytes (4 pixels)
  m_numRgbBytesTotal = ((4 * m_numLeds) + 15) & ~0xF;
  m_rgb = (float *) malloc(m_numRgbBytesTotal * sizeof(float));
  assert(rgb != nullptr);

  // prepare SPI data
  // https://cpldcpu.com/2014/11/30/understanding-the-apa102-superled/
  // NOTE(mhroth): because getSpiBytes() processes 4 LEDS at a time (i.e. 16-byte output),
  // and so that no memory outside of spi_data will be overwritten,
  // ((4*m_numLeds) + numSpiTrailerBytes) must be positive mulitple of 16.
  m_numSpiTrailerBytes = (m_numLeds >> 4) + 1;
  m_numSpiBytes = 4 + (4*m_numLeds) + m_numSpiTrailerBytes;
  // ensure that it is the next largest multiple-of-16 (if necessary)
  m_numSpiBytesTotal = (m_numSpiBytes + 15) & ~0xF;
  m_spiData = (uint8_t *) malloc(m_numSpiBytesTotal * sizeof(uint8_t));
  assert(m_spiData != nullptr);

  // reset all buffers
  clear();
}

PixelBuffer::~PixelBuffer() {
  free(m_rgb);
  free(m_spiData);
}

void PixelBuffer::clear() {
  // reset RGB buffer
  memset(m_rgb, 0, m_numRgbBytesTotal*sizeof(float));

  // reset SPI buffer
  memset(m_spiData, 0, m_numSpiBytesTotal); // leading zeros
}

void PixelBuffer::setGlobal(float g) {
  m_global = fminf(1.0f, fmaxf(0.0f, g));
}

void PixelBuffer::setPowerLimit(float wattLimit) {
  m_ampLimit = (wattLimit > 0.0f) ? wattLimit/5.0f : INFINITY;
}

float PixelBuffer::getPowerLimit() {
  return !isinf(m_ampLimit) ? 5.0f*m_ampLimit : -1.0f;
}

// https://gist.github.com/paulkaplan/5184275
// http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
static void __kelvin_to_rgb(float kelvin, float *r, float *g, float *b) {
  float temp = kelvin / 100.0f;
  float _r, _g, _b;
  if (temp <= 66.0f) {
    _r = 1.0f;
    _g = 0.39008157876902f * logf(temp) - 0.631841443788627f;
    _b = (temp <= 19.0f) ? 0.0f : 0.543206789110196f * logf(temp-10.0f) - 1.19625408914f;
  } else {
    _r = 1.292936186062745f * powf(temp-60.0f, -0.1332047592f);
    _g = 1.129890860895294f * powf(temp-60.0f, -0.0755148492f);
    _b = 1.0f;
  }

  *r = fmaxf(0.0f, fminf(1.0f, _r));
  *g = fmaxf(0.0f, fminf(1.0f, _g));
  *b = fmaxf(0.0f, fminf(1.0f, _b));
}

void PixelBuffer::fill_rgb(float r, float g, float b) {
  const float32x4_t RGB = (float32x4_t) {0.0f, b, g, r};
  for (int i = 0, j = 0; i < m_numLeds; i++, j+=4) {
    vst1q_f32(m_rgb+j, RGB);
  }
}

void PixelBuffer::apply_gain(float f) {
  for (int i = 0, j = 0; i < m_numLeds; ++i, j+=4) {
    float32x4_t x = vld1q_f32(m_rgb+j);
    x = vmulq_n_f32(x, f);
    vst1q_f32(m_rgb+j, x);
  }
}

uint8_t *PixelBuffer::prepareAndGetSpiBytes() {
  const float32x4_t CLAMP_ONE = vdupq_n_f32(1.0f);
  const float32x4_t CLAMP_ZERO = vdupq_n_f32(0.0f);

  // total brightness, _not_ including global
  float32x4_t total = vdupq_n_f32(0.0f);

  // nightshift
  float rw, gw, bw;
  __kelvin_to_rgb(6600.0f*(1.0f-m_nightshift), &rw, &gw, &bw);
  const float32x4_t ns = (float32x4_t) {
      0.0f, // global
      bw,   // blue
      gw,   // green
      rw    // red
  };
  uint8_t G = 0xE0 | static_cast<uint8_t>(m_global * 31.0f);
  uint8x16_t GLOBAL = (uint8x16_t) {G,0,0,0,G,0,0,0,G,0,0,0,G,0,0,0};
  for (int i = 0, j = 0; i < m_numLeds; i+=4, j+=16) {
    float32x4_t x = vld1q_f32(m_rgb+j);
    x = vmaxq_f32(x, CLAMP_ZERO); // clamp to [0,1]
    x = vminq_f32(x, CLAMP_ONE);
    x = vmulq_f32(x, ns); // apply nightshift
    // apply gamma adjustment (based on lookup table)
    // NOTE(mhroth): original LUT is roughly x**2.8 (==14/5). In this case we calculate x**3 as it is much
    // easier and faster. The deviation is at most 7 steps darker.
    x = vmulq_f32(x, vmulq_f32(x, x));
    total = vaddq_f32(total, x); // keep track of total brightness
    x = vmulq_n_f32(x, 255.0f);
    uint16x4_t x_u16 = vmovn_u32(vcvtq_u32_f32(x));

    float32x4_t y = vld1q_f32(m_rgb+j+4);
    y = vmaxq_f32(vminq_f32(y, CLAMP_ONE), CLAMP_ZERO);
    y = vmulq_f32(y, ns);
    y = vmulq_f32(y, vmulq_f32(y, y));
    total = vaddq_f32(total, y);
    y = vmulq_n_f32(y, 255.0f);
    uint16x4_t y_u16 = vmovn_u32(vcvtq_u32_f32(y));

    uint8x8_t z_u8 = vmovn_u16(vcombine_u16(x_u16, y_u16));

    float32x4_t a = vld1q_f32(m_rgb+j+8);
    a = vmaxq_f32(vminq_f32(a, CLAMP_ONE), CLAMP_ZERO);
    a = vmulq_f32(a, ns);
    a = vmulq_f32(a, vmulq_f32(a, a));
    total = vaddq_f32(total, a);
    a = vmulq_n_f32(a, 255.0f);
    uint16x4_t a_u16 = vmovn_u32(vcvtq_u32_f32(a));

    float32x4_t b = vld1q_f32(m_rgb+j+12);
    b = vmaxq_f32(vminq_f32(b, CLAMP_ONE), CLAMP_ZERO);
    b = vmulq_f32(b, ns);
    b = vmulq_f32(b, vmulq_f32(b, b));
    total = vaddq_f32(total, b);
    b = vmulq_n_f32(b, 255.0f);
    uint16x4_t b_u16 = vmovn_u32(vcvtq_u32_f32(b));

    uint8x8_t c_u8 = vmovn_u16(vcombine_u16(a_u16, b_u16));

    uint8x16_t d_u8 = vorrq_u8(vcombine_u8(z_u8, c_u8), GLOBAL); // add global value into bytestream

    // write to the spi_data buffer
    vst1q_u8(m_spiData+4+j, d_u8);
  }

  // update current amperage usage
  //                                 BLUE                       GREEN                      RED
  m_currentAmps = m_global * .02f * (vgetq_lane_f32(total, 1) + vgetq_lane_f32(total, 2) + vgetq_lane_f32(total, 3));

  // adjust global brightness if we are over the power limit
  if (m_currentAmps > m_ampLimit) {
    int8_t gf = static_cast<int8_t>(31.0f * m_global * m_ampLimit / m_currentAmps);
    m_currentAmps = gf * m_currentAmps / (31.0f * m_global);

    gf -= static_cast<int8_t>(G);

    // go through the whole SPI buffer and update with the new global
    int8x16_t GF = (int8x16_t) {gf,0,0,0,gf,0,0,0,gf,0,0,0,gf,0,0,0};
    for (int i = 0, j = 0; i < m_numLeds; i+=4, j+=16) {
      int8_t *const data = (int8_t *) (m_spiData + 4 + j);
      vst1q_s8(data, vaddq_s8(vld1q_s8(data), GF));
    }

    m_isPowerSuppressionEngaged = true;
  } else {
    m_isPowerSuppressionEngaged = false;
  }

  // clear trailing bytes, as above loop may have overwriten some
  memset(m_spiData + (4*(m_numLeds+1)), 0xFF, m_numSpiTrailerBytes);

  return m_spiData;
}

void PixelBuffer::set_pixel_rgb_blend(int i, float r, float g, float b, float a, BlendMode mode) {
  assert(i >= 0 && i < m_numLeds);
  const int j = 4 * i;

  switch (mode) {
    default:
    case SET: {
      m_rgb[j+3] = r; m_rgb[j+2] = g; m_rgb[j+1] = b;
      break;
    }
    case ADD: {
      const float z = 1.0f - a;
      m_rgb[j+3] = a*r + z*m_rgb[j+3]; m_rgb[j+2] = a*g + z*m_rgb[j+2]; m_rgb[j+1] = a*b + z*m_rgb[j+1];
      break;
    }
    case ACCUMULATE: {
      m_rgb[j+3] += a*r; m_rgb[j+2] += a*g; m_rgb[j+1] += a*b;
      break;
    }
    case DIFFERENCE: {
      set_pixel_rgb_blend(i, fabsf(r-m_rgb[j+3]), fabsf(g-m_rgb[j+2]), fabsf(b-m_rgb[j+1]), a, BlendMode::ADD);
      break;
    }
    case MULTIPLY: {
      set_pixel_rgb_blend(i, m_rgb[j+3]*r, m_rgb[j+2]*g, m_rgb[j+1]*g, a, BlendMode::ADD);
      break;
    }
    case SCREEN: {
      m_rgb[j+3] = 1.0f-(r*m_rgb[j+3]); m_rgb[j+2] = 1.0f-(g*m_rgb[j+2]); m_rgb[j+1] = 1.0f-(b*m_rgb[j+1]);
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
