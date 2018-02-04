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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>

#include "AnimChuaOsc.hpp"

AnimChuaOsc::AnimChuaOsc(PixelBuffer *_pixbuf) :
    Animation(_pixbuf) {

  // init base hue
  __d_uniform = std::uniform_real_distribution<float>(0.0f, 360.0f);
  __base_hue = __d_uniform(_gen);

  // init next color change
  __d_exp = std::exponential_distribution<float>(1.0f/60.0f); // 60 seconds
  __t_next_color_change = __d_exp(_gen);

  // random starting position on unit sphere
  srand((unsigned)time(0));
  x = ((double) rand()) / ((double) RAND_MAX);
  y = ((double) rand()) / ((double) RAND_MAX);
  z = ((double) rand()) / ((double) RAND_MAX);
  double norm = sqrt(x*x + y*y + z*z);
  x /= norm; y /= norm; z /= norm;
  // x = 0.1; y = 0.3; z = -0.6;

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;
  __dx_range = -INFINITY; __dy_range = -INFINITY; __dz_range = -INFINITY;
}

AnimChuaOsc::~AnimChuaOsc() {
  // nothing to do
}

void AnimChuaOsc::setParameter(int index, float value) {
  switch (index) {
    case 0: __t_next_color_change = _t; break; // change color immediately
    default: break;
  }
}

void AnimChuaOsc::_process(double dt) {
  if (__t_next_color_change <= _t) {
    __t_next_color_change = _t + __d_exp(_gen);
    __base_hue = __d_uniform(_gen);
  }

  // https://en.wikipedia.org/wiki/Multiscroll_attractor
  const double a = 36.0;
  const double b = 3.0;
  const double c = 20.0;

  double osc0 = sin(2.0 * M_PI * (1.0/(30*60.0)) * _t); // 30 minutes
  double u = lin_scale(osc0, -1.0, 1.0, -15.0, 15.0);

  double dx = a * (y - x);
  double dy = x - x*z + c*y + u;
  double dz = x*y - b*z;

  x += dx * dt;
  y += dy * dt;
  z += dz * dt;

  const float k1_decay = expf(-((float) dt)/300.0f);
  min_x = fmin(min_x*k1_decay, x); max_x = fmax(max_x*k1_decay, x);
  min_y = fmin(min_y*k1_decay, y); max_y = fmax(max_y*k1_decay, y);
  min_z = fmin(min_z*k1_decay, z); max_z = fmax(max_z*k1_decay, z);

  __dx_range = fmax(k1_decay*__dx_range, fabs(dx));
  __dy_range = fmax(k1_decay*__dy_range, fabs(dy));
  __dz_range = fmax(k1_decay*__dz_range, fabs(dz));

  const float k_decay = expf(-((float) dt)/1.0f);
  _pixbuf->apply_gain(k_decay);

  const int N = _pixbuf->getNumLeds();

  int i_r = lin_scale(x, min_x, max_x, 0, N-1);
  double l_x = lin_scale(fabs(dx), 0.0, __dx_range, 0.01, 0.55+0.1);
  _pixbuf->set_pixel_hsl_blend(i_r, __base_hue, 0.69f, l_x, 200.0f*dt, PixelBuffer::BlendMode::ACCUMULATE);

  int i_g = lin_scale(y, min_y, max_y, 0, N-1);
  double l_y = lin_scale(fabs(dy), 0.0, __dy_range, 0.01, 0.48+0.1);
  _pixbuf->set_pixel_hsl_blend(i_g, __base_hue+30.0f, 0.36f, l_y, 200.0f*dt, PixelBuffer::BlendMode::ACCUMULATE);

  int i_b = lin_scale(z, min_z, max_z, 0, N-1);
  double l_z = lin_scale(fabs(dz), 0.0, __dz_range, 0.01, 0.48+0.1);
  _pixbuf->set_pixel_hsl_blend(i_b, __base_hue-30.0f, 0.9f, l_z, 200.0f*dt, PixelBuffer::BlendMode::ACCUMULATE);
}
