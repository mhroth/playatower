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

#import "AnimChuaOsc.hpp"

AnimChuaOsc::AnimChuaOsc(uint32_t numLeds, uint8_t global) :
    Animation(numLeds, global) {

  t = 0.0;

  x = 0.1; y = 0.3; z = -0.6;

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;
  min_dx = INFINITY; max_dx = -INFINITY;
  min_dy = INFINITY; max_dy = -INFINITY;
  min_dz = INFINITY; max_dz = -INFINITY;

  rgb_buffer = (float *) malloc(3 * numLeds * sizeof(float));
  memset(rgb_buffer, 0, 3 * numLeds * sizeof(float));
}

AnimChuaOsc::~AnimChuaOsc() {
  free(rgb_buffer);
}

void AnimChuaOsc::process(double dt, uint8_t *data) {
  t += dt;

  // https://en.wikipedia.org/wiki/Multiscroll_attractor
  double a = 36.0;
  double b = 3.0;
  double c = 20.0;

  double osc0 = sin(2.0 * M_PI * (1.0/(30*60.0)) * t);
  double u = lin_scale(osc0, -1.0, 1.0, -15.0, 15.0);

  dx = a * (y - x);
  dy = x - x*z + c*y + u;
  dz = x*y - b*z;

  x += dx * dt;
  y += dy * dt;
  z += dz * dt;

  const float k1_decay = expf(-((float) dt)/300.0f);
  min_x = fmin(min_x*k1_decay, x); max_x = fmax(max_x*k1_decay, x);
  min_y = fmin(min_y*k1_decay, y); max_y = fmax(max_y*k1_decay, y);
  min_z = fmin(min_z*k1_decay, z); max_z = fmax(max_z*k1_decay, z);
  min_dx = fmin(min_dx*k1_decay, dx); max_dx = fmax(max_dx*k1_decay, dx);
  min_dy = fmin(min_dy*k1_decay, dy); max_dy = fmax(max_dy*k1_decay, dy);
  min_dz = fmin(min_dz*k1_decay, dz); max_dz = fmax(max_dz*k1_decay, dz);

  const float k_decay = expf(-((float) dt)/1.0f);
  for (int i = 0; i < 3*numLeds; ++i) {
    rgb_buffer[i] *= k_decay;
  }

  int i_r = lin_scale(x, min_x, max_x, 0, numLeds-1);
  // add_pixel_rgb(i_r, 213/255.0f, 94/255.0f, 0/255.0f, 1.0f*dt);
  double l_x = lin_scale(fabs(dx), 0.0, fmax(fabs(min_dx), fabs(max_dx)), 0.01, 0.55+0.1);
  add_pixel_hsl(i_r, 35.0f, 0.69f, l_x, 1.0f*dt);

  int i_g = lin_scale(y, min_y, max_y, 0, numLeds-1);
  // add_pixel_rgb(i_g, 0/255.0f, 158/255.0f, 115/255.0f, 1.0f*dt);
  double l_y = lin_scale(fabs(dy), 0.0, fmax(fabs(min_dy), fabs(max_dy)), 0.01, 0.48+0.1);
  add_pixel_hsl(i_g, 96.0f, 0.36f, l_y, 1.0f*dt);

  int i_b = lin_scale(z, min_z, max_z, 0, numLeds-1);
  // add_pixel_rgb(i_b, 0/255.0f, 114/255.0f, 178/255.0f, 1.0f*dt);
  double l_z = lin_scale(fabs(dz), 0.0, fmax(fabs(min_dz), fabs(max_dz)), 0.01, 0.48+0.1);
  add_pixel_hsl(i_b, 210.0f, 0.9f /* 0.38f */, l_z, 1.0f*dt);

  for (int i = 0; i < numLeds; ++i) {
    set_pixel_rgb(data, i, rgb_buffer[3*i+0], rgb_buffer[3*i+1], rgb_buffer[3*i+2]);
  }

  ++step;
}

void AnimChuaOsc::add_pixel_rgb(int i, float r, float g, float b, float a) {
  const int j = 3 * i;
  // rgb_buffer[j+0] = (1.0f-a)*rgb_buffer[j+0] + a*r;
  // rgb_buffer[j+1] = (1.0f-a)*rgb_buffer[j+1] + a*g;
  // rgb_buffer[j+2] = (1.0f-a)*rgb_buffer[j+2] + a*b;
  rgb_buffer[j+0] += a*r;
  rgb_buffer[j+1] += a*g;
  rgb_buffer[j+2] += a*b;
}

// http://www.rapidtables.com/convert/color/hsl-to-rgb.htm
void AnimChuaOsc::add_pixel_hsl(int i, float h, float s, float l, float a) {
  assert(h >= 0.0f && h < 360.0f);
  assert(s >= 0.0f && s <= 1.0f);
  l = fmax(0.0, fmin(1.0, l));
  // assert(l >= 0.0f && l <= 1.0f);

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

  add_pixel_rgb(i, r, g, b, a);
}
