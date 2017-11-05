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

AnimLorenzOscFade::AnimLorenzOscFade(PixelBuffer *pixbuf) :
    Animation(pixbuf) {

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
  max_speed = -INFINITY;

  c_h = 360.0 * ((double) rand()) / ((double) RAND_MAX);
  c_s = (0.8-0.2) * ((double) rand()) / ((double) RAND_MAX) + 0.2;
  // c_l = (0.6-0.2) * ((double) rand()) / ((double) RAND_MAX) + 0.2;
  c_l = 0.5;
}

AnimLorenzOscFade::~AnimLorenzOscFade() {
  // nothing to do
}

void AnimLorenzOscFade::process(double dt) {
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

  double speed = sqrt(dx*dx + dy*dy + dz*dz);
  max_speed = fmax(max_speed, speed);

  const float k_decay = expf(-((float) dt)/1.0f);
  pixbuf->apply_gain(k_decay);

  const int N = pixbuf->getNumLeds();

  c_h = lin_scale(speed, 0.0, max_speed, 0.0, 360.0);

  int i_r = lin_scale(x, min_x, max_x, 0, N-1);
  // add_pixel_rgb(i_r, 213/255.0f, 94/255.0f, 0/255.0f, 1.0f*dt);
  double l_x = lin_scale(fabs(dx), 0.0, fmax(fabs(min_dx), fabs(max_dx)), 0.05, c_l);
  pixbuf->add_pixel_hsl(i_r, c_h, c_s, l_x, 30.0f*dt);

  int i_g = lin_scale(y, min_y, max_y, 0, N-1);
  // add_pixel_rgb(i_g, 0/255.0f, 158/255.0f, 115/255.0f, 1.0f*dt);
  double l_y = lin_scale(fabs(dy), 0.0, fmax(fabs(min_dy), fabs(max_dy)), 0.05, c_l);
  pixbuf->add_pixel_hsl(i_g, c_h+150.0, c_s, l_y, 30.0f*dt);

  int i_b = lin_scale(z, min_z, max_z, 0, N-1);
  // add_pixel_rgb(i_b, 0/255.0f, 114/255.0f, 178/255.0f, 1.0f*dt);
  double l_z = lin_scale(fabs(dz), 0.0, fmax(fabs(min_dz), fabs(max_dz)), 0.05, c_l);
  pixbuf->add_pixel_hsl(i_b, c_h-150.0, c_s, l_z, 30.0f*dt);

  ++step;
}
