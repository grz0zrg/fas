[Fragment Synthesizer](https://github.com/grz0zrg/fsynth) Band-aid
=====

Band-aid raw additive synthesizer built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth), a [web-based Collaborative Spectral Synthesizer](https://www.fsynth.com)

This program collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sound in real-time, this serve as a fast and independent alternative to play audio for the Fragment Synthesizer.

This can be run on a [Raspberry Pi](https://www.raspberrypi.org/) with a [HifiBerry](https://www.hifiberry.com/) DAC for example, ~700 oscillators can be played simultaneously on the Raspberry Pi at the moment with two cores.

Only one client is supported (altough many can connect, not tested but it may result in a big audio mess and likely a crash!)

The audio callback contain its own synth. data structure, the data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

There is a generic lock-free thread-safe commands queue for synth. parameters change (gain, oscillators etc.).

A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

Advanced optimizations can be enabled when compiling (only -DFIXED_WAVETABLE at the moment, which will use a fixed wavetable length of 2^16 for fast phase index warping)

**Can be used as a raw generic additive synthesizer if you feed it correctly! :)**

###Build

Under Windows, [MSYS2](https://msys2.github.io/) with mingw32 is used and well tested.

Requirements :

 * [PortAudio](http://www.portaudio.com/download.html)
 * [liblfds](http://liblfds.org/)
 * [libwebsockets](https://libwebsockets.org/)

Compiling requirements for Ubuntu/Raspberry Pi/Linux (default build) :

 * Get latest PortAudio v19 package
   * sudo apt-get install libasound-dev jackd qjackctl libjack-jackd2-dev
   * uncompress, go into the directory
   * ./configure
   * make clean
   * make
   * sudo make install
   * the static library can now be found at "lib/.libs/libportaudio.a"
 * Get latest liblfds 7.1.0 package
   * uncompress, go into the directory "liblfds7.1.0"
   * go into the directory "build/gcc_gnumake"
   * make
   * "liblfds710.a" can now be found in the "bin" directory
 * Get latest libwebsockets 2.1 package from github
   * sudo apt-get install cmake
   * go into the libwebsockets directory
   * mkdir build
   * cd build
   * cmake .. -DLWS_WITH_SSL=0 -DLWS_WITHOUT_CLIENT=1
   * make
   * sudo make install
   * "libwebsockets.a" can now be found in the "build/lib" directory

Copy the \*.a into "fas" root directory then compile by using one of the rule below.

####Makefile rules

Debug : **make**

Profile (benchmark) : **make profile**

Release : **make release**

Statically linked : **make release-static**

Statically linked and advanced optimizations (default build): **make release-static-o**

With MinGW (Statically linked) :  **make win-release-static**

With MinGW (Statically linked + advanced optimizations, default build) :  **make win-release-static-o**

###Usage

You can tweak this program by passing parametsrs to its arguments, for command-line help : **fas --h**

Usage: fas [list_of_settings]
 * --sample_rate 44100
 * --frames 512
 * --wavetable_size 8192 **no effects if built with advanced optimizations option**
 * --fps 60
 * --ssl 0
 * --fas_max_height 4096 **this should be changed if you go above 4096 for the score height**
 * --deflate 0
 * --rx_buffer_size 4096
 * --port 3003
 * --iface 127.0.0.1
 * --device -1 **PortAudio audio device index (informations about audio devices are displayed when the app. start)**
 * --alsa_realtime_scheduling 0 **not under Windows**
 * --frames_queue_size 7 **important parameter, if you increase this too much the audio will be delayed**
 * --commands_queue_size 16

Self-signed certificates are provided in case you compile/run it with SSL. (Note: This is useless for many reasons and HTTP should _**ALWAYS**_ be the prefered protocol for online Fragment application, this is explained in [this issue](https://github.com/grz0zrg/fas/issues/1).)

**You can stop the application by pressing any keys while it is running.**

https://www.fsynth.com
