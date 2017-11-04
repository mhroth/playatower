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

#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h> // nanosleep, clock_gettime

#include "AnimPhasor.hpp"
#include "AnimLorenzOsc.hpp"
#include "AnimLorenzOscFade.hpp"
#include "AnimChuaOsc.hpp"

#include "tiny_spi.h"

#define SEC_TO_NS 1000000000LL
#define ONE_MHZ 1000000

static volatile bool _keepRunning = true;

static void sigintHandler(int x) {
  printf("Termination signal received.\n"); // handle Ctrl+C
  _keepRunning = false;
}

static void timespec_subtract(struct timespec *result, struct timespec *end, struct timespec *start) {
  if (end->tv_nsec < start->tv_nsec) {
    result->tv_sec = end->tv_sec - start->tv_sec - 1;
    result->tv_nsec = SEC_TO_NS + end->tv_nsec - start->tv_nsec;
  } else {
    result->tv_sec = end->tv_sec - start->tv_sec;
    result->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}

int main(int narg, char **argc) {

  TinySpi tspi;
  struct timespec tick, tock, diff_tick;
  uint32_t global_step = 0; // the current frame index

  // register the SIGINT handler
  signal(SIGINT, &sigintHandler);
  printf("Press Ctrl+C to quit.\n");

  const int NUM_LEDS = (narg > 1) ? atoi(argc[1]) : 0;
  if (NUM_LEDS <= 0) {
    printf("Must have at least one argument indicating the number of LEDs.\n");
    return -1;
  }
  printf("* leds: %i\n", NUM_LEDS);

  const double FPS = (narg > 2) ? atof(argc[2]) : -1.0; // frames per second
  const double SPF = 1.0/FPS; // second per frame
  const uint64_t NS_FRAME = (FPS > 0.0) ? (uint64_t) (1000000000.0/FPS) : 0;
  printf("* fps: %g\n", FPS);

  const uint8_t GLOBAL_BRIGHTNESS = (narg > 3) ? (atoi(argc[3]) & 0x1F) : 31;
  printf("* global: %i/31\n", GLOBAL_BRIGHTNESS);

  // open the SPI interface
  tspi_open(&tspi, "/dev/spidev0.0", ONE_MHZ);

  Animation *anim = new AnimChuaOsc(NUM_LEDS, GLOBAL_BRIGHTNESS);

  // prepare SPI data
  // https://cpldcpu.com/2014/11/30/understanding-the-apa102-superled/
  const int NUM_SPI_TRAILER_BYTES = (NUM_LEDS>>4) + 1;
  const int NUM_SPI_BYTES = 4 + (4*NUM_LEDS) + NUM_SPI_TRAILER_BYTES;
  // TODO(mhroth): unknown why 3x bytes are needed. But otherwise free(spi_data) fails!
  uint8_t *spi_data = (uint8_t *) malloc(3 * NUM_SPI_BYTES * sizeof(uint8_t));
  assert(spi_data != nullptr);

  double dt = 0.0;
  while (_keepRunning) {
    clock_gettime(CLOCK_REALTIME, &tick);

    // calculate animation
    anim->process(dt, spi_data);

    // send LED data via SPI
    // *((uint32_t *) spi_data) = 0; // clear the header
    memset(spi_data + (4*(NUM_LEDS+1)), NUM_SPI_TRAILER_BYTES, 0xFF); // set the trailer
    tspi_write(&tspi, NUM_SPI_BYTES, spi_data);

    clock_gettime(CLOCK_REALTIME, &tock);
    timespec_subtract(&diff_tick, &tock, &tick);
    const int64_t elapsed_ns = (((int64_t) diff_tick.tv_sec) * SEC_TO_NS) + diff_tick.tv_nsec;
    if (elapsed_ns < NS_FRAME) {
      // there is never a need to sleep longer than 1 second (famous last words...)
      diff_tick.tv_sec = 0;
      diff_tick.tv_nsec = (long) (NS_FRAME-elapsed_ns);
      dt = SPF;
      nanosleep(&diff_tick, NULL);
    } else {
      dt = ((double) elapsed_ns) / 1000000000.0;
      if (FPS > 0.0) printf("Warning: frame underrun.\n");
      else if (global_step % 1000 == 0) printf("fps: %0.3ffps\n", 1.0/dt);
    }

    ++global_step;
  }

  // close the SPI interface
  tspi_close(&tspi);

  free(spi_data);

  delete anim;

  return 0;
}
