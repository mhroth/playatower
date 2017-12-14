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

#include "Animation.hpp"

Animation::Animation(PixelBuffer *pixbuf) :
    pixbuf(pixbuf), step(0) {
  assert(pixbuf != nullptr);
}

double Animation::lin_scale(double x, double min_in, double max_in, double min_out, double max_out) {
  return ((x-min_in)/(max_in-min_in))*(max_out-min_out) + min_out;
}

float Animation::log_scale(float x, float min_log, float max_log) {
  assert(x >= 0.0f && x <= 1.0f); // [0,1]
  return powf(10.0f, x*(max_log-min_log) + min_log);
}

float Animation::pdf_normal(float x, float mu, float sigma) {
  assert(sigma >= 0.0f);
  const float a = (x - mu) / sigma;
  return expf(-0.5*a*a) / (sigma*M_SQRT_TAU);
}

float Animation::pdf_logNormal(float x, float mu, float sigma) {
  assert(x > 0.0f);
  assert(sigma > 0.0f);
  const float a = (logf(x) - mu) / sigma;
  return expf(-0.5*a*a) / (x*sigma*M_SQRT_TAU);
}
