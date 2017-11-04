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

#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

class Animation {
 public:
  Animation(uint32_t numLeds, uint8_t global=31);
  virtual ~Animation() {}

  /** Set the global brightness factor. [0-31] */
  void setGlobal(uint8_t g) { global = (g & 0x1F); }

  virtual void process(double dt, uint8_t *data) = 0;

 protected:
  void set_pixel_rgb(uint8_t *data, int i, float r, float g, float b);

  double lin_scale(double x, double min_in, double max_in, double min_out, double max_out);

  /** The global brightness factor. [0-31] */
  uint8_t global;

  /** The total number of LEDs in this animation. */
  uint32_t numLeds;

  /** The number of frames processed so far by this animation. */
  uint32_t step;
};

#endif // _ANIMATION_HPP_
