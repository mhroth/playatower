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

#import "AnimXmasPhasor.hpp"

AnimXmasPhasor::AnimXmasPhasor(PixelBuffer *pixbuf) : Animation(pixbuf) {
  f_min = 1.0f/120.0f; // 1/2min
  __f_target = 1.0f/2.0f;
  __f_prev_target = __f_target;
  __t_o = 0.0f;

  __d_uniform = std::uniform_real_distribution<float>(0.0f, 1.0f);
  __d_exp = std::exponential_distribution<float>(1.0f/(5.0f*60.0f)); // 5 minutes
  __t_c = 30.0f; // first change happens after 30 seconds
}

AnimXmasPhasor::~AnimXmasPhasor() {}

void AnimXmasPhasor::updateTarget(float value) {
  __f_prev_target = lin_scale(1.0f/(1.0f+expf(-(_t-__t_o-6.0f))),
      0, 1, __f_prev_target, __f_target);

  __t_o = _t;
  __f_target = log_scale(value, log10f(f_min), log10f(2.0f));
}

void AnimXmasPhasor::setParameter(int index, float value) {
  switch (index) {
    case 0: updateTarget(value); break;
    default: break;
  }
}

float AnimXmasPhasor::getParameter(int index) {
  switch (index) {
    case 0: return __f_target;
    default: return -1.0f;
  }
}

void AnimXmasPhasor::_process(double dt) {
  if (__t_c <= _t) {
    updateTarget(__d_uniform(_gen));
    __t_c = _t + __d_exp(_gen);
  }

  float x = 1.0f/(1.0f+expf(-(_t-__t_o-6.0f))); // [0,1]
  float f_z = lin_scale(1.0f/(1.0f+expf(-(_t-__t_o-6.0f))),
      0, 1, __f_prev_target, __f_target);

  const int N = _pixbuf->getNumLeds();
  const float n = (float) N;
  for (int i = 0; i < N; ++i) {
    float f = lin_scale(i, 0, n, f_min, f_z);
    float y = fabsf(sinf(2.0f * M_PI * f * _t));
    if (y < 0.5f) {
      _pixbuf->set_pixel_rgb_blend(i, 0.0f*y/255.0f, 255.0f*y/255.0f, 0.0f*y/255.0f);
    } else {
      _pixbuf->set_pixel_rgb_blend(i, y*255.0f/255.0f, y*0.0f/255.0f, y*0.0f/255.0f);
    }
  }
}
