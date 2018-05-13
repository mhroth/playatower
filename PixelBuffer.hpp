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

  PixelBuffer(uint32_t numLeds);
  ~PixelBuffer();

  /** Returns the number of LEDs in the strip. */
  int getNumLeds() const { return m_numLeds; }

  /**
   * Returns the current ampere currently being consumed by the LED strip.
   * Takes into account global brightness setting.
   */
  float getCurrentAmperes() const { return m_currentAmps; }

  /** Returns the number of watts currently being consumed by the LED strip. */
  float getCurrentWatts() const { return 5.0f * getCurrentAmperes(); }

  /** Returns the maximum number of amperes that could be consumed by the LED strip. */
  float getMaxAmperes() const { return 0.06f * getNumLeds(); }

  /** Returns the maximum number of watts that could be consumed by the LED strip. */
  float getMaxWatts() const { return 5.0f*getMaxAmperes(); }

  /** Set the global brightness value. [0,1] */
  void setGlobal(float g);

  /** Returns the current global brightness value. [0,1] */
  float getGlobal() const { return m_global; }

  /** Set the power limit. -1 for no ceiling.  */
  void setPowerLimit(float wattLimit);

  float getPowerLimit();

  void setNightshift(float nightshift) { m_nightshift = nightshift; }

  float getNightshift() const { return m_nightshift; }

  /** The number of valid bytes in the SPI buffer. */
  uint32_t getNumSpiBytes() const { return m_numSpiBytes; }

  /** The total number of bytes backing the SPI buffer. */
  uint32_t getNumSpiBytesTotal() const { return m_numSpiBytesTotal; }

  /**
   * Converts RGB data into a buffer suitable for sending over SPI to the LED strip.
   * Takes into account global, nightshift, and power limit settings.
   *
   * @return A pointer to the SPI buffer.
   */
  uint8_t *prepareAndGetSpiBytes();

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

  bool isPowerSuppressionEngaged() const { return m_isPowerSuppressionEngaged; }

 private:
  /** The global brightness factor. [0,1] */
  float m_global;

  /** The total number of LEDs in this animation. */
  int m_numLeds;

  /** The RGB pixel buffer. It has a format (per LED) of GLOBAL, BLUE, GREEN, RED. */
  float *m_rgb;

  /** The total length of the m_rgb buffer. */
  int m_numRgbBytesTotal;

  /** The SPI buffer. */
  uint8_t *m_spiData;

  /** The total number of bytes in the SPI buffer. */
  uint32_t m_numSpiBytes;

  /** The number of trailer bytes in the SPI buffer. */
  uint32_t m_numSpiTrailerBytes;

  /** The total length of the spi_data buffer. */
  uint32_t m_numSpiBytesTotal;

  float m_ampLimit;

  /** Nightshift. [0,1]. 1 is maximum nightshift. */
  float m_nightshift;

  float m_currentAmps;

  bool m_isPowerSuppressionEngaged;
};

#endif // _PIXEL_BUFER_HPP_
