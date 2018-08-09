# Playatower 2018

Playatower is an ongoing art project conceived in 2016 generally intended for Burning Man and related festivals. The Tower, or Beacon, is a two meter tall spiral of LEDs with the practical intention of acting as a widely visible location marker for an event or camp. Artistically the programmed light patterns are an exploration of mathematically chaotic systems, projected onto a one-dimensional manifold, two physical dimensions (in this case, a spiral), and the full visible light spectrum. Each pattern rhymes, it repeats but never exactly, intending to lure and enchant the viewer. The Beacon is a _Tripper Trap_.

The animations are based on a few mathematical principles, including chaotic oscillators (e.g. Lorenz, Chua, and Van der Pol), loosely-coupled oscillators (e.g. the firefly synchronisation effect), reaction diffusion systems, random walks, pheromone-based systems, and cellular automata. Ideally the uninformed viewer may simply be mesmerised by the patterns, while the more curious mind may venture down an infinite rabbit hole.

## Animations
* `Phasor`
* `ChuaOsc`
* `Firefly`
* `LorenzOsc`
* `LorenzOscFade`
* `VanDerPol`

## Approaches
* Pheromone Layering
* Distribution Layering

## GPIO
* https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root

# Setup

## Flash SD Disk
* https://www.raspberrypi.org/documentation/installation/installing-images/mac.md

## SPI on RPi3
* https://github.com/raspberrypi/linux/issues/2094
  * add `core_freq=250` into `/boot/config.txt`

## Update
* `$ sudo rpi-update`
* `$ sudo apt-get update`
* `$ sudo apt-get upgrade`
* `$ sudo raspi-config`
* `$ sudo apt-get install clang`
* `$ sudo apt-get install git` (optional)

## Wifi
* https://www.raspberrypi.org/documentation/configuration/wireless/wireless-cli.md
* `$ sudo nano /etc/wpa_supplicant/wpa_supplicant.conf`
* Add SSID/pw info as below:
```
network={
    ssid="xxx"
    psk="yyy"
}
```

## Autostart
* `$ sudo nano /etc/rc.local`
* add `/home/pi/projects/my_project.a &`
