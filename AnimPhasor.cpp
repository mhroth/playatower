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
}

AnimPhasor::~AnimPhasor() {
  // nothing to do
}

void AnimPhasor::process(double dt) {
  t += (float) dt;

  const int N = pixbuf->getNumLeds();
  for (int i = 0; i < N; ++i) {
    float f = ((i/((float) N)) * (2.0f-0.1f)) + 0.1f;
    float x = sinf(2.0f * M_PI * f * t);
    pixbuf->set_pixel_rgb(i, x, 0.0f, 0.0f);
  }

  ++step;
}
