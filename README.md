Fragment Synthesizer Band-aid
=====

Band-aid raw additive synthesizer built for the Fragment Synthesizer, a [web-based Collaborative Spectral Synthesizer](https://www.fsynth.com)

This program collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sound from it in real-time, this serve as a band-aid for crackling audio in the Fragment Synthesizer until some heavier optimizations are done or Web Audio API change.

Only one client is supported (altough many can connect, not tested but it may result in a big audio mess and likely a crash!)

The audio callback contain its own synth. data structure, the data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

There is a generic lock-free thread-safe commands queue for synth. parameters change (gain, oscillators etc.).

A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

**Can be used as a generic additive synthesizer if you feed it correctly! :)**

####Build

Requirements :

* [PortAudio](http://www.portaudio.com/download.html)
* [liblfds](http://liblfds.org/)
* [libwebsockets](https://libwebsockets.org/)

libwebsockets build options : -DLWS_WITH_SSL=0 -DLWS_WITHOUT_CLIENT=1

Debug : **make**
Release : **make release**
Statically linked : **make release-static**

#####Usage

You can tweak this program by passing settings to its arguments, for command-line help : **fas --h**

Usage: fas [list_of_settings]
* --sample_rate 44100
* --frames 512
* --wavetable 1
* --wavetable_size 8192
* --fps 60
* --deflate 0
* --rx_buffer_size 4096
* --port 3003
* --alsa_realtime_scheduling 0
* --frames_queue_size 127
* --commands_queue_size 16

https://www.fsynth.com
