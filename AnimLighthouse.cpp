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

#define LIGHTHOUSE_FREQ_R (1.0/300.0)
#define LIGHTHOUSE_FREQ_G (-1.0/60.0)
#define LIGHTHOUSE_FREQ_B (1.0/10.0)

AnimLighthouse::AnimLighthouse(PixelBuffer *pixbuf) :
    Animation(pixbuf) {
  __saturation = 0.67f;
}

AnimLighthouse::~AnimLighthouse() {}

void AnimLighthouse::setParameter(int index, float value) {
  switch (index) {
    case 0: __saturation = value; break;
    default: break;
  }
}

void AnimLighthouse::_process(double dt) {
  const int N = _pixbuf->getNumLeds();
  for (int i = 0; i < N; i++) {
    float r = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_R * _t) + (2*M_PI/((i%11)+1)));
    float g = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_G * _t) + (2*M_PI/((i%11)+1)));
    float b = sinf((2.0 * M_PI * LIGHTHOUSE_FREQ_B * _t) + (2*M_PI/((i%11)+1)));
    _pixbuf->set_pixel_hsl_blend(i, 0.0f, __saturation, fmaxf(0.0f,r));
    _pixbuf->set_pixel_hsl_blend(i, 60.0f, __saturation, fmaxf(0.0f,g), 0.5f, PixelBuffer::BlendMode::ADD);
    _pixbuf->set_pixel_hsl_blend(i, 210.0f, __saturation, fmaxf(0.0f,b), 0.333f, PixelBuffer::BlendMode::ADD);
  }
}
