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

#include "AnimLighthouse.hpp"

#define LIGHTHOUSE_FREQ_R 1.0/15.5
#define LIGHTHOUSE_FREQ_G 1.0/15.0
#define LIGHTHOUSE_FREQ_B 1.0/14.5

AnimLighthouse::AnimLighthouse(PixelBuffer *pixbuf) : Animation(pixbuf) {
  t = 0.0;
}

AnimLighthouse::~AnimLighthouse() {}

void AnimLighthouse::process(double dt) {
  t += dt;

  for (int i = 0; i < pixbuf->getNumLeds(); i++) {
    float r = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_R * t) + (2*M_PI/(i%11)));
    float g = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_G * t) + (2*M_PI/(i%11)));
    float b = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_B * t) + (2*M_PI/(i%11)));
    pixbuf->set_pixel_rgb_blend(i, fmaxf(0.0f,r), fmaxf(0.0f,g), fmaxf(0.0f,b));
  }

  ++step;
}
