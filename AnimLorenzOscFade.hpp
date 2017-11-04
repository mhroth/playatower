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

#ifndef _ANIM_LORENZ_OSC_FADE_HPP_
#define _ANIM_LORENZ_OSC_FADE_HPP_

#import "Animation.hpp"

class AnimLorenzOscFade: public Animation {
 public:
  AnimLorenzOscFade(uint32_t numLeds, uint8_t global);
  ~AnimLorenzOscFade();

  void process(double dt, uint8_t *data) override;

 private:
  void add_pixel_rgb(int i, float r, float g, float b, float a);

  double x, y, z, dx, dy, dz;
  double beta, rho, sigma;
  double min_x, max_x, min_y, max_y, min_z, max_z;

  float *rgb_buffer;
};

#endif // _ANIM_LORENZ_OSC_FADE_HPP_
