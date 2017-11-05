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

#include "PixelBuffer.hpp"

class Animation {
 public:
  Animation(PixelBuffer *pixbuf);
  virtual ~Animation() {}

  virtual void process(double dt) = 0;

 protected:
  double lin_scale(double x, double min_in, double max_in, double min_out, double max_out);

  /** The number of frames processed so far by this animation. */
  uint32_t step;

  PixelBuffer *pixbuf;
};

#endif // _ANIMATION_HPP_
