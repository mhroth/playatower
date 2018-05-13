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

#ifndef _LORENZ_OSCILLATOR_HPP_
#define _LORENZ_OSCILLATOR_HPP_

#include "Animation.hpp"

class LorenzOscillator {
 public:
  LorenzOscillator(double sigma, double rho, double beta, double x=0.0, double y=0.0, double z=0.0);
  ~LorenzOscillator();

  void setPosition(double x, double y, double z) { m_x = x; m_y = y; m_z = z; }

  void setSigma(double sigma) { m_sigma = sigma; }
  void setRho(double rho) { m_rho = rho; }
  void setBeta(double beta) { m_beta = beta; }

  double getSigma() const { return m_sigma; }
  double getRho() const { return m_rho; }
  double getBeta() const { return m_beta; }

  void process(double dt, double *x, double *y, double *z);

  void getRangeX(double *min, double *max) const { *min = m_minX; *max = m_maxX; }
  void getRangeY(double *min, double *max) const { *min = m_minY; *max = m_maxY; }
  void getRangeZ(double *min, double *max) const { *min = m_minZ; *max = m_maxZ; }

  void getVelocity(double *dx, double *dy, double *dz) const { *dx = m_dx; *dy = m_dy; *dz = m_dz; }

 private:
   double m_sigma, m_rho, m_beta;
   double m_x, m_y, m_z;
   double m_dx, m_dy, m_dz;
   double m_minX, m_maxX, m_minY, m_maxY, m_minZ, m_maxZ;
};

#endif // _LORENZ_OSCILLATOR_HPP_
