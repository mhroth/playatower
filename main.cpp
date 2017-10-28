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

#include "AnimLorenzOsc.hpp"

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

  // register the SIGINT handler
  signal(SIGINT, &sigintHandler);
  printf("Press Ctrl+C to quit.\n");

  const int NUM_LEDS = (narg > 1) ? atoi(argc[1]) : 0;
  if (NUM_LEDS <= 0) {
    printf("Must have at least one argument indicating the number of LEDs.\n");
    return -1;
  }
  printf("* leds: %i\n", NUM_LEDS);

  const double FPS = (narg > 2) ? atof(argc[2]) : -1.0;
  const uint64_t NS_FRAME = (FPS > 0.0) ? (uint64_t) (1000000000.0/FPS) : 0;
  printf("* fps: %g\n", FPS);

  const uint8_t GLOBAL_BRIGHTNESS = (narg > 3) ? (atoi(argc[3]) | 0x1F) : 31;
  printf("* global: %i/31\n", GLOBAL_BRIGHTNESS);

  // open the SPI interface
  tspi_open(&tspi, "/dev/spidev0.0", ONE_MHZ);

  Animation *anim = new AnimLorenzOsc(NUM_LEDS, GLOBAL_BRIGHTNESS);

  // prepare SPI data
  // https://cpldcpu.com/2014/11/30/understanding-the-apa102-superled/
  const int NUM_SPI_BYTES = 4 + (4*NUM_LEDS) + ((NUM_LEDS/2/8) + 1);
  uint8_t *spi_data = (uint8_t *) malloc(NUM_SPI_BYTES * sizeof(uint8_t));
  assert(spi_data != nullptr);

  float dt = 0.0f;
  while (_keepRunning) {
    clock_gettime(CLOCK_REALTIME, &tick);

    // calculate animation
    anim->process(dt, spi_data);
    // for (int i = 0; i < NUM_LEDS; i++) {
    //   printf("0x%X ", ((uint32_t *) spi_data)[i+1]);
    // }
    // printf("\n");

    // send SPI
    memset(spi_data, 4, 0x00); // header
    memset(spi_data + (4*(NUM_LEDS+1)), (NUM_LEDS/2/8)+1, 0xFF); // trailer
    tspi_write(&tspi, NUM_SPI_BYTES, spi_data);

    clock_gettime(CLOCK_REALTIME, &tock);
    timespec_subtract(&diff_tick, &tock, &tick);
    const int64_t elapsed_ns = (((int64_t) diff_tick.tv_sec) * SEC_TO_NS) + diff_tick.tv_nsec;
    // printf("Frame prep-time: %gms\n", ((double) elapsed_ns)/1000000.0);
    dt = 0.0f;
    if (elapsed_ns < NS_FRAME) {
      // there is never a need to sleep longer than 1 second (famous last words...)
      diff_tick.tv_sec = 0;
      diff_tick.tv_nsec = (long) (NS_FRAME-elapsed_ns);
      dt = (float) (((double) diff_tick.tv_nsec) / 1000000000.0);
      nanosleep(&diff_tick, NULL);
    }
  }

  // close the SPI interface
  tspi_close(&tspi);

  free(spi_data);

  delete anim;

  return 0;
}
