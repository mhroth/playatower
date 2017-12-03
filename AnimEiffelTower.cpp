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

#include "AnimEiffelTower.hpp"

// candle-like
#define BASE_COLOR_R (255.0f/255.0f)
#define BASE_COLOR_G (132.0f/255.0f)
#define BASE_COLOR_B (1.0f/255.0f)

AnimEiffelTower::AnimEiffelTower(PixelBuffer *pixbuf) : Animation(pixbuf) {
  t = 0.0;
  mean_flash_time = 10.0f; // seconds
  flash_decay_period = 0.1f; // seconds
  __shimmer_rate = 0.5f;
  __shimmer.resize(pixbuf->getNumLeds(), 1.0f);
  d_uniform = std::uniform_real_distribution<float>(0.0f, 1.0f);
  d_gauss = std::normal_distribution<float>(1.0f, 0.2f);

  pixbuf->fill_rgb(BASE_COLOR_R, BASE_COLOR_G, BASE_COLOR_B);
}

AnimEiffelTower::~AnimEiffelTower() {}

void AnimEiffelTower::setParameter(int index, float value) {
  switch (index) {
    case 0: {
      mean_flash_time = powf(10.0f, lin_scale(value, 0.0f, 1.0f, -2.0f, 2.0f));
      break;
    }
    default: break;
  }
}

void AnimEiffelTower::process(double dt) {
  t += dt;

  float p_flash = 1.0f - expf(-dt/mean_flash_time);
  float r_flash_decay = 1.0f - expf(-dt/flash_decay_period);

  const int N = pixbuf->getNumLeds();
  for (int i = 0; i < N; i++) {
    if (d_uniform(gen) < p_flash) {
      pixbuf->set_pixel_rgb_blend(i, 1.0f, 1.0f, 1.0f);
    } else {
      __shimmer[i] = __shimmer[i]*__shimmer_rate + d_gauss(gen)*(1.0f-__shimmer_rate);
      float r = BASE_COLOR_R * __shimmer[i];
      float g = BASE_COLOR_G * __shimmer[i];
      float b = BASE_COLOR_B * __shimmer[i];
      pixbuf->set_pixel_rgb_blend(i, r, g, b, r_flash_decay, PixelBuffer::BlendMode::ADD);
    }
  }

  ++step;
}
