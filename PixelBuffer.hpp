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

#ifndef _PIXEL_BUFER_HPP_
#define _PIXEL_BUFER_HPP_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class PixelBuffer {
 public:
  PixelBuffer(uint32_t numLeds, uint8_t global=31);
  ~PixelBuffer();

  /** Returns the number of LEDs in the strip. */
  uint32_t getNumLeds() { return numLeds; }

  /** Returns the number of amperes currently being consumed by the LED strip. */
  float getAmperes();

  /** Returns the number of watts currently being consumed by the LED strip. */
  float getWatts() { return 5.0f*getAmperes(); }

  /** Returns the maximum number of amperes that could be consumed by the LED strip. */
  float getMaxAmperes() { return (numLeds * 0.06f); }

  /** Returns the maximum number of watts that could be consumed by the LED strip. */
  float getMaxWatts() { return 5.0f*getMaxAmperes(); }

  /** Set the global brightness factor. [0-31] */
  void setGlobal(uint8_t g) { global = (g & 0x1F); }

  uint8_t getGlobal() { return global; }

  uint32_t getNumSpiBytes() { return numSpiBytes; }

  /**
   * Converts RGB data into a buffer suitable for sending over SPI to the LED strip.
   *
   * @param n  Nightshift factor, [0,1]. Zero is no nightshift, one is maximum nightshift.
   */
  uint8_t *getSpiBytes(float n=0.0f);

  void set_pixel_rgb(int i, float r, float g, float b);
  void add_pixel_rgb(int i, float r, float g, float b, float a);
  void add_pixel_hsl(int i, float h, float s, float l, float a);

  void clear();

  /** Multiply all RGB elements by f. */
  void apply_gain(float f);

 private:
  /** The global brightness factor. [0-31] */
  uint8_t global;

  /** The total number of LEDs in this animation. */
  uint32_t numLeds;

  float *rgb;
  uint8_t *spi_data;

  uint32_t numSpiBytes;
};

#endif // _PIXEL_BUFER_HPP_
