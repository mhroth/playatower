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

#include "AnimVanDerPol.hpp"

AnimVanDerPol::AnimVanDerPol(PixelBuffer *pixbuf) : Animation(pixbuf) {
  x = 2.0;
  y = 0.0;
  u = 8.53;
  t = 0.0f;
  min_x = -2.03f;
  max_x = 2.03f;
  min_y = -12.36f;
  max_y = 12.36f;

  state_x = (float *) calloc(pixbuf->getNumLeds(), sizeof(float));
  state_y = (float *) calloc(pixbuf->getNumLeds(), sizeof(float));

  for (int i = 0; i < pixbuf->getNumLeds(); i++) {
    state_x[i] = lin_scale(x, 0, pixbuf->getNumLeds(), 0.0f, 5.0f);
  }
}

AnimVanDerPol::~AnimVanDerPol() {
  free(state_x);
  free(state_y);
}

void AnimVanDerPol::process(double dt) {
/*
  t += dt;

  float dx = y;
  float dy = u*(1.0 - x*x)*y - x + 1.2f*sinf(M_TAU*0.1f*t);
  x += dx * dt;
  y += dy * dt;

  min_x = fminf(min_x, x);
  max_x = fmaxf(max_x, x);
  min_y = fminf(min_y, y);
  max_y = fmaxf(max_y, y);

  // printf("[%0.2f|%0.2f|%0.2f] [%0.2f|%0.2f|%0.2f] [%0.2f|%0.2f]\n", min_x, x, max_x, min_y, y, max_y, dx, dy);

  const int N = pixbuf->getNumLeds();
  float mu = lin_scale(x, min_x, max_x, 0, N-1);
  float sigma = lin_scale(y, min_y, max_y, 0.1f, 1.0f);
  // printf("[%0.2f|%0.2f]\n", mu, sigma);

  pixbuf->clear();

  for (int i = 0; i < N; ++i) {
    float dx = state_y[i];
    float dy = u*(1.0 - state_x[i]*state_x[i])*state_y[i] - state_x[i] + 1.2f*sinf(M_TAU*0.1f*t);
    state_x[i] += dx * dt;
    state_y[i] += dy * dt;

    float x = lin_scale(state_x[i], min_x, max_x, 0, 1);
    pixbuf->set_pixel_rgb_blend(i, x, x, x);

    // evaporate energy
    // pixbuf->set_pixel_rgb_blend(i, 0.0f, 0.0f, 0.0f, dt, PixelBuffer::BlendMode::ADD);
    // float x = fminf(1.0f, pdf_logNormal(i+1, mu, sigma)); // [0,1]

    // float a = pdf_normal(i, mu, sigma) * M_SQRT_TAU * sigma; // a = [0,1]
    // pixbuf->set_pixel_hsl_blend(i, 60.0f, 0.5f, a, 1.0f, PixelBuffer::BlendMode::SET);
  }

  // float a = floorf(mu);
  // float b = mu - a;
  // float c = a + 1.0f - b;
  // pixbuf->set_pixel_rgb_blend(mu, 1.0f, 1.0f, 1.0f, 1.0f, PixelBuffer::BlendMode::SET);
  // pixbuf->set_pixel_rgb_blend(a+1.0f, c*1.0f, c*1.0f, c*1.0f, 1.0f, PixelBuffer::BlendMode::SET);
*/
}
