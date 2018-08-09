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

#define RESET_PERIOD_SEC 75
#define FADE_PERIOD_SEC 1

AnimLorenzPhasor::AnimLorenzPhasor(PixelBuffer *pixbuf) : Animation(pixbuf) {
  m_oscList.reserve(_pixbuf->getNumLeds());
  for (int i = 0; i < _pixbuf->getNumLeds(); i++) {
    m_oscList.push_back(LorenzOscillator(10.0, 28, 8.0/3.0));
  }

  m_uniform = std::uniform_real_distribution<double>(0.0, 1.0);
  m_tSwitch = 0.0;
  m_timeDilation = 0.43f;

  // m_minGlobalX = INFINITY; m_maxGlobalX = -INFINITY;
  // m_minGlobalY = INFINITY; m_maxGlobalY = -INFINITY;
  // m_minGlobalZ = INFINITY; m_maxGlobalZ = -INFINITY;
  m_minGlobalX = -21.0; m_maxGlobalX = 21.0; // a good guess on the limits
  m_minGlobalY = -30.0; m_maxGlobalY = 30.0;
  m_minGlobalZ = -3.0; m_maxGlobalZ = 50.0;
}

AnimLorenzPhasor::~AnimLorenzPhasor() {}

void AnimLorenzPhasor::setParameter(int index, float value) {
  m_timeDilation = log_scale(value, -1.0f, 1.0f);
}

float AnimLorenzPhasor::getParameter(int index) {
  return m_timeDilation;
}

void AnimLorenzPhasor::_process(double dt) {
  if (m_tSwitch <= _t) {
    m_tSwitch += RESET_PERIOD_SEC;

    // determine new direction
    double az = 2.0 * M_PI * m_uniform(_gen);
    double el = (M_PI * m_uniform(_gen)) - M_PI_2;

    double x = cos(el) * cos(az);
    double y = cos(el) * sin(az);
    double z = sin(el);

    // double rho = m_oscList[0].getRho();
    // double beta = m_oscList[0].getBeta();
    // double sigma = m_oscList[0].getSigma();
    //
    // // determine attractor point
    // double xa = sqrt(beta * (rho  -1.0));
    // double ya = sqrt(beta * (rho - 1.0));
    // double za = rho - 1.0;

    double r = 1.0; // r ranges from [1,3]
    const int N = m_oscList.size();
    for (int i = 0; i < N; i++) {
      r += 2.0/N;
      m_oscList[i].setPosition(r*x, r*y, r*z);
    }

    m_lowColour =  m_uniform(_gen) * 360;

    // reset osc limits
    // m_minGlobalX = INFINITY; m_maxGlobalX = -INFINITY;
    // m_minGlobalY = INFINITY; m_maxGlobalY = -INFINITY;
    // m_minGlobalZ = INFINITY; m_maxGlobalZ = -INFINITY;
  }

  dt *= m_timeDilation;

  const int NUM_LEDS = _pixbuf->getNumLeds();
  double x, y, z;
  double minX, maxX, minY, maxY, minZ, maxZ;
  for (int i = 0; i < NUM_LEDS; i++) {
    m_oscList[i].process(dt, &x, &y, &z);

    m_minGlobalX = fmin(m_minGlobalX, x); m_maxGlobalX = fmax(m_maxGlobalX, x);
    m_minGlobalY = fmin(m_minGlobalY, y); m_maxGlobalY = fmax(m_maxGlobalY, y);
    m_minGlobalZ = fmin(m_minGlobalZ, z); m_maxGlobalZ = fmax(m_maxGlobalZ, z);

    // NOTE:(mhroth) constants are to prevent case of minX == maxX
    // x = lin_scale(x, 0.99*m_minGlobalX, 1.01*m_maxGlobalX, m_lowColour, m_lowColour+60);
    // x = lin_scale(x, 0.99*m_minGlobalX, 1.01*m_maxGlobalX, m_lowColour, m_lowColour+90);
    // y = lin_scale(y, 0.99*m_minGlobalY, 1.01*m_maxGlobalY);
    // z = lin_scale(z, 0.99*m_minGlobalZ, 1.01*m_maxGlobalZ);
    x = lin_scale(x, m_minGlobalX, m_maxGlobalX, m_lowColour, m_lowColour+90);
    y = lin_scale(y, m_minGlobalY, m_maxGlobalY);
    z = lin_scale(z, m_minGlobalZ, m_maxGlobalZ);

    _pixbuf->set_pixel_mhroth_hsl_blend(i, x, y, z);
  }

  double tt = m_tSwitch - _t;
  if (tt < FADE_PERIOD_SEC) {
    float gain = sinf(M_PI_2*(tt/FADE_PERIOD_SEC)); // fade out
    _pixbuf->apply_gain(gain);
  } else if (tt > (RESET_PERIOD_SEC-FADE_PERIOD_SEC)) {
    float gain = sinf(M_PI_2*(RESET_PERIOD_SEC - tt)/FADE_PERIOD_SEC); // fade in
    _pixbuf->apply_gain(gain);
  }
}
