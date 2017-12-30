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

#include "AnimRandomFlow.hpp"

AnimRandomFlow::AnimRandomFlow(PixelBuffer *pixbuf) : Animation(pixbuf) {
  __state.resize(3*pixbuf->getNumLeds());
  __kernel.resize(9);
  __d_uniform = std::uniform_real_distribution<float>(0.0f, 1.0f);

  for (int i = 0; i < __state.size(); i++) {
    __state[i] = __d_uniform(_gen);
  }

  __kernel[0] = 0.2f;
  __kernel[1] = 0.4f;
  __kernel[2] = 0.6f;
  __kernel[3] = 0.8f;
  __kernel[4] = 1.0f;
  __kernel[5] = 0.8f;
  __kernel[6] = 0.6f;
  __kernel[7] = 0.4f;
  __kernel[8] = 0.2f;
  __normalise(__kernel.data(), __kernel.size());
}

AnimRandomFlow::~AnimRandomFlow() {}

void AnimRandomFlow::__normalise(float *b, int n) {
  float sum = 0.0f;
  for (int i = 0; i < n; ++i) sum += b[i];
  for (int i = 0; i < n; ++i) b[i] /= sum;
}

void AnimRandomFlow::_process(double dt) {
  for (int i = 0; i < __state.size(); i++) {
    __state[i] = __d_uniform(_gen);
  }

  const int N = _pixbuf->getNumLeds();
  for (int i = 5; i < N-5; ++i) {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    // for (int j = 0; j < __kernel.size(); ++j) {
    //   r += __state[i-4+j]*__kernel[j];
    //   g += __state[i-4+j+N]*__kernel[j];
    //   b += __state[i-4+j+N+N]*__kernel[j];
    // }

    r = __state[i];
    g = __state[i+N];
    b = __state[i+N+N];

    _pixbuf->set_pixel_rgb_blend(i, r, g, b);
  }
}
