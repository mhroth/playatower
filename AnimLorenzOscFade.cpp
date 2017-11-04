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

  t = 0.0;

  sigma = 10.0;
  rho = 28.0;
  beta = 8.0/3.0;

  // random starting position on unit sphere
  srand((unsigned)time(0));
  x = ((double) rand()) / ((double) RAND_MAX);
  y = ((double) rand()) / ((double) RAND_MAX);
  z = ((double) rand()) / ((double) RAND_MAX);
  double norm = sqrt(x*x + y*y + z*z);
  x /= norm; y /= norm; z /= norm;

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;
  min_dx = INFINITY; max_dx = -INFINITY;
  min_dy = INFINITY; max_dy = -INFINITY;
  min_dz = INFINITY; max_dz = -INFINITY;

  rgb_buffer = (float *) malloc(3 * numLeds * sizeof(float));
  memset(rgb_buffer, 0, 3 * numLeds * sizeof(float));
}

AnimLorenzOscFade::~AnimLorenzOscFade() {
  free(rgb_buffer);
}

void AnimLorenzOscFade::process(double dt, uint8_t *data) {
  t += dt;

  double osc0 = sin(2.0 * M_PI * (1.0/(10*60.0)) * t);
  double osc1 = sin(2.0 * M_PI * (1.0/(2.5*60.0)) * t);
  double osc2 = sin(2.0 * M_PI * (1.0/(14*60.0)) * t);
  sigma = lin_scale(osc0, -1.0, 1.0, 5.0, 15.0);
  rho = lin_scale(osc1, -1.0, 1.0, 24.0, 36.0);
  beta = lin_scale(osc2, -1.0, 1.0, 2.0, 3.33);

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
  min_dx = fmin(min_dx, dx); max_dx = fmax(max_dx, dx);
  min_dy = fmin(min_dy, dy); max_dy = fmax(max_dy, dy);
  min_dz = fmin(min_dz, dz); max_dz = fmax(max_dz, dz);

  const float k_decay = expf(-((float) dt)/1.0f);
  for (int i = 0; i < 3*numLeds; ++i) {
    rgb_buffer[i] *= k_decay;
  }

  int i_r = lin_scale(x, min_x, max_x, 0, numLeds-1);
  // add_pixel_rgb(i_r, 213/255.0f, 94/255.0f, 0/255.0f, 1.0f*dt);
  double l_x = lin_scale(fabs(dx), 0.0, fmax(fabs(min_dx), fabs(max_dx)), 0.05, 0.84);
  add_pixel_hsl(i_r, 26.0f, 1.0f, l_x, 1.0f*dt);

  int i_g = lin_scale(y, min_y, max_y, 0, numLeds-1);
  // add_pixel_rgb(i_g, 0/255.0f, 158/255.0f, 115/255.0f, 1.0f*dt);
  double l_y = lin_scale(fabs(dy), 0.0, fmax(fabs(min_dy), fabs(max_dy)), 0.05, 0.62);
  add_pixel_hsl(i_g, 164.0f, 1.0f, l_y, 1.0f*dt);

  int i_b = lin_scale(z, min_z, max_z, 0, numLeds-1);
  // add_pixel_rgb(i_b, 0/255.0f, 114/255.0f, 178/255.0f, 1.0f*dt);
  double l_z = lin_scale(fabs(dz), 0.0, fmax(fabs(min_dz), fabs(max_dz)), 0.05, 0.7);
  add_pixel_hsl(i_b, 202.0f, 1.0f, l_z, 1.0f*dt);

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

// http://www.rapidtables.com/convert/color/hsl-to-rgb.htm
void AnimLorenzOscFade::add_pixel_hsl(int i, float h, float s, float l, float a) {
  assert(h >= 0.0f && h < 360.0f);
  assert(s >= 0.0f && s <= 1.0f);
  assert(l >= 0.0f && l <= 1.0f);

  float C = (1.0f - fabsf(2.0f*l-1.0f)) * s;
  float X = ((((int) (h/60.0f)) % 2) == 0) ? C : 0.0f;
  float m = l - C*0.5f;

  float r, g, b;
  if (h < 60.0f) {
    r = C+m; g = X+m; b = 0.0f+m;
  } else if (h < 120.0f) {
    r = X+m; g = C+m; b = 0.0f+m;
  } else if (h < 180.0f) {
    r = 0.0f+m; g = C+m; b = X+m;
  } else if (h < 240.0f) {
    r = 0.0f+m; g = X+m; b = C+m;
  } else if (h < 300.0f) {
    r = X+m; g = 0.0f+m; b = C+m;
  } else {
    r = C+m; g = 0.0f+m; b = X+m;
  }

  const int j = 3 * i;
  rgb_buffer[j+0] += a*r;
  rgb_buffer[j+1] += a*g;
  rgb_buffer[j+2] += a*b;
}
