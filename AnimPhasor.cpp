/**
 * Copyright (c) 2017-2018, Martin Roth (mhroth@gmail.com)
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

#include "AnimPhasor.hpp"

AnimPhasor::AnimPhasor(PixelBuffer *pixbuf) : Animation(pixbuf) {
  mFMin = 1.0f/120.0f; // 1/2min
  mFMaxNext = 1.0f/60.0f; // 1/1min
  mFMaxPrev = mFMaxNext;
  mt_o = 0.0f;

  md_uniform = std::uniform_real_distribution<float>(0.0f, 1.0f);
  mt_c = 12.0f; // first change always happens after 30 seconds

  mHueNext = 360.0f * md_uniform(_gen);
  mHuePrev = mHueNext;

  mHueOffset = 220.0f;

  mPhase = (float *) malloc(pixbuf->getNumLeds() * sizeof(float));
  memset(mPhase, 0, pixbuf->getNumLeds() * sizeof(float));
}

AnimPhasor::~AnimPhasor() {
  free(mPhase);
}

void AnimPhasor::setParameter(int index, float value) {
  switch (index) {
    case 0: mHueOffset = 360.0f * value; break;
    default: break;
  }
}

float AnimPhasor::getParameter(int index) {
  switch (index) {
    case 0: return mHueOffset;
    default: return -1.0f;
  }
  return 0.0f;
}

void AnimPhasor::_process(double dt) {
  if (mt_c <= _t) {
    mFMaxPrev = mFMaxNext;
    mFMaxNext = 1.0f / (9.0f*md_uniform(_gen) + 1.0f);
    mFMaxNext *= (md_uniform(_gen) > 0.5f) ? 1.0f : -1.0f;

    mHuePrev = mHueNext;
    mHueNext = 360.0f * md_uniform(_gen);

    mt_o = _t;
    mt_c = _t + 12.0;
  }

  float x = 1.0f/(1.0f+expf(-(_t-mt_o-6.0f))); // sigmoid function, [0,1]

  float fMax = lin_scale(x, 0, 1, mFMaxPrev, mFMaxNext);
  float hue = lin_scale(x, 0, 1, mHuePrev, mHueNext);

  const int N = _pixbuf->getNumLeds();
  const float n = (float) N;
  for (int i = 0; i < N; ++i) {
    float f = lin_scale(i, 0, n, mFMin, fMax);
    mPhase[i] += f * dt;
    float y = fabsf(sinf(2.0f * M_PI * mPhase[i]));
    float h = (y >= 0.5f) ? hue : hue+mHueOffset;
    _pixbuf->set_pixel_mhroth_hsl_blend(i, h, 0.9f, 0.8f*y);
  }
}
