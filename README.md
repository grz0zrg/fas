[Fragment Synthesizer](https://github.com/grz0zrg/fsynth) Band-aid
=====

Raw additive/granular/PM synthesizer built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth), a [web-based Collaborative Spectral Synthesizer](https://www.fsynth.com)

This program should compile on most platforms!

This program collect Fragment settings and RGBA (8-bit or 32-bit float) notes data over WebSocket, convert them to a suitable data structure and generate sounds in real-time by adding sine waves from a wavetable and add band-limited noise to enhance the synthesized sound, it can also interpret the data for granular synthesis (synchronous and asynchronous) and phase/frequency modulation (WIP), it is a generic image synth, this serve as a fast and independent alternative to output audio for the Fragment Synthesizer.

This program is tailored for performances (it is memory intensive, most things are pre-allocated with near zero real-time allocations) and can be executed on a [Raspberry Pi](https://www.raspberrypi.org/) with a [HifiBerry](https://www.hifiberry.com/) DAC for example, ~700 oscillators can be played simultaneously on the Raspberry Pi at the moment with two cores and minimum Raspbian stuff enabled (additive synthesis), note that frames drop can happen if the client is too late sending its slices per frame (this is controlled by the `frames_queue_size` option parameter), different reasons can make that happen such as slow connectivity, client side issues (slow browser/client), the RPI having too much load from stuff running in the background, etc.

Smooth audio can be produced even in the case of frames drop with the `max_drop` program option.

Only one client is supported at the moment (altough many can connect, not tested but it may result in a big audio mess and likely a crash!), you can launch multiple servers instance on different port and feed it different data if you need a multi-client audio server

The server only send the CPU load of the stream at regular interval (adjustable) to the client (double type).

The audio callback contain its own synth. data structure, the data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

There is a generic lock-free thread-safe commands queue for synth. parameters change (gain, oscillators etc.).

A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

Advanced optimizations can be enabled when compiling (only -DFIXED_WAVETABLE at the moment, which will use a fixed wavetable length of 2^16 for fast phase index warping), bandwidth enhanced sines can also be disabled for lightning fast additive synthesis.

In monophonic mode the Alpha channel value is the amplitude value.

**Can be used as a raw generic additive/granular/PM/FM synthesizer if you feed it correctly! :)**

### Additive/spectral synthesis

For additive synthesis and in stereophonic mode the pixels data channel R and G is the amplitude value of the oscillators (for L/R) while the B channel is the band-limited noise multiplier, if you set B to 0, no noise will added to the oscillator while a value of 1 will apply the global noise amount.

### Granular synthesis

The granular synthesis part is being actively developed and is mature enough to be used, you can have additive and granular synthesis at the same time with different output channel, all the grains are loaded from audio files found in the "grains" folder (put your .wav or .flac audio files there), FAS will load them all into memory at the moment.

Granular synthesis is less optimized than additive synthesis but has ok performances.

FAS will try to guess the sample pitch to map it correctly to the image height with several methods, an exact one from the filename (the filename should contain a note such as A#4 for example or a frequency between "#" character such as "flute_#440#.wav") and Yin pitch detection if everything else fail.

With granular synthesis method, the Blue pixel value is mapped to sample index (bounded to [0, 1]) and granular density when higher than 2, the Alpha value is mapped to sample index, the Alpha value can be used to play the sample backward as well when less than zero.

The ongoing development is to add more synthesis methods (FM/PM is WIP) and with the help of the essentia framework. (a C essentia wrapper is available)

### OSC

This support OSC output of pixels data on the channel "/fragment" with data type "idff" and data (in order) "osc index", "osc frequency", "osc amplitude L value", "osc amplitude R value"

With OSC you can basically do whatever you want with the pixels data, feeding SuperCollider synths for example, sending the data as an OSC bundle is WIP.

### Packets

To communicate with FAS with a custom client, there is only five type of packets to handle, the first byte of the packet is the packet identifier, below is the expected data for each packets (Note: you must send settings before sending any frames, otherwise they are simply ignored!) :

Synth settings, packet identifier 0 :
```c
struct _synth_settings {
    unsigned int h; // image height
    unsigned int octave; // octaves count
    unsigned int data_type; // the frame data type, 0 = 8-bit, 1 = float
    double base_frequency;
};
```

Frame data, packet identifier 1 :
```c
struct _frame_data {
    unsigned int channels; // channels count
    unsigned int monophonic; // 0 = stereo, 1 = mono
    // Note : the expected data length is computed by : (4 * (_synth_settings.data_type * sizeof(float)) * _synth_settings.h) * (fas_output_channels / 2)
    // Example with one output channel (L/R) and a 8-bit image with height of 400 pixels : (4 * sizeof(unsigned char) * 400)
    void *rgba_data;
};
```

Synth gain, packet identifier 2 :
```c
struct _synth_gain {
    double gain_lr;
};
```

Synth channels settings, packet identifier 3 :

`unsigned int channels_count;` followed by

```c
struct _synth_chn_settings {
    unsigned int synthesis_method; // 0 = additive, 1 = spectral, 2 = granular, 3 = sampler, 4 = FM/PM
    int env_type; // granular envelope type for this channel (there is 13 types of envelopes)
    double gmin_size; // granular grain duration (min. bound)
    double gmax_size; // granular grain duration (max. bound)
};
```

for every channels.

Server actions, packet identifier 4 :

- At the moment, this just reload samples in the grains folder.

### Build

Under Windows, [MSYS2](https://msys2.github.io/) with mingw32 is used and well tested.

Requirements :

 * [PortAudio](http://www.portaudio.com/download.html)
 * [liblfds](http://liblfds.org/)
 * [libwebsockets](https://libwebsockets.org/)
 * [liblo](http://liblo.sourceforge.net/)

The granular synthesis part make use of [libsndfile](https://github.com/erikd/libsndfile) and [tinydir](https://github.com/cxong/tinydir) (bundled)

Compiling requirements for Ubuntu/Raspberry Pi/Linux (default build) :

 * Get latest PortAudio v19 package
   * sudo apt-get install libasound-dev jackd qjackctl libjack-jackd2-dev
   * uncompress, go into the directory
   * ./configure
   * make clean
   * make
   * sudo make install
   * the static library can now be found at "lib/.libs/libportaudio.a"
 * Get latest liblfds 7.1.1 package
   * uncompress, go into the directory "liblfds711"
   * go into the directory "build/gcc_gnumake"
   * make
   * "liblfds711.a" can now be found in the "bin" directory
 * Get latest libwebsockets 2.1 package from github
   * sudo apt-get install cmake
   * go into the libwebsockets directory
   * mkdir build
   * cd build
   * cmake .. -DLWS_WITH_SSL=0 -DLWS_WITHOUT_CLIENT=1
   * make
   * sudo make install
   * "libwebsockets.a" can now be found in the "build/lib" directory
 * Get latest [liblo package](http://liblo.sourceforge.net/)
   * uncompress, go into the directory "liblo-0.29"
   * ./configure
   * copy the library found in "src/.libs/liblo.so.7.3.0" to FAS root folder
   * sudo make install

Copy the \*.a into "fas" root directory then compile by using one of the rule below (recommended rule for Linux and similar is "release-static-o").

Recommended launch parameters with HiFiBerry DAC+ :
    ./fas --alsa_realtime_scheduling 1 --frames_queue_size 63 --sample_rate 48000 --device 2
Bit depth is fixed to 32 bits float at the moment.

### Cross-compiling under Linux for Windows

The audio server was successfully cross-compiled under Windows (x86_64) with the Ubuntu package **mingw-w64** and the **win-cross-x86-64** makefile rule.

Most libraries will compile easily, some may take some workaround which are noted below, notably those which are using **cmake** (liblo, libwebsockets) and also libsndfile.

For those which are using cmake, a custom cmake toolchain file must be used

##### Custom cmake toolchain for x86_64

`set(CMAKE_SYSTEM_NAME Windows)`
`set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)`

`set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)`
`set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)`
`set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)`

`set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})`

`set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)`
`set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)`
`set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)`

##### With liblo

`cmake -DCMAKE_TOOLCHAIN_FILE=/home/julien/toolchain.cmake ../cmake`

##### With libwebsockets

`cmake -DLWS_WITH_SSL=0 -DCMAKE_TOOLCHAIN_FILE=/home/julien/toolchain.cmake ..`

##### For libsndfile

`./configure --build=x86_64 --host=x86_64-w64-mingw32`

#### Makefile rules

Debug : **make**

Debug (with libessentia) : **make debug-essentia**

Profile (benchmark) : **make profile**

Release : **make release**

Statically linked : **make release-static**

Statically linked and advanced optimizations : **make release-static-o**

Statically linked with bandlimited-noise, advanced optimizations (default build) : **make release-bln-static-o**

Statically linked, advanced optimizations and profiling: **make release-static-o-profile**

Statically linked (with libessentia) : **make release-essentia-static**

With MinGW (Statically linked) :  **make win-release-static**

With MinGW (Statically linked + advanced optimizations, default build) :  **make win-release-static-o**

With mingw-w64 package (Ubuntu) cross-compilation for Windows : **make win-cross-x86-64**

### Usage

You can tweak this program by passing parameters to its arguments, for command-line help : **fas --h**

A wxWidget user-friendly launcher is also available [here](https://github.com/grz0zrg/fas_launcher)

Usage: fas [list_of_parameters]
 * --i **print audio device infos**
 * --sample_rate 44100
 * --noise_amount 0.1 **the amount of band-limited noise**
 * --frames 512 **audio buffer**
 * --wavetable_size 8192 **no effects if built with advanced optimizations option**
 * --fps 60 **you can experiment with this but this may have strange effects**
 * --smooth_factor 8.0 **this is the samples interpolation factor between frames**
 * --ssl 0
 * --deflate 0
 * --max_drop 8 **this allow smooth audio in the case of frames drop, allow 8 frames drop by default**
 * --osc_out 0 **you can enable OSC output of notes by setting this argument to 1**
 * --osc_addr 127.0.0.1 **the OSC server address**
 * --osc_port 57120 **the OSC server port**
 * --grains_folder ./grains/
 * --granular_max_density 128 **this control how dense grains can be**
 * --rx_buffer_size 8192 **this is how much data is accepted in one single packet**
 * --port 3003 **the listening port**
 * --iface 127.0.0.1 **the listening address**
 * --device -1 **PortAudio audio device index (informations about audio devices are displayed when the app. start)**
 * --output_channels 2 **stereo pair**
 * --alsa_realtime_scheduling 0 **Linux only**
 * --frames_queue_size 7 **important parameter, if you increase this too much the audio will be delayed**
 * --commands_queue_size 16 **should be a positive integer power of 2**
 * --stream_load_send_delay 2 **FAS will send the stream CPU load every two seconds**

Self-signed certificates are provided in case you compile/run it with SSL. (Note: This is useless for many reasons and HTTP should _**ALWAYS**_ be the prefered protocol for online Fragment application, this is explained in [this issue](https://github.com/grz0zrg/fas/issues/1).)

**You can stop the application by pressing any keys while it is running.**

https://www.fsynth.com
