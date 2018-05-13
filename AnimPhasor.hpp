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

#ifndef _ANIM_PHASOR_HPP_
#define _ANIM_PHASOR_HPP_

#include "Animation.hpp"

class AnimPhasor: public Animation {
 public:
  AnimPhasor(PixelBuffer *pixbuf);
  ~AnimPhasor();

  void setParameter(int index, float value) override;
  float getParameter(int index) override;

  const char *getName() override { return "Phasor"; }

 private:
  void _process(double dt) override;

  void updateTarget(float value);

  float __t_o; // time of last change
  float __t_c; // time of next change
  float __f_min, __f_target, __f_prev_target;
  std::uniform_real_distribution<float> __d_uniform;
  std::exponential_distribution<float> __d_exp;
};

#endif // _ANIM_PHASOR_HPP_
