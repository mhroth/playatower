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

#import "AnimLorenzOsc.hpp"

AnimLorenzOsc::AnimLorenzOsc(uint32_t numLeds, uint8_t global) :
    Animation(numLeds, global) {
  // nothing to do
}

AnimLorenzOsc::~AnimLorenzOsc() {
  // nothing to do
}

void AnimLorenzOsc::process(float dt, uint8_t *data) {
  for (int i = 0; i < numLeds; ++i) {
    write_pixel_rgb(data, i, 1.0f, 0.5f, 0.25f);
  }
}
