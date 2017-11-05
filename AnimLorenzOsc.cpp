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

#include <stdlib.h>
#include <ctime>

#import "AnimLorenzOsc.hpp"

AnimLorenzOsc::AnimLorenzOsc(PixelBuffer *pixbuf) :
    Animation(pixbuf) {
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
  printf("AnimLorenzOsc starting point: (%0.3f, %0.3f, %0.3f)\n", x, y, z);

  min_x = INFINITY; max_x = -INFINITY;
  min_y = INFINITY; max_y = -INFINITY;
  min_z = INFINITY; max_z = -INFINITY;
}

AnimLorenzOsc::~AnimLorenzOsc() {
  // nothing to do
}

void AnimLorenzOsc::process(double dt) {
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

  // clear the pixel data
  pixbuf->clear();

  int i_r = (int) (((double) (pixbuf->getNumLeds()-1)) * ((x - min_x) / (max_x - min_x)));
  pixbuf->set_pixel_rgb(i_r, 1.0f, 0.0f, 0.0f);

  int i_g = (int) (((double) (pixbuf->getNumLeds()-1)) * ((y - min_y) / (max_y - min_y)));
  pixbuf->set_pixel_rgb(i_g, 0.0f, 1.0f, 0.0f);

  int i_b = (int) (((double) (pixbuf->getNumLeds()-1)) * ((z - min_z) / (max_z - min_z)));
  pixbuf->set_pixel_rgb(i_b, 0.0f, 0.0f, 1.0f);

  ++step;
}
