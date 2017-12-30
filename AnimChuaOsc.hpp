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

#ifndef _ANIM_CHUA_OSC_HPP_
#define _ANIM_CHUA_OSC_HPP_

#import "Animation.hpp"

class AnimChuaOsc: public Animation {
 public:
  AnimChuaOsc(PixelBuffer *pixbuf);
  ~AnimChuaOsc();

  void setParameter(int index, float value) override;

  const char *getName() override { return "Chua Oscillator"; }

 private:
  void _process(double dt) override;

  double x, y, z;
  double min_x, max_x, min_y, max_y, min_z, max_z;
  double __dx_range, __dy_range, __dz_range;
  std::exponential_distribution<float> __d_exp;
  std::uniform_real_distribution<float> __d_uniform;
  float __t_next_color_change;
  float __base_hue;
};

#endif // _ANIM_CHUA_OSC_HPP_
