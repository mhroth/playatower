/**
 * Copyright (c) 2017, Martin Roth (mhroth@gmail.com)
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

#include <math.h>

#include "PixelBuffer.hpp"

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

  // RGB buffer. Order is red, green, blue
  rgb = (float *) malloc(3 * numLeds * sizeof(float));
  assert(rgb != nullptr);

  // prepare SPI data
  // https://cpldcpu.com/2014/11/30/understanding-the-apa102-superled/
  const int NUM_SPI_TRAILER_BYTES = (numLeds >> 4) + 1;
  numSpiBytes = 4 + (4*numLeds) + NUM_SPI_TRAILER_BYTES;
  // TODO(mhroth): unknown why 3x bytes are needed. But otherwise free(spi_data) fails!
  spi_data = (uint8_t *) malloc(3 * numSpiBytes * sizeof(uint8_t));
  assert(spi_data != nullptr);

  // initialise SPI data
  memset(spi_data, 0, numSpiBytes);
  memset(spi_data + (4*(numLeds+1)), NUM_SPI_TRAILER_BYTES, 0xFF); // set the trailer

  // reset all buffers to zero
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
  memset(rgb, 0, 3*numLeds*sizeof(float));
}

void PixelBuffer::fill_rgb(float r, float g, float b) {
  for (int i = 0; i < numLeds; i++) {
    set_pixel_rgb_blend(i, r, g, b);
  }
}

void PixelBuffer::apply_gain(float f) {
  const float alpha = 1.0f - f;
  for (int i = 0; i < numLeds; ++i) {
    set_pixel_rgb_blend(i, 0.0f, 0.0f, 0.0f, alpha, BlendMode::ADD);
  }
}

uint8_t *PixelBuffer::getSpiBytes(float n) {
  // TODO(mhroth): just for fun, SIMD type conversion and table lookup,
  // https://stackoverflow.com/questions/22158186/arm-neon-how-to-implement-a-256bytes-look-up-table

  const float n_b = 1.0f - n*0.75f;
  const float n_g = 1.0f - n*0.60f;
  for (int i = 0; i < numLeds; ++i) {
    int j = (i+1)*4;
    spi_data[j+0] = 0xE0 | global;
    spi_data[j+1] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*3+2]) * n_b * 255.0f)]; // blue
    spi_data[j+2] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*3+1]) * n_g * 255.0f)]; // green
    spi_data[j+3] = APA102_GAMMA[(uint8_t) (_clamp(rgb[i*3+0]) * 255.0f)]; // red
  }

  return spi_data;
}

void PixelBuffer::set_pixel_rgb_blend(int i, float r, float g, float b, float a, BlendMode mode) {
  assert(i >= 0 && i < numLeds);
  const int j = 3 * i;

  switch (mode) {
    default:
    case SET: {
      rgb[j+0] = r; rgb[j+1] = g; rgb[j+2] = b;
      break;
    }
    case ADD: {
      const float z = 1.0f - a;
      rgb[j+0] = a*r + z*rgb[j+0]; rgb[j+1] = a*g + z*rgb[j+1]; rgb[j+2] = a*b + z*rgb[j+2];
      break;
    }
    case ACCUMULATE: {
      rgb[j+0] += a*r; rgb[j+1] += a*g; rgb[j+2] += a*b;
      break;
    }
    case DIFFERENCE: {
      set_pixel_rgb_blend(i, fabsf(r-rgb[j+0]), fabsf(g-rgb[j+1]), fabsf(b-rgb[j+2]), a, BlendMode::ADD);
      break;
    }
    case MULTIPLY: {
      set_pixel_rgb_blend(i, rgb[j+0]*r, rgb[j+1]*g, rgb[j+2]*g, a, BlendMode::ADD);
      break;
    }
    case SCREEN: {
      rgb[j+0] = 1.0f-(r*rgb[j+0]); rgb[j+1] = 1.0f-(g*rgb[j+1]); rgb[j+2] = 1.0f-(b*rgb[j+2]);
      break;
    }
  }
}

// http://www.rapidtables.com/convert/color/hsl-to-rgb.htm
void PixelBuffer::set_pixel_hsl_blend(int i, float h, float s, float l, float a, BlendMode mode) {
  // assert(h >= 0.0f && h < 360.0f);
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
