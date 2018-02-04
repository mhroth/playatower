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

#include "AnimRain.hpp"

#define DROP_INTERVAL_UPDATE_SEC 10.0f

AnimRain::AnimRain(PixelBuffer *pixbuf) : Animation(pixbuf) {
  __d_dd = std::normal_distribution<float>(0.0f, 0.2f);
  __drop_lambda = 1.0f; // 1/second
  __d_exp = std::exponential_distribution<float>(__drop_lambda);
  __d_up = std::exponential_distribution<float>(1.0f/4.0f); // every 4 seconds
  __d_vel = std::uniform_real_distribution<float>(0.0f, 5.0f); // initial velocity
  __t_d = __d_exp(_gen); // schedule first drop
  __t_u = __d_up(_gen);
  __t_dd = DROP_INTERVAL_UPDATE_SEC;
  __a = -1.4f; // default acceleration

  v_min = INFINITY;
  v_max = -INFINITY;
}

AnimRain::~AnimRain() {}

void AnimRain::setParameter(int index, float value) {
  switch (index) {
    case 0: __a = 2.0f * -9.8f * value; break;
    default: break;
  }
}

float AnimRain::getParameter(int index) {
  switch (index) {
    case 0: return __a;
    default: return -1.0f;
  }
}

void AnimRain::_process(double dt) {
  if (_t >= __t_dd) {
    __drop_lambda += __d_dd(_gen);
    // ensure that lambda is never less frequent than once per 10s
    __drop_lambda = fmaxf(__drop_lambda, 0.1f);
    __d_exp = std::exponential_distribution<float>(__drop_lambda);
    __t_dd = _t + DROP_INTERVAL_UPDATE_SEC;
  }


  if (_t >= __t_d) {
    // add a new Drop to the list

    Drop d = {
      _t,                // initial time
      __d_vel(_gen),     // initial velocity
      0
    };
    drop_list.push_back(d);

    // schedule the next drop
    __t_d = _t + __d_exp(_gen);
  }

  const float k_decay = expf(-((float) dt)/0.10f);
  _pixbuf->apply_gain(k_decay);

  for (Drop &d : drop_list) {
    float t = _t - d.t;
    float y = __a*t*t + d.v_o*t + 8.0f;

    y *= 30.0f;
    float a, b, c, e;
    a = floorf(y);
    b = a + 1.0f;
    c = y - a;
    e = b - y;

    d.i = (int) a;

    float v = fabsf(__a*t + d.v_o);
    v_min = fminf(v, v_min);
    v_max = fmaxf(v, v_max);

    if (a < 0.0f || b >= _pixbuf->getNumLeds()) continue;

    float h = lin_scale(v, v_min, v_max, 180.0f, 240.0f);
    float vv = lin_scale(v, v_min, v_max, 0.4f, 0.8f);

    _pixbuf->set_pixel_hsl_blend((int) a, h, c, 0.5f, 100*dt, PixelBuffer::BlendMode::ACCUMULATE);
    _pixbuf->set_pixel_hsl_blend((int) b, h, e, 0.5f, 100*dt, PixelBuffer::BlendMode::ACCUMULATE);
  }

  while (!drop_list.empty() && drop_list.front().i <= 0) {
    drop_list.pop_front();
  }



  if (_t >= __t_u) {
    // add a new Up to the list

    Drop d = {_t, 0.0f, 0};
    up_list.push_back(d);

    // schedule the next drop
    __t_u = _t + __d_up(_gen);
  }

  for (Drop &d : up_list) {
    float t = _t - d.t;
    float y = 0.4f*t + 0.0f;

    y *= 30.0f;
    float a, b, c, e;
    a = floorf(y);
    b = a + 1.0f;
    c = y - a;
    e = b - y;

    d.i = (int) a;

    // float v = fabsf(__a*t + d.v_o);
    // v_min = fminf(v, v_min);
    // v_max = fmaxf(v, v_max);

    if (a < 0.0f || b >= _pixbuf->getNumLeds()) continue;

    // float h = lin_scale(v, v_min, v_max, 180.0f, 240.0f);
    // float vv = lin_scale(v, v_min, v_max, 0.0f, 0.8f);

#define BASE_COLOR_R (255.0f/255.0f)
#define BASE_COLOR_G (132.0f/255.0f)
#define BASE_COLOR_B (1.0f/255.0f)

    _pixbuf->set_pixel_rgb_blend((int) a, BASE_COLOR_R, BASE_COLOR_G, BASE_COLOR_B, 20*dt, PixelBuffer::BlendMode::ACCUMULATE);
    _pixbuf->set_pixel_rgb_blend((int) b, BASE_COLOR_R, BASE_COLOR_G, BASE_COLOR_B, 20*dt, PixelBuffer::BlendMode::ACCUMULATE);
  }

  while (!up_list.empty() && up_list.front().i >= _pixbuf->getNumLeds()) {
    up_list.pop_front();
  }



  // the white starter line
  int x = 0.8f * _pixbuf->getNumLeds();
  for (int i = 0; i < 11; i+=2) {
    _pixbuf->set_pixel_rgb_blend(x+i-5, 0.2f, 0.05f, 0.2f, 0.7f, PixelBuffer::BlendMode::ADD);
    _pixbuf->set_pixel_rgb_blend(x+i-5+1, 0.4f, 0.2f, 0.4f, 0.7f, PixelBuffer::BlendMode::ADD);
  }
}
