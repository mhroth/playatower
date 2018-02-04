/**
 * Copyright (c) 2018, Martin Roth (mhroth@gmail.com)
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

#ifndef _ANIM_RAIN_HPP_
#define _ANIM_RAIN_HPP_

#include <list>

#include "Animation.hpp"

class AnimRain: public Animation {
 public:
  AnimRain(PixelBuffer *pixbuf);
  ~AnimRain();

  void setParameter(int index, float value) override;
  float getParameter(int index) override;

  const char *getName() override { return "Rain"; }

 private:
  typedef struct {
    float t;   // time at which this drop was instantiated
    float v_o; // initial velocity
    int i;
  } Drop;

  void _process(double dt) override;

  std::list<Drop> drop_list;
  std::list<Drop> up_list;

  float __t_d; // time of next drop
  float __t_u; // time of next up
  float __t_dd; // time of next drop distribution update
  float __a; // acceleration
  float __drop_lambda;
  std::exponential_distribution<float> __d_exp;
  std::exponential_distribution<float> __d_up;
  std::uniform_real_distribution<float> __d_vel;
  std::normal_distribution<float> __d_dd;

  float v_min, v_max;
};

#endif // _ANIM_RAIN_HPP_
