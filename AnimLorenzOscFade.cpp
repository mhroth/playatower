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

#include "AnimLorenzOscFade.hpp"

AnimLorenzOscFade::AnimLorenzOscFade(PixelBuffer *_pixbuf) :
    Animation(_pixbuf) {

  alpha_mult = 200.0;

  sigma = 10.0;
  rho = 28.0;
  beta = 8.0/3.0;

  // random starting position on unit sphere
  srand((unsigned)time(0));
  x = ((double) rand()) / ((double) RAND_MAX);
  y = ((double) rand()) / ((double) RAND_MAX);
  z = ((double) rand()) / ((double) RAND_MAX);
  double norm = sqrt(x*x + y*y + z*z);
  x /= norm; y /= norm; z /= norm;

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;
  max_dx = -INFINITY; max_dy = -INFINITY; max_dz = -INFINITY;
  max_speed = -INFINITY;

  c_h = 360.0 * ((double) rand()) / ((double) RAND_MAX);
  c_s = (0.8-0.2) * ((double) rand()) / ((double) RAND_MAX) + 0.2;
  // c_l = (0.6-0.2) * ((double) rand()) / ((double) RAND_MAX) + 0.2;
  c_l = 0.5;
}

AnimLorenzOscFade::~AnimLorenzOscFade() {
  // nothing to do
}

void AnimLorenzOscFade::setParameter(int index, float value) {
  switch (index) {
    case 0: alpha_mult = 300.0*value; break;
    default: break;
  }
}

float AnimLorenzOscFade::getParameter(int index) {
  switch (index) {
    case 0: return alpha_mult;
    default: return -1.0f;
  }
}

void AnimLorenzOscFade::_process(double dt) {
  double osc0 = sin(2.0 * M_PI * (1.0/(10*60.0)) * _t); // 10 minutes
  double osc1 = sin(2.0 * M_PI * (1.0/(2.5*60.0)) * _t); // 2.5 minutes
  double osc2 = sin(2.0 * M_PI * (1.0/(14*60.0)) * _t); // 14 minutes
  sigma = lin_scale(osc0, -1.0, 1.0, 5.0, 15.0);
  rho = lin_scale(osc1, -1.0, 1.0, 24.0, 36.0);
  beta = lin_scale(osc2, -1.0, 1.0, 2.0, 3.33);

  // https://en.wikipedia.org/wiki/Lorenz_system
  dx = sigma * (y - x);
  dy = (x * (rho - z)) - y;
  dz = x*y - beta*z;

  x += dx * dt;
  y += dy * dt;
  z += dz * dt;

  // measure coordinate bounds
  min_x = fmin(min_x, x); max_x = fmax(max_x, x);
  min_y = fmin(min_y, y); max_y = fmax(max_y, y);
  min_z = fmin(min_z, z); max_z = fmax(max_z, z);

  // measure speed bounds (with auto reset)
  const double s_decay = expf(-dt/300.0); // over 300 seconds
  max_dx = fmax(fabs(dx), max_dx*s_decay);
  max_dy = fmax(fabs(dy), max_dy*s_decay);
  max_dz = fmax(fabs(dz), max_dz*s_decay);

  double speed = sqrt(dx*dx + dy*dy + dz*dz);
  max_speed = fmax(max_speed, speed);

  const float k_decay = expf(-((float) dt)/1.0f);
  _pixbuf->apply_gain(k_decay);

  const int N = _pixbuf->getNumLeds();

  double osc4 = sin(2.0 * M_PI * (1.0/(30*60.0)) * _t); // 30 minutes
  c_h = lin_scale(osc4, -1.0, 1.0, 0.0, 360.0);
  // double a = lin_scale(speed, 0.0, max_speed, 0.0, 1.0);
  double a = speed/max_speed;
  a = lin_scale(a*a,  0.0, 1.0, 25.0, 150.0);

  int i_r = lin_scale(x, min_x, max_x, 0, N-1);
  double l_x = lin_scale(fabs(dx), 0.0, max_dx, 0.05, c_l);
  _pixbuf->set_pixel_hsl_blend(i_r, c_h, c_s, l_x, alpha_mult*dt, PixelBuffer::BlendMode::ACCUMULATE);

  int i_g = lin_scale(y, min_y, max_y, 0, N-1);
  double l_y = lin_scale(fabs(dy), 0.0, max_dy, 0.05, c_l);
  _pixbuf->set_pixel_hsl_blend(i_g, c_h+a, c_s, l_y, alpha_mult*dt, PixelBuffer::BlendMode::ACCUMULATE);

  int i_b = lin_scale(z, min_z, max_z, 0, N-1);
  double l_z = lin_scale(fabs(dz), 0.0, max_dz, 0.05, c_l);
  _pixbuf->set_pixel_hsl_blend(i_b, c_h-a, c_s, l_z, alpha_mult*dt, PixelBuffer::BlendMode::ACCUMULATE);
}
