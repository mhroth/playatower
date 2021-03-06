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

#ifndef _ANIM_LORENZ_PHASOR_HPP_
#define _ANIM_LORENZ_PHASOR_HPP_

#include <vector>

#include "Animation.hpp"
#include "LorenzOscillator.hpp"

class AnimLorenzPhasor: public Animation {
 public:
  AnimLorenzPhasor(PixelBuffer *pixbuf);
  ~AnimLorenzPhasor();

  void setParameter(int index, float value) override;
  float getParameter(int index) override;

  const char *getName() override { return "LorenzPhasor"; }

 private:
  void _process(double dt) override;

  std::vector<LorenzOscillator> m_oscList;

  std::uniform_real_distribution<double> m_uniform;

  double m_tSwitch;
  float m_lowColour;
  float m_timeDilation;

  double m_minGlobalX, m_maxGlobalX;
  double m_minGlobalY, m_maxGlobalY;
  double m_minGlobalZ, m_maxGlobalZ;
};

#endif // _ANIM_LORENZ_PHASOR_HPP_
