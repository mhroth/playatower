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

#include <assert.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <ifaddrs.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <time.h> // nanosleep, clock_gettime
#include <unistd.h> // for close

#include "HvLightPipe.h"
#include "tinyosc.h"
#include "tiny_spi.h"

#include "PixelBuffer.hpp"

#include "AnimPhasor.hpp"
#include "AnimLorenzOsc.hpp"
#include "AnimLorenzOscFade.hpp"
#include "AnimChuaOsc.hpp"
#include "AnimAllWhite.hpp"
#include "AnimLighthouse.hpp"
#include "AnimEiffelTower.hpp"
#include "AnimXmasPhasor.hpp"
#include "AnimRandomFlow.hpp"

#define SEC_TO_NS 1000000000LL
#define SPI_HZ 2*1000000
#define GPIO_INPUT_PIN 2


// https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
#define BCM2708_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) // GPIO controller
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
static volatile unsigned int *gpio;
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7) // sets bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

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

// Set up a memory regions to access GPIO
void gpio_open() {
  // open /dev/mem (requires sudo)
  // open /dev/gpiomem (does not require sudo)
  int fd = open("/dev/gpiomem", O_RDWR|O_SYNC);
  assert(fd > 0 && "Can't open /dev/gpiomem");

  // mmap GPIO
  void *gpio_map = mmap(
    NULL,                 // Any adddress in our space will do
    BLOCK_SIZE,           // Map length
    PROT_READ|PROT_WRITE, // Enable reading & writting to mapped memory
    MAP_SHARED,           // Shared with other processes
    fd,                   // File to map
    GPIO_BASE             // Offset to GPIO peripheral
  );

  close(fd); // No need to keep fd open after mmap

  if (gpio_map == MAP_FAILED) {
    printf("mmap error %d\n", (int)gpio_map); // errno also set!
    exit(-1);
  }

  // always use volatile pointer
  gpio = (volatile unsigned *) gpio_map;
}

// declare the network run function
static void *network_run(void *q);

int main(int narg, char **argc) {

  TinySpi tspi;
  struct timespec tick, tock, diff_tick;
  uint32_t global_step = 0; // the current frame index
  uint64_t total_elapsed_ns = 0;
  uint64_t next_print_ns = 0;

  // register signal handlers
  signal(SIGINT, &sigintHandler); // SIGINT (Crtl+C)
  signal(SIGTERM, &sigintHandler); // SIGTERM (kill pid)
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
  tspi_open(&tspi, "/dev/spidev0.0", SPI_HZ);

  // open the GPIO interface
  gpio_open();
  INP_GPIO(GPIO_INPUT_PIN); // configure GPIO pin as input

  PixelBuffer *pixbuf = new PixelBuffer(NUM_LEDS, GLOBAL_BRIGHTNESS);
  printf("* SPI buffer: %i [%i] bytes\n", pixbuf->getNumSpiBytes(), pixbuf->getNumSpiBytesTotal());
  printf("\n");

  Animation *anim = new AnimPhasor(pixbuf); // initialise with default animation

  if (MAX_WATTS > 0.0f) {
    // if maximum watts is specified, reset global brightness so that over wattage can happen
    uint8_t global = (uint8_t) (31.0f*MAX_WATTS/pixbuf->getMaxWatts());
    pixbuf->setGlobal(global);
  }

  // start the network thread (with pipe)
  HvLightPipe pipe;
  hLp_init(&pipe, 4*1024); // 4KB pipe
  pthread_t networkThread = 0;
  pthread_create(&networkThread, NULL, &network_run, &pipe);

  // default to no nightshift
  float nightshift = 0.0f;

  int lastButtonState = 1; // GPIO pin is high when *not* connected
  uint32_t anim_index = 0;
  bool toNextAnim = false;

  double dt = 0.0;
  while (_keepRunning) {

    clock_gettime(CLOCK_REALTIME, &tick);

    // check the state of the button
    int currentButtonState = GET_GPIO(GPIO_INPUT_PIN);
    if (lastButtonState != 0 && currentButtonState == 0) {
      toNextAnim = true;
    }
    lastButtonState = currentButtonState;

    // read messages from network
    while (hLp_hasData(&pipe)) {
      uint32_t numBytes = 0;
      uint8_t *osc_buffer = hLp_getReadBuffer(&pipe, &numBytes);
      assert(numBytes != 0);
      assert(osc_buffer != nullptr);
      tosc_message osc;
      if (!tosc_parseMessage(&osc, (char *) osc_buffer, numBytes)) {
        if (!strcmp(tosc_getAddress(&osc), "/next")) {
          toNextAnim = true;
        } else if (!strcmp(tosc_getAddress(&osc), "/global")) {
          pixbuf->setGlobal((int32_t) tosc_getNextFloat(&osc));
        } else if (!strcmp(tosc_getAddress(&osc), "/nightshift")) {
          nightshift = tosc_getNextFloat(&osc); // update nightshift
        } else if (!strncmp(tosc_getAddress(&osc), "/param/", 7)) {
          // e.g. /param/0 0.5
          int index = atoi(tosc_getAddress(&osc)+7); // parameter index >= 0
          float value = tosc_getNextFloat(&osc); // parameter value [0,1]
          anim->setParameter(index, value);
        }
      }
      hLp_consume(&pipe); // consume the message from the pipe
    }

    // check if we need to move to the next animation
    if (toNextAnim) {
      toNextAnim = false;

      // on button press
      delete anim;     // delete the existing animation
      pixbuf->clear(); // clear the pixel buffer

      // instantiate the next animation
      anim_index = (anim_index+1) % 8;
      switch (anim_index) {
        default:
        case 0: anim = new AnimPhasor(pixbuf); break;
        case 1: anim = new AnimXmasPhasor(pixbuf); break;
        case 2: anim = new AnimLorenzOsc(pixbuf); break;
        case 3: anim = new AnimLorenzOscFade(pixbuf); break;
        case 4: anim = new AnimChuaOsc(pixbuf); break;
        case 5: anim = new AnimLighthouse(pixbuf); break;
        case 6: anim = new AnimEiffelTower(pixbuf); break;
        case 7: anim = new AnimRandomFlow(pixbuf); break;
      }

      // FPS = anim->getPreferredFps();
      dt = 0.0;
    }

    // calculate animation
    anim->process(dt);

    // send LED data via SPI
    tspi_write(&tspi, pixbuf->getNumSpiBytes(), pixbuf->getSpiBytes(nightshift));

    clock_gettime(CLOCK_REALTIME, &tock);
    timespec_subtract(&diff_tick, &tock, &tick);
    const uint64_t elapsed_ns = (((uint64_t) diff_tick.tv_sec) * SEC_TO_NS) + (uint64_t) diff_tick.tv_nsec;
    if (elapsed_ns < NS_FRAME) {
      // NOTE(mhroth): there is never a need to sleep longer than 1 second (famous last words...)
      diff_tick.tv_sec = 0;
      diff_tick.tv_nsec = (long) (NS_FRAME-elapsed_ns);
      dt = SPF;
      nanosleep(&diff_tick, NULL);
    } else {
      dt = ((double) elapsed_ns) / 1000000000.0;
      if (FPS > 0.0) printf("Warning: frame underrun.\n");
    }

    // track total elapsed time
    total_elapsed_ns += elapsed_ns;

    // print info every half second
    if (total_elapsed_ns > next_print_ns) {
      next_print_ns += SEC_TO_NS/2;

      const float amps = pixbuf->getAmperes();
      printf("\r| %6.1f fps | %7.3f Watts (%4.1f%%) [%4.1f Amps] | %9i frames | %2i global | %0.3f nightshift | %.32s | [%g]",
          1.0/dt, 5.0f*amps, 100.0f*5.0f*amps/pixbuf->getMaxWatts(), amps,
          global_step, pixbuf->getGlobal(), nightshift, anim->getName(), anim->getParameter(0));
      fflush(stdout);
    }

    ++global_step;
  }

  // turn off all LEDs
  pixbuf->clear();
  tspi_write(&tspi, pixbuf->getNumSpiBytes(), pixbuf->getSpiBytes(0.0f));

  munmap((void *) gpio, BLOCK_SIZE); // unmap the gpio memory
  pthread_join(networkThread, NULL); // wait for the network thread to stop
  hLp_free(&pipe); // destroy the pipe from the network thread to the main thread
  tspi_close(&tspi); // close the SPI interface
  delete anim; // delete the animation
  delete pixbuf; // delete the pixel buffer

  return 0;
}

void *network_run(void *q) {
  assert(q != nullptr);

  // open receive socket
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd > 0);
  fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2018); // port 2018
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));

  while (_keepRunning) {
    // set up structs for select
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // wait up to X seconds for new packet
    struct timeval tv = {0, 200000}; // sec, usec

    if (select(fd+1, &rfds, NULL, NULL, &tv) > 0) {
      uint8_t network_buffer[1024]; // buffer into which network data is received
      struct sockaddr_in sin;
      int len = 0;
      int sa_len = sizeof(struct sockaddr_in);

      while ((len = recvfrom(fd, network_buffer, sizeof(network_buffer), 0, (struct sockaddr *) &sin, (socklen_t *) &sa_len)) > 0) {
        // put message on pipe
        uint8_t *pipe_buffer = hLp_getWriteBuffer((HvLightPipe *) q, len);
        assert(pipe_buffer != nullptr);
        memcpy(pipe_buffer, network_buffer, len);
        hLp_produce((HvLightPipe *) q, len);
      }
    }
  }

  // close the receive socket
  close(fd);

  return NULL;
}
