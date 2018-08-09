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

#include "AnimReactionDiffusion.hpp"

#define M_TAU 6.283185307179586f

AnimReactionDiffusion::AnimReactionDiffusion(PixelBuffer *pixbuf) : Animation(pixbuf) {
  m_height = _pixbuf->getNumLeds()/2;
  m_width = _pixbuf->getNumLeds();

  m_u_grid = (float *) malloc(2*m_height*m_width*sizeof(float)); // width first
  m_v_grid = (float *) malloc(2*m_height*m_width*sizeof(float));
  m_grid_index = 0;

  // m_d_uniform = std::uniform_real_distribution<float>(0.0f, 1.0f);
  // for (int i = 0; i < m_width*m_height; i++) {
  //   float u = m_d_uniform(_gen);
  //   float v = m_d_uniform(_gen) * (1.0f-u);
  //   m_u_grid[i] = u;
  //   m_v_grid[i] = v;
  // }
  for (int i = 0; i < m_width; i++) {
    for (int j = 0; j < m_height; j++) {
      __set_grid_value(m_u_grid, 0, i, j, 1.0f);
      __set_grid_value(m_v_grid, 0, i, j, 0.0f);
    }
  }
  for (int i = m_width/3; i < 2*m_width/3; i++) {
    for (int j = m_height/3; j < 2*m_height/3; j++) {
      __set_grid_value(m_u_grid, 0, i, j, 0.33f);
      __set_grid_value(m_v_grid, 0, i, j, 0.67f);
    }
  }

  m_scan_freq = 1.0f/20.0f;
}

AnimReactionDiffusion::~AnimReactionDiffusion() {
  free(m_u_grid);
  free(m_v_grid);
}

void AnimReactionDiffusion::_process(double dt) {
  // float f = 0.026f;
  // float k = 0.049f;
  // float r_u = 0.2097f;
  // float r_v = 0.105f;

  // float f = 0.012f;
  // float k = 0.05f;
  // float r_u = 0.01f;
  // float r_v = 0.005f;

  // float f = 0.062f;
  // float k = 0.062f;
  // float r_u = 0.19f;
  // float r_v = 0.09f;

  float f = 0.023f;
  float k = 0.074f;
  float r_u = 0.095f;
  float r_v = 0.03f;


  // process diffusion
  // https://groups.csail.mit.edu/mac/projects/amorphous/GrayScott/
  // https://www.uni-muenster.de/imperia/md/content/physik_tp/lectures/ws2016-2017/num_methods_i/rd.pdf
  for (int i = 0; i < m_width; i++) {
    for (int j = 0; j < m_height; j++) {
      float u_xm1 = __get_grid_value(m_u_grid, m_grid_index, i-1, j  );
      float u_xp1 = __get_grid_value(m_u_grid, m_grid_index, i+1, j  );
      float u     = __get_grid_value(m_u_grid, m_grid_index, i,   j  );
      float u_ym1 = __get_grid_value(m_u_grid, m_grid_index, i,   j-1);
      float u_yp1 = __get_grid_value(m_u_grid, m_grid_index, i,   j+1);
      // float du_xx = u_xm1 - 2.0f*u + u_xp1;
      // float du_yy = u_ym1 - 2.0f*u + u_yp1;
      // float u_div = du_xx + du_yy;
      float u_div = (u_xm1 + u_xp1 + u_ym1 + u_yp1 - 4.0f*u) / 1.0f;

      float v_xm1 = __get_grid_value(m_v_grid, m_grid_index, i-1, j  );
      float v_xp1 = __get_grid_value(m_v_grid, m_grid_index, i+1, j  );
      float v     = __get_grid_value(m_v_grid, m_grid_index, i,   j  );
      float v_ym1 = __get_grid_value(m_v_grid, m_grid_index, i,   j-1);
      float v_yp1 = __get_grid_value(m_v_grid, m_grid_index, i,   j+1);
      // float dv_xx = v_xm1 - 2.0f*v + v_xp1;
      // float dv_yy = v_ym1 - 2.0f*v + v_yp1;
      // float v_div = dv_xx + dv_yy;
      float v_div = (v_xm1 + v_xp1 + v_ym1 + v_yp1 - 4.0f*v) / 1.0f;

      float dt_u = r_u*u_div - u*v*v + f*(1.0f-u);
      float dt_v = r_v*v_div + u*v*v - (f+k)*v;

      // dt = 1.0f;
      u += dt_u*dt;
      v += dt_v*dt;
      u = fminf(1.0f, fmaxf(0.0f, u));
      v = fminf(1.0f-u, fmaxf(0.0f, v));
      __set_grid_value(m_u_grid, m_grid_index^1, i, j, u);
      __set_grid_value(m_v_grid, m_grid_index^1, i, j, v);
    }
  }

  // switch to next grid
  m_grid_index ^= 1;

  // float h = 0.0f;
  // float h = m_height * (1.0f+sinf(M_TAU*m_scan_freq*_t)) / 2.0f;
  float h = m_height / 2.0f;
  // scan grid at height h z
  float h_floor = floorf(h);
  const int h_low = (int) h_floor;
  const int h_high = h_low+1;
  const float frac = h - h_floor;
  for (int i = 0; i < _pixbuf->getNumLeds(); i++) {
    float v_low = __get_grid_value(m_v_grid, m_grid_index, i, h_low);
    float v_high = __get_grid_value(m_v_grid, m_grid_index, i, h_high);
    float v = v_low + frac*(v_high-v_low);
    // _pixbuf->set_pixel_hsl_blend(i, 230.0f, 0.8f, v);
    // _pixbuf->set_pixel_rgb_blend(i, v, v, v);
    // _pixbuf->set_pixel_rgb_blend(i, 0.74f, 0.87f, (1.0f-v));
    _pixbuf->set_pixel_hsl_blend(i, 266.4f, 0.87f, v);
  }
}

float AnimReactionDiffusion::__get_grid_value(float *grid, int grid_index, int x, int y) {
  while (x < 0) x += m_width;
  while (x >= m_width) x -= m_width;
  while (y < 0) y += m_height;
  while (y >= m_height) y -= m_height;

  int i = y*m_height + x;
  return grid[grid_index*m_width*m_height + i];
}

void AnimReactionDiffusion::__set_grid_value(float *grid, int grid_index, int x, int y, float f) {
  while (x < 0) x += m_width;
  while (x >= m_width) x -= m_width;
  while (y < 0) y += m_height;
  while (y >= m_height) y -= m_height;

  int i = y*m_height + x;
  grid[grid_index*m_width*m_height + i] = f;
}
