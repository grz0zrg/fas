Fragment Synthesizer Band-aid
=====

Band-aid raw additive synthesizer built for the Fragment Synthesizer, a [web-based Collaborative Spectral Synthesizer](https://www.fsynth.com)

This program collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sound from it in real-time, this serve as a band-aid for crackling audio in the Fragment Synthesizer until some heavier optimizations are done or Web Audio API change.

Only one client is supported (altough many can connect, not tested but it may result in a big audio mess and likely a crash!)

The audio callback contain its own synth. data structure, the data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

There is a generic lock-free thread-safe commands queue for synth. parameters change (gain, oscillators etc.).

A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

Advanced optimizations can be enabled when compiling (only -DFIXED_WAVETABLE at the moment, which will use a fixed wavetable length of 2^16 for fast phase index warping)

**Can be used as a generic additive synthesizer if you feed it correctly! :)**

####Build

Requirements :

 * [PortAudio](http://www.portaudio.com/download.html)
 * [liblfds](http://liblfds.org/)
 * [libwebsockets](https://libwebsockets.org/)

libwebsockets default build options for fas : -DLWS_WITH_SSL=0 -DLWS_WITHOUT_CLIENT=1

Under Windows, [MSYS2](https://msys2.github.io/) with mingw32 is used and well tested.

Debug : **make**

Release : **make release**

Statically linked : **make release-static**

Statically linked and advanced optimizations (default build): **make release-static-o**

With MinGW (Statically linked) :  **make win-release-static**

With MinGW (Statically linked + advanced optimizations, default build) :  **make win-release-static-o**

#####Usage

You can tweak this program by passing settings to its arguments, for command-line help : **fas --h**

Usage: fas [list_of_settings]
 * --sample_rate 44100
 * --frames 512
 * --wavetable 1 **no effect at the moment**
 * --wavetable_size 8192 **no effect if built with advanced optimizations option**
 * --fps 60
 * --ssl 0
 * --fas_max_height 4096 **this should be changed if you go above 4096 for the score height**
 * --deflate 0
 * --rx_buffer_size 4096
 * --port 3003
 * --alsa_realtime_scheduling 0 **not under Windows**
 * --frames_queue_size 127
 * --commands_queue_size 16

Self-signed certificates are provided in case you compile/run it with SSL. (Note: This is useless for many reasons and HTTP should _**ALWAYS**_ be the prefered protocol for online Fragment application, this is explained in [this issue](https://github.com/grz0zrg/fas/issues/1).)

**You can stop the application by pressing any keys while it is running.**

https://www.fsynth.com
