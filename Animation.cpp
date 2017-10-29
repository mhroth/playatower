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

Animation::Animation(uint32_t numLeds, uint8_t global) :
    numLeds(numLeds), global(global & 0x1F), step(0) {
  // nothing to do
}

void Animation::write_pixel_rgb(uint8_t *data, int i, float r, float g, float b) {
  assert(data != nullptr);
  int j = (i+1)*4;
  data[j] = 0xE0 | global;
  data[j+1] = (uint8_t) (fminf(fmaxf(b, 0.0f), 1.0f) * 255.0f); // blue
  data[j+2] = (uint8_t) (fminf(fmaxf(g, 0.0f), 1.0f) * 255.0f); // green
  data[j+3] = (uint8_t) (fminf(fmaxf(r, 0.0f), 1.0f) * 255.0f); // red
}
