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

#include "LorenzOscillator.hpp"

LorenzOscillator::LorenzOscillator(double sigma, double rho, double beta, double x, double y, double z) {
  m_sigma = sigma; m_rho = rho; m_beta = beta;
  m_x = x; m_y = y; m_z = z;
  m_dx = 0.0; m_dy = 0.0; m_dz = 0.0;
  m_minX = INFINITY; m_minY = INFINITY; m_minZ = INFINITY;
  m_maxX = -INFINITY; m_maxY = -INFINITY; m_maxZ = -INFINITY;
}

LorenzOscillator::~LorenzOscillator() {}

void LorenzOscillator::process(double dt, double *x, double *y, double *z) {
  m_dx = m_sigma * (m_y - m_x);
  m_dy = (m_x * (m_rho - m_z)) - m_y;
  m_dz = m_x*m_y - m_beta*m_z;

  m_x += m_dx * dt;
  m_y += m_dy * dt;
  m_z += m_dz * dt;

  m_minX = fmin(m_minX, m_x); m_minY = fmin(m_minY, m_y); m_minZ = fmin(m_minZ, m_z);
  m_maxX = fmax(m_maxX, m_x); m_maxY = fmax(m_maxY, m_y); m_maxZ = fmax(m_maxZ, m_z);

  *x = m_x;
  *y = m_y;
  *z = m_z;
}
