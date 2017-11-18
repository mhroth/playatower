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

#include "tiny_spi.h"

#include "PixelBuffer.hpp"

#include "AnimPhasor.hpp"
#include "AnimLorenzOsc.hpp"
#include "AnimLorenzOscFade.hpp"
#include "AnimChuaOsc.hpp"
#include "AnimFirefly.hpp"

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

static void *network_run(void *x);

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

  const float MAX_WATTS = (narg > 4) ? atof(argc[4]) : -1.0f;
  printf("* max. watts: %0.3f\n", MAX_WATTS);
  printf("\n");

  // open the SPI interface
  tspi_open(&tspi, "/dev/spidev0.0", ONE_MHZ);

  PixelBuffer *pixbuf = new PixelBuffer(NUM_LEDS, GLOBAL_BRIGHTNESS);
  Animation *anim = new AnimChuaOsc(pixbuf); // initialise with default animation

  if (MAX_WATTS > 0.0f) {
    // if maximum watts is specified, reset global brightness so that over wattage can happen
    uint8_t global = (uint8_t) (31.0f*MAX_WATTS/pixbuf->getMaxWatts());
    pixbuf->setGlobal(global);
  }

  // bool lastButtonState = 0;
  // uint32_t anim_index = 0;

  double dt = 0.0;
  while (_keepRunning) {
/*
    bool currentButtonState = GET_GPIO();
    if (lastButtonState == 0 && currentButtonState == 1) {
      // on button press
      delete anim;     // delete the existing animation
      pixbuf->clear(); // clear the pixel buffer

      // instantiate the next animation
      anim_index = (anim_index+1) % 4;
      switch (anim_index) {
        default:
        case 0: anim = new AnimPhasor(pixbuf); break;
        case 1: anim = new AnimLorenzOsc(pixbuf); break;
        case 2: anim = new AnimLorenzOscFade(pixbuf); break;
        case 3: anim = new AnimChuaOsc(pixbuf); break;
      }

      FPS = anim->getPreferredFps();
      dt = 0.0;
    }
    lastButtonState = currentButtonState;
*/

    clock_gettime(CLOCK_REALTIME, &tick);

    // calculate animation
    anim->process(dt);

    // send LED data via SPI
    tspi_write(&tspi, pixbuf->getNumSpiBytes(), pixbuf->getSpiBytes());

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
      else if (global_step % 1000 == 0) {
        const float watts = pixbuf->getWatts();
        printf("\r| %6.1f fps | %7.3f Watts (%4.1f%%) | %9i frames | %2i global |",
            1.0/dt, watts, 100.0f*watts/pixbuf->getMaxWatts(), global_step, pixbuf->getGlobal());
        fflush(stdout);
      }
    }

    ++global_step;
  }

  tspi_close(&tspi); // close the SPI interface
  delete anim; // delete the animation
  delete pixbuf; // delete the pixel buffer

  return 0;
}
/*
void *network_run(void *x) {
  // open receive socket
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd > 0);
  fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2018); // port 2018
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));

  // set up structs for select
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd_receive, &rfds);

  // wait up to 1 second for new packet
  struct timeval tv = {1, 0};

  if (select(fd_receive+1, &rfds, NULL, NULL, &tv) > 0) {

    uint8_t buffer[1024]; // buffer into which network data is received
    struct sockaddr_in sin;
    int len = 0;
    int sa_len = sizeof(struct sockaddr_in);
    tosc_message osc;

    while ((len = recvfrom(fd_receive, buffer, sizeof(buffer), 0, (struct sockaddr *) &sin, (socklen_t *) &sa_len)) > 0) {
      // TODO(mhroth): put message on pipe

      if (!tosc_parseMessage(&osc, buffer, len)) {
        if (!strcmp(tosc_getAddress(&osc), "/next")) {
          // TODO(mhroth): trigger next animation
        } else if (!strcmp(tosc_getAddress(&osc), "/global")) {
          int32_t global = tosc_getNextInt(&osc);
          (uint8_t) (global & 0x1F);
        } else if (!strcmp(tosc_getAddress(&osc), "/nightshift")) {
          float nightshift = tosc_getNextFloat(&osc);
        }
      }
    }
  }

  // close the socket on exit
  close(fd);

  return nullptr;
}
*/