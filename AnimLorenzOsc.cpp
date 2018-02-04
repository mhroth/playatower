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

#include <stdlib.h>
#include <ctime>

#include "AnimLorenzOsc.hpp"

AnimLorenzOsc::AnimLorenzOsc(PixelBuffer *pixbuf) :
    Animation(pixbuf) {
  sigma = 10.0;
  rho = 28.0;
  beta = 8.0/3.0;

  __rgb_sigma = 13.0f;

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
}

AnimLorenzOsc::~AnimLorenzOsc() {
  // nothing to do
}

void AnimLorenzOsc::setParameter(int index, float value) {
  switch (index) {
    case 0: __rgb_sigma = powf(10.0f, lin_scale(value, 0.0f, 1.0f, 0.0f, 2.0f)); break;
    default: break;
  }
}

float AnimLorenzOsc::getParameter(int index) {
  switch(index) {
    case 0: return __rgb_sigma;
    default: return -1.0f;
  }
}

void AnimLorenzOsc::_process(double dt) {
  // https://en.wikipedia.org/wiki/Lorenz_system
  dx = sigma * (y - x);
  dy = (x * (rho - z)) - y;
  dz = x*y - beta*z;

  x += dx * dt;
  y += dy * dt;
  z += dz * dt;

  min_x = fmin(min_x, x); max_x = fmax(max_x, x);
  min_y = fmin(min_y, y); max_y = fmax(max_y, y);
  min_z = fmin(min_z, z); max_z = fmax(max_z, z);
  max_dx = fmax(fabs(dx), max_dx);
  max_dy = fmax(fabs(dx), max_dy);
  max_dx = fmax(fabs(dz), max_dz);

  const int N = _pixbuf->getNumLeds();

  int i_r = lin_scale(x, min_x, max_x, 0, N-1);
  int i_g = lin_scale(y, min_y, max_y, 0, N-1);
  int i_b = lin_scale(z, min_z, max_z, 0, N-1);

  float r_sigma = lin_scale(fabs(dx), 0.0f, max_dx, 0, 1);
  float g_sigma = lin_scale(fabs(dy), 0.0f, max_dy, 0, 1);
  float b_sigma = lin_scale(fabs(dz), 0.0f, max_dz, 0, 1);

  const float lightness_sigma_const = __rgb_sigma * M_SQRT_TAU * 0.67f;

  for (int i = 0; i < N; ++i) {
    float r = pdf_normal(i, i_r, __rgb_sigma) * lightness_sigma_const;
    float g = pdf_normal(i, i_g, __rgb_sigma) * lightness_sigma_const;
    float b = pdf_normal(i, i_b, __rgb_sigma) * lightness_sigma_const;

    _pixbuf->set_pixel_hsl_blend(i, 0.0f, r_sigma, r);
    _pixbuf->set_pixel_hsl_blend(i, 120.0f, g_sigma, g, 0.5f, PixelBuffer::BlendMode::ADD);
    _pixbuf->set_pixel_hsl_blend(i, 240.0f, b_sigma, b, 0.33f, PixelBuffer::BlendMode::ADD);
  }
}
