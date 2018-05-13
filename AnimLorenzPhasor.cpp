/**
 * Copyright (c) 2018, Martin Roth (mhroth@gmail.com)
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

#include <cmath>

#include "AnimLorenzPhasor.hpp"

AnimLorenzPhasor::AnimLorenzPhasor(PixelBuffer *pixbuf) : Animation(pixbuf) {
  m_oscList.reserve(_pixbuf->getNumLeds());
  for (int i = 0; i < _pixbuf->getNumLeds(); i++) {
    m_oscList.push_back(LorenzOscillator(10.0, 28, 8.0/3.0, 1.0+0.01*i, 0.0, 0.0));
  }

  m_normal = std::normal_distribution<double>(0.0, 1.0);

  m_k = 0.0;
}

AnimLorenzPhasor::~AnimLorenzPhasor() {}

void AnimLorenzPhasor::setParameter(int index, float value) {
  m_k = 1000.0f * value;
  for (auto& osc : m_oscList) {
    // TODO:(mhroth)?
  }
}

float AnimLorenzPhasor::getParameter(int index) {
  return m_k;
}

void AnimLorenzPhasor::_process(double dt) {
  const int NUM_LEDS = _pixbuf->getNumLeds();
  double x, y, z;
  double dx, dy, dz;
  double minX, maxX, minY, maxY, minZ, maxZ;
  for (int i = 0; i < NUM_LEDS; i++) {
    m_oscList[i].process(dt, &x, &y, &z);
    m_oscList[i].getRangeX(&minX, &maxX);
    m_oscList[i].getRangeY(&minY, &maxY);
    m_oscList[i].getRangeZ(&minZ, &maxZ);

    m_oscList[i].getVelocity(&dx, &dy, &dz);

    x = lin_scale(x, 0.99*minX, 1.01*maxX); // constants are to prevent case of minX == maxX
    y = lin_scale(y, 0.99*minY, 1.01*maxY);
    z = lin_scale(z, 0.99*minZ, 1.01*maxZ);

    _pixbuf->set_pixel_rgb_blend(i, x, y, z);
  }
}
