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

#ifndef _ANIM_REACTION_DIFFUSION_HPP_
#define _ANIM_REACTION_DIFFUSION_HPP_

#include "Animation.hpp"

class AnimReactionDiffusion: public Animation {
 public:
  AnimReactionDiffusion(PixelBuffer *pixbuf);
  ~AnimReactionDiffusion();

  const char *getName() override { return "Reaction Diffusion"; }

 private:
  void _process(double dt) override;
  // void __sample_grid(float );
  float __get_grid_value(float *grid, int grid_index, int x, int y);
  void __set_grid_value(float *grid, int grid_index, int x, int y, float f);

  int m_height;
  int m_width;
  float *m_u_grid;
  float *m_v_grid;
  int m_grid_index;
  std::uniform_real_distribution<float> m_d_uniform;

  float m_scan_freq; // Hz
};

#endif // _ANIM_REACTION_DIFFUSION_HPP_
