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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>

#import "AnimLorenzOscFade.hpp"

AnimLorenzOscFade::AnimLorenzOscFade(uint32_t numLeds, uint8_t global) :
    Animation(numLeds, global) {
  sigma = 11.0; // 10.0;
  rho = 28.0;
  beta = 8.0/3.0;

  // random starting position on unit sphere
  srand((unsigned)time(0));
  x = ((double) rand()) / ((double) RAND_MAX);
  y = ((double) rand()) / ((double) RAND_MAX);
  z = ((double) rand()) / ((double) RAND_MAX);
  double norm = sqrt(x*x + y*y + z*z);
  x /= norm; y /= norm; z /= norm;
  printf("AnimLorenzOsc starting point: (%0.3f, %0.3f, %0.3f)\n", x, y, z);

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;

  rgb_buffer = (float *) malloc(3 * numLeds * sizeof(float));
  memset(rgb_buffer, 0, 3 * numLeds * sizeof(float));
}

AnimLorenzOscFade::~AnimLorenzOscFade() {
  free(rgb_buffer);
}

void AnimLorenzOscFade::process(double dt, uint8_t *data) {
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

  const float k_decay = expf(-((float) dt)/1.0f);
  for (int i = 0; i < 3*numLeds; ++i) {
    rgb_buffer[i] *= k_decay;
  }

  int i_r = (int) (((double) (numLeds-1)) * ((x - min_x) / (max_x - min_x)));
  add_pixel_rgb(i_r, 231/255.0f, 34/255.0f, 34/255.0f, 1.0f*dt);

  int i_g = (int) (((double) (numLeds-1)) * ((y - min_y) / (max_y - min_y)));
  add_pixel_rgb(i_g, 231/255.0f, 164/255.0f, 59/255.0f, 1.0f*dt);

  int i_b = (int) (((double) (numLeds-1)) * ((z - min_z) / (max_z - min_z)));
  add_pixel_rgb(i_b, 140/255.0f, 184/255.0f, 197/255.0f, 1.0f*dt);

  for (int i = 0; i < numLeds; ++i) {
    set_pixel_rgb(data, i, rgb_buffer[3*i+0], rgb_buffer[3*i+1], rgb_buffer[3*i+2]);
  }

  ++step;
}

void AnimLorenzOscFade::add_pixel_rgb(int i, float r, float g, float b, float a) {
  const int j = 3 * i;
  // rgb_buffer[j+0] = (1.0f-a)*rgb_buffer[j+0] + a*r;
  // rgb_buffer[j+1] = (1.0f-a)*rgb_buffer[j+1] + a*g;
  // rgb_buffer[j+2] = (1.0f-a)*rgb_buffer[j+2] + a*b;
  rgb_buffer[j+0] += a*r;
  rgb_buffer[j+1] += a*g;
  rgb_buffer[j+2] += a*b;
}
