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

  // https://css-tricks.com/almanac/properties/m/mix-blend-mode/
  // https://www.w3.org/TR/compositing-1
  enum BlendMode : uint32_t {
    SET,
    ADD,
    ACCUMULATE,
    DIFFERENCE, // subtracts the darker of the two colors from the lightest color
    MULTIPLY,
    SCREEN, // multiplies the background and the content then complements the result
  };

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

  /**
   * Set a pixel with a given RGBA value and blend mode.
   *
   * @param i  Pixel index.
   * @param r  Red channel value.
   * @param g  Green channel value.
   * @param b  Blue channel value.
   * @param a  Alpha value. Defaults to 1. [0,1]
   * @param mode  Blend mode to combine the new and existing colors. Default to BlendMode::SET.
   */
  void set_pixel_rgb_blend(int i, float r, float g, float b, float a=1.0f, BlendMode mode=BlendMode::SET);

  /**
   * Set a pixel with a given HSLA value and blend mode. See @set_pixel_rgb_blend.
   *
   * @param i  Pixel index.
   * @param h  Hue channel value.
   * @param s  Saturation channel value.
   * @param l  Luminosity channel value.
   * @param a  Alpha value. Defaults to 1. [0,1]
   * @param mode  Blend mode to combine the new and existing colors. Default to BlendMode::SET.
   */
  void set_pixel_hsl_blend(int i, float h, float s, float l, float a=1.0f, BlendMode mode=BlendMode::SET);

  /** Clear the buffer, set all values to 0. */
  void clear();

  /** Set all pixels to the given values. */
  void fill_rgb(float r, float g, float b);

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
