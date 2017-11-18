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

#import "AnimFirefly.hpp"

#define M_TAU 6.283185307179586f

#include <ctime>

static float rnd() {
  return ((float) rand()) / RAND_MAX;
}

AnimFirefly::AnimFirefly(PixelBuffer *pixbuf) : Animation(pixbuf) {
  t = 0.0;

  const int N = pixbuf->getNumLeds();
  phases = (float *) malloc(N * sizeof(float));
  frequencies = (float *) malloc(N * sizeof(float));
  states = (float *) malloc(N * sizeof(float));
  t_next = (double *) malloc(N * sizeof(double));

  srand((unsigned)time(0));

  memset(t_next, 0, N*sizeof(double));
  memset(phases, 0, N*sizeof(float));
  memset(states, 0, N*sizeof(float));
  memset(t_next, 0, N*sizeof(double));
  for (int i = 0; i < N; i++) {
    phases[i] = rnd(); // phases are normalised to unity
    frequencies[i] = 0.5f*rnd() + 0.5f;
  }
  phases[0] = 0.0f;
  phases[N-1] = 0.0f;
  frequencies[0] = 1.0f;
  frequencies[N-1] = 1.0f;
}

AnimFirefly::~AnimFirefly() {
  free(phases);
  free(frequencies);
  free(states);
}

void AnimFirefly::process(double dt) {
  t += dt;

  const int N = pixbuf->getNumLeds();
  for (int i = 1; i < N-1; ++i) {
    float x = cosf(M_TAU * phases[i]);
    x = powf(x,9);
    float s = fmaxf(0.0f, x);

    float dp = phases[i+1] - phases[i];
    float ds_l = dp - truncf(dp); // [-1, +1]
    if (ds_l > 0.5f) ds_l -= 1.0f;
    else if (ds_l < -0.5f) ds_l += 1.0f; // [-0.5, +0.5]
/*
    dp = phases[i+1] - phases[i];
    float ds_r = dp - truncf(dp);
    if (ds_r > 0.5f) ds_r -= 1.0f;
    else if (ds_r < -0.5f) ds_r += 1.0f;

    float ds = 0.75f*ds_l + 0.25f*ds_r;

    float b = fabsf(ds_l-ds_r);
*/

    float k = expf(-dt/0.5f);
    states[i] = states[i]*k + ds_l*(1.0f-k);


    // float f = frequencies[i] + states[i]/dt; // max frequency change rate is X Hz/s
    // float f = frequencies[i] * (1.0f + 0.1f*states[i]);
    float f = frequencies[i];
    if (ds_l >= 0.0f) f += 0.1f;
    else f -= 0.1f;
    f = fmaxf(0.45f, fminf(1.1f, f)); // freq is limited to [Y,Z] Hz
    frequencies[i] = f;

    phases[i] += frequencies[i] * dt;

    pixbuf->set_pixel_rgb_blend(i, s, 0.0f, 0.0f);
  }

  // update the last oscillator
  phases[0] += frequencies[0] * dt;
  phases[N-1] += frequencies[N-1] * dt;
}
