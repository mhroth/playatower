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

#import "AnimPhasor.hpp"

AnimPhasor::AnimPhasor(PixelBuffer *pixbuf) :
    Animation(pixbuf) {
  t = 0.0f;
  f_min = 1/120.0f;
  f_max = 0.5f;
}

AnimPhasor::~AnimPhasor() {
  // nothing to do
}

void AnimPhasor::process(double dt) {
  t += (float) dt;

  const int N = pixbuf->getNumLeds();
  const float n = (float) N;
  for (int i = 0; i < N; ++i) {
    float f = ((f_max-f_min)*i/n) + f_min;
    float y = fabsf(sinf(2.0f * M_PI * f * t));
    if (y < 0.5f) {
      pixbuf->set_pixel_rgb_blend(i, 0.0f*y/255.0f, 191.0f*y/255.0f, 255.0f*y/255.0f);
    } else {
      pixbuf->set_pixel_rgb_blend(i, y*153.0f/255.0f, y*50.0f/255.0f, y*204.0f/255.0f);
    }
  }

  ++step;
}
