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
#include <stdlib.h>
#include <string.h>

#include "PixelBuffer.hpp"

#define M_TAU 6.283185307179586f
#define M_SQRT_TAU 2.506628274631001f // sqrt(2*pi)

class Animation {
 public:
  Animation(PixelBuffer *pixbuf);
  virtual ~Animation() {}

  virtual void process(double dt) = 0;

  /**
   * Returns the preferred frames per second of this animation.
   *
   * A non-positive number indicates an infinite fps i.e. as many fps as can be rendered.
   */
  virtual double getPreferredFps() { return -1.0; }

 protected:
  /** Linear scaling. */
  double lin_scale(double x, double min_in, double max_in, double min_out, double max_out);

  /**
   * Gaussian distribution.
   * https://en.wikipedia.org/wiki/Normal_distribution
   *
   * @param mu  Distribution mean.
   * @param sigma  Distribution standard deviation.
   */
  float pdf_normal(float x, float mu, float sigma);

  /**
   * Log-Normal distribution.
   * https://en.wikipedia.org/wiki/Log-normal_distribution
   *
   * @param mu  Distribution mean.
   * @param sigma  Distribution standard deviation.
   */
  float pdf_logNormal(float x, float mu, float sigma);

  /** The number of frames processed so far by this animation. */
  uint32_t step;

  PixelBuffer *pixbuf;
};

#endif // _ANIMATION_HPP_
