/**
 * Copyright (c) 2017-2018, Martin Roth (mhroth@gmail.com)
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
#include <unistd.h> // for close and execl

#include "tinypipe.h"
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
#include "AnimRain.hpp"
#include "AnimReactionDiffusion.hpp"
#include "AnimLorenzPhasor.hpp"

#define SEC_TO_NS 1000000000LL
#define SPI_HZ 9000000
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
    printf("mmap error %d\n", (int) gpio_map); // errno also set!
    exit(-1);
  }

  // always use volatile pointer
  gpio = (volatile unsigned *) gpio_map;
}

// declare the network run function
static void *network_run(void *q);

/**
 * The main function has a number of commandline arguments, including:
 *
 * number of LEDs: > 0
 * FPS: -1 if unlimited
 * Global Brightness: [0,1]
 * Power Limit: -1 if unlimited
 *
 * e.g. 300 LEDs, maximum framerate, full brightness, limited to 50 watts
 * ./playatower 300 -1 1 50
 */
int main(int narg, char **argc) {

  TinySpi tspi;
  struct timespec tick_start, tick, tock, diff_tick;
  uint32_t global_step = 0; // the current frame index
  float total_energy = 0.0f; // the total energy (joules) used since the beginning
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

  const float GLOBAL_BRIGHTNESS = (narg > 3) ? fmaxf(0.0f,fminf(1.0f,atof(argc[3]))) : 1.0f;
  printf("* global: %0.3g (%i/31)\n", GLOBAL_BRIGHTNESS, static_cast<int>(GLOBAL_BRIGHTNESS*31.0f));

  const float MAX_WATTS = (narg > 4) ? atof(argc[4]) : -1.0f;
  printf("* max. watts: %0.3f\n", MAX_WATTS);

  // open the SPI interface
  tspi_open(&tspi, "/dev/spidev0.0", SPI_HZ);

  // open the GPIO interface
  gpio_open();
  INP_GPIO(GPIO_INPUT_PIN); // configure GPIO pin as input

  PixelBuffer *pixbuf = new PixelBuffer(NUM_LEDS);
  pixbuf->setGlobal(GLOBAL_BRIGHTNESS);
  pixbuf->setPowerLimit(MAX_WATTS);

  printf("* SPI buffer: %i [%i] bytes\n", pixbuf->getNumSpiBytes(), pixbuf->getNumSpiBytesTotal());
  printf("\n");

  Animation *anim = new AnimPhasor(pixbuf); // initialise with default animation

  // start the network thread (with pipe)
  TinyPipe pipe;
  tpipe_init(&pipe, 4*1024); // 4KB pipe
  pthread_t networkThread = 0;
  pthread_create(&networkThread, NULL, &network_run, &pipe);

  int lastButtonState = (1<<GPIO_INPUT_PIN); // GPIO pin is high when *not* connected
  uint32_t anim_index = 0;
  bool toNextAnim = false;

  // record the start of the program
  clock_gettime(CLOCK_REALTIME, &tick_start);

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
    while (tpipe_hasData(&pipe)) {
      int numBytes = 0;
      char *osc_buffer = tpipe_getReadBuffer(&pipe, &numBytes);
      assert(numBytes != 0);
      assert(osc_buffer != nullptr);
      tosc_message osc;
      if (!tosc_parseMessage(&osc, (char *) osc_buffer, numBytes)) {
        if (!strcmp(tosc_getAddress(&osc), "/next")) {
          toNextAnim = true;
        } else if (!strcmp(tosc_getAddress(&osc), "/global")) {
          pixbuf->setGlobal(tosc_getNextFloat(&osc));
        } else if (!strcmp(tosc_getAddress(&osc), "/nightshift")) {
          pixbuf->setNightshift(tosc_getNextFloat(&osc));
        } else if (!strcmp(tosc_getAddress(&osc), "/powerlimit")) {
          pixbuf->setPowerLimit(tosc_getNextFloat(&osc));
        } else if (!strncmp(tosc_getAddress(&osc), "/param/", 7)) {
          // e.g. /param/0 0.5
          int index = atoi(tosc_getAddress(&osc)+7); // parameter index >= 0
          float value = tosc_getNextFloat(&osc); // parameter value [0,1]
          anim->setParameter(index, value);
        }
      }
      tpipe_consume(&pipe); // consume the message from the pipe
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
        case 1: anim = new AnimLorenzOsc(pixbuf); break;
        case 2: anim = new AnimLorenzOscFade(pixbuf); break;
        case 3: anim = new AnimChuaOsc(pixbuf); break;
        case 4: anim = new AnimLighthouse(pixbuf); break;
        case 5: anim = new AnimEiffelTower(pixbuf); break;
        case 6: anim = new AnimAllWhite(pixbuf); break;
        case 7: anim = new AnimLorenzPhasor(pixbuf); break;
        // case 6: anim = new AnimRain(pixbuf); break;
        // case 7: anim = new AnimRandomFlow(pixbuf); break;
        // case 9: anim = new AnimReactionDiffusion(pixbuf); break;
      }

      // FPS = anim->getPreferredFps();
      dt = 0.0;
    }

    // calculate animation
    anim->process(dt);

    // send LED data via SPI
    tspi_write(&tspi, pixbuf->getNumSpiBytes(), pixbuf->prepareAndGetSpiBytes());

    // keep track of total energy use
    total_energy += pixbuf->getCurrentWatts() * dt;

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

      // calculate total runtime
      timespec_subtract(&diff_tick, &tock, &tick_start);
      float total_watt_hours = total_energy/3600.0f;

      printf("\r| %6.1f fps | %7.3f Watts (%4.1f%%) [%4.1f Amps] | %8.2fWh total | %9i frames | %0.3f global [%s] | %0.3f nightshift | %.32s | [%g]",
          1.0/dt, pixbuf->getCurrentWatts(), 100.0f*pixbuf->getCurrentWatts()/pixbuf->getMaxWatts(), pixbuf->getCurrentAmperes(),
          total_watt_hours, global_step, pixbuf->getGlobal(), pixbuf->isPowerSuppressionEngaged() ? "x" : " ",
          pixbuf->getNightshift(), anim->getName(), anim->getParameter(0));
      fflush(stdout);
    }

    ++global_step;
  }

  // turn off all LEDs
  pixbuf->clear();
  tspi_write(&tspi, pixbuf->getNumSpiBytes(), pixbuf->prepareAndGetSpiBytes());

  munmap((void *) gpio, BLOCK_SIZE); // unmap the gpio memory
  pthread_join(networkThread, NULL); // wait for the network thread to stop
  tpipe_free(&pipe); // destroy the pipe from the network thread to the main thread
  tspi_close(&tspi); // close the SPI interface
  delete anim; // delete the animation
  delete pixbuf; // delete the pixel buffer

  return 0;
}

void *network_run(void *q) {
  assert(q != nullptr);

  // open receive socket
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert((fd > 0) && "Could not open network socket.");
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
    struct timeval tv = {0, 300000}; // sec, usec

    if (select(fd+1, &rfds, NULL, NULL, &tv) > 0) {
      uint8_t network_buffer[1024]; // buffer into which network data is received
      struct sockaddr_in sin;
      int len = 0;
      int sa_len = sizeof(struct sockaddr_in);

      while ((len = recvfrom(fd, network_buffer, sizeof(network_buffer), 0, (struct sockaddr *) &sin, (socklen_t *) &sa_len)) > 0) {
        // put message on pipe
        char *pipe_buffer = tpipe_getWriteBuffer((TinyPipe *) q, len);
        assert(pipe_buffer != nullptr);
        memcpy(pipe_buffer, network_buffer, len);
        tpipe_produce((TinyPipe *) q, len);
      }
    }
  }

  // close the receive socket
  close(fd);

  return NULL;
}
