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
  x = 0.0;
  y = 0.0;
  u = 8.53;
  t = 0.0f;
}

AnimVanDerPol::~AnimVanDerPol() {

}

void AnimVanDerPol::process(double dt) {
/*
  t += dt;

  // TODO(mhroth): display shit

  for (int i = 0; i < numLeds; ++i) {
    // evaporate energy

    pixbuf->set_pixel_rgb_blend(i, 0.0f, 0.0f, 0.0f, dt, PixelBuffer::BlendMode::ADD);

    float x = pdf_logNormal(i, x, sigma);

    pixbuf->set_pixel_hsl_blend(i, 30.0f, 0.5f, x, 1.0f, PixelBuffer::BlendMode::SCREEN);
  }

  float dx = y;
  float dy = u*(1.0 - x*x)*y - x + 1.2f*sinf(M_TAU*0.1f*t);
  x += dx * dt;
  y += dy * dt;
*/
}
