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

#ifndef _ANIM_EIFFEL_TOWER_HPP_
#define _ANIM_EIFFEL_TOWER_HPP_

#include "Animation.hpp"

class AnimEiffelTower: public Animation {
 public:
  AnimEiffelTower(PixelBuffer *pixbuf);
  ~AnimEiffelTower();

  void process(double dt) override;

  void setParameter(int index, float value) override;

  const char *getName() override { return "Eiffel Tower"; }

 private:
  double t;
  float mean_flash_time;
  float flash_decay_period;
  float __shimmer_rate;
  std::vector<float> __shimmer;
  std::default_random_engine gen;
  std::uniform_real_distribution<float> d_uniform;
  std::normal_distribution<float> d_gauss;
};

#endif // _ANIM_EIFEL_TOWER_HPP_
