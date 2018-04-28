Fragment : Additive/Spectral/Granular/Subtractive/PM/Wavetable/Physical modelling synthesizer
=====

Raw additive/subtractive/spectral/granular/PM/Wavetable/Physical modelling synthesizer built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth), a [web-based and pixels-based collaborative synthesizer](https://www.fsynth.com)

This program should compile on most platforms!

Table of Contents
=================

* [<a href="https://www.fsynth.com">Fragment Synthesizer</a>](#fragment-:-additive/spectral/granular/pm-synthesizer)
   * [About FAS](#about)
      * [Pixels-based](#pixels-based)
      * [Additive synthesis](#additive-synthesis)
      * [Granular synthesis](#granular-synthesis)
      * [Spectral synthesis (planned)](#spectral-synthesis-(planned))
      * [Sampler](#sampler)
      * [Subtractive synthesis](#subtractive-synthesis)
      * [PM synthesis](#pm-synthesis)
      * [Wavetable synthesis](#wavetable-synthesis)
      * [Samples map](#samples-map)
      * [Performances](#performances)
         * [Raspberry PI](#raspberry-pi)
         * [Distributed/multi-core synthesis](#distributed/multi-core-synthesis)
         * [Frames drop](#frames-drop)
      * [Limitations](#limitations)
      * [What is sent](#what-is-sent)
      * [Offline rendering (planned)](#offline-rendering-(planned))
      * [OSC](#osc)
   * [Technical Implementation](#technical-implementation)
   * [Packets description](#packets-description)
   * [Building FAS](#build)
   * [Cross-compiling under Linux for Windows](#cross-compiling-under-linux-for-windows)
   * [Makefile rules](#makefile-rules)
   * [Usage](#usage)

## About

Fragment Audio Server (FAS) is a pixels-based additive, spectral, granular, subtractive, wavetable and phase modulated (PM) audio synthesizer implemented as a WebSocket server with the C language.

All the synthesis methods can be used at the same time by using different output channels.

FAS is focused on **real-time performances**, being **cross-platform** and **pixels-based**.

This project was built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth), a [web-based and pixels-based collaborative synthesizer](https://www.fsynth.com)

The most pixels-adapted synthesis methods are (in order) additive/spectral, wavetable, granular/PM/Physical modelling; re-synthesis is possible with them.

### Pixels-based

Unlike other synthesizers, the notes data format understood by FAS is entirely pixels-based, the notes data format can be

- **8-bit RGBA**
- **32-bit float RGBA**

The RGBA data collected is 1px wide with an user-defined height, the height is mapped to frequencies with an user-defined logarithmic frequency map.

FAS collect the RGBA data over WebSocket at an user-defined rate (commonly 60 or 120 Hz), convert the RGBA data to a suitable internal data structure and produce sounds in real-time by adding sine waves + noise together (additive synthesis), subtractive synthesis, wavetable synthesis, by interpreting the data for granular synthesis (synchronous and asynchronous) or through phase modulation (PM) or physical modelling.

It can be said that FAS/Fragment is a generic image-synth : any RGBA images can be used to produce an infinite variety of sounds by streaming vertical slices of the image to FAS.

### Additive synthesis

Additive synthesis is a mean to generate sounds by adding sine waves together, it is an extremely powerful type of sound synthesis able to reproduce any waveforms in theory.

FAS allow to add some amount of white noise to the phase of each sine waves, this may result in enhancement of noisy sounds.

#### RGBA interpretation

##### Monophonic

| Components | Interpretations                |
| ---------: | :----------------------------- |
|          R | unused                         |
|          G | unused                         |
|          B | unused                         |
|          A | Amplitude value of the channel |

##### Stereophonic

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | Added noise factor for the LEFT and RIGHT oscillator |
|          A | unused                                   |

### Granular synthesis

Granular synthesis is a mean to generate sounds by using small grains of sounds blended together and forming a continuous stream.

FAS grains source are audio files (.wav, .flac or any formats supported by [libsndfile](https://github.com/erikd/libsndfile)) automatically loaded into memory from the "grains" folder by default.

Both asynchronous and synchronous granular synthesis is implemented and can be used at the same time.

Granular synthesis implementation is less optimal than additive synthesis but has good performances.

The granular synthesis algorithm is also prototyped in JavaScript (one channel only) and can be tried step by step in a browser by opening the `lab/granular/algorithm.html`

All granular synthesis parameters excluding density and envelope type can be changed in real-time without issues.

**Note** : Monophonic mode granular synthesis is not implemented.

#### Window type

The grains window/envelope type is defined as a channel dependent settings, FAS allow the selection of 13 envelopes, they can be visualized in a browser by opening the `lab/envs.html` file.

#### RGBA interpretation

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | Sample index bounded to [0, 1] (cyclic) and grains density when > 2 |
|          A | Grains start index bounded [0, 1] (cyclic), grains start index random [0, 1] factor when > 1, play the grain backward when negative |

### Spectral synthesis (planned)

Spectral synthesis is a mean to generate sounds by using the FFT and IFFT algorithm preserving phase information, this is one of the most powerful synthesis algorithm available, it is able to re-produce sounds with great accuracy.

### Sampler

Granular synthesis with grain start index of 0 and min/max duration of 1/1 can be used to trigger samples as-is like a regular sampler, samples are loaded from the `grains` folder.

**Note** : Monophonic mode sampler is not implemented.

### Subtractive synthesis

Subtractive synthesis start from harmonically rich waveforms which are then filtered.

The default implementation is fast and use PolyBLED anti-aliased waveforms, an alternative, much slower which use additive synthesis is also available by commenting `POLYBLEP` in `constants.h`.

There is only one high quality low-pass filter (Moog type) implemented.

There is three type of band-limited waveforms : sawtooth, square, triangle

This type of synthesis may improve gradually with more waveforms and more filters.

**Note** : Additive synthesis waveforms are constitued of a maximum of 128 partials

#### RGBA interpretation

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | Moog filter cutoff multiplier; the cutoff is set to the fundamental frequency, 1.0 = cutoff at fundamental frequency |
|          A | Moog filter resonance [0, 1] & waveform selection on integral part (0.x, 1.x, 2.x etc) |

**Note** : Monophonic mode subtractive synthesis is not implemented.

### PM synthesis

Phase modulation (PM) is a mean to generate sounds by modulating the phase of an oscillator (carrier) from another oscillator (modulator), it is very similar to frequency modulation (FM).

PM synthesis in Fragment use a simple algorithm with one carrier and one modulator, the modulator amplitude and frequency can be set with B or A channel.

PM synthesis is one of the fastest method to generate sounds with Fragment and is able to do re-synthesis.

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the LEFT channel    |
|          G | Amplitude value of the RIGHT channel   |
|          B | Fractionnal part : Modulator amplitude |
|          A | Modulator frequency                    |

**Note** : Monophonic mode PM synthesis is not implemented.

### Wavetable synthesis

Wavetable synthesis is a sound synthesis technique that employs arbitrary periodic waveforms in the production of musical tones or notes.

Wavetable synthesis use single cycle waveforms / samples loaded from the `waves` folder.

The wavetable can be switched with the alpha channel (integral part), a linear interpolation will happen between current & next wavetable upon switch.

Wavetable synthesis is fast.

There is only one high quality low-pass filter (Moog type) implemented.

#### RGBA interpretation

| Components | Interpretations                                              |
| ---------: | :----------------------------------------------------------- |
|          R | Amplitude value of the LEFT channel                          |
|          G | Amplitude value of the RIGHT channel                         |
|          B | Filter cutoff parameter                                      |
|          A | Filter resonance [0, 1] & wavetable selection on integral part (0.x, 1.x, 2.x etc) |

**Note** : Monophonic mode Wavetable synthesis is not implemented.

### Physical modelling

Physical modelling synthesis refers to sound synthesis methods in which the waveform of the sound to be generated is computed using a mathematical model, a set of equations and algorithms to simulate a physical source of sound, usually a musical instrument.

Physical modelling in Fragment use Karplus-Strong string synthesis (for now).

This is a fast method which generate pleasant string-like sounds.

Physical modelling is WIP and may be subject to major changes.

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the LEFT channel    |
|          G | Amplitude value of the RIGHT channel   |
|          B | Noise wavetable cutoff lp filter / fractional part : stretching factor       |
|          A | Noise wavetable res. lp filter         |

**Note** : Monophonic mode Physical modelling synthesis is not implemented.

### Samples map

Each samples loaded from the `grains` or `waves` folder are processed, one of the most important process is the sample pitch mapping, this process try to gather informations or guess the sample pitch to map it correctly onto the user-defined image height when used, in order :

1. from the filename, the filename should contain a specific pattern which indicate the sample pitch such as `A#4` or a frequency between "#" character such as `flute_#440#.wav`
2. with Yin pitch detection algorithm, this method can be heavily inaccurate and depend on the sample content

### Performances

This program is tailored for performances, it is memory intensive (about 1 Gb is needed without samples, about 2 Gb with few samples), most things are pre-allocated with near zero real-time allocations.

#### Raspberry PI

FAS was executed on a [Raspberry Pi](https://www.raspberrypi.org/) with a [HifiBerry](https://www.hifiberry.com/) DAC for example, ~700 additive synthesis oscillators can be played simultaneously on the Raspberry Pi with two cores used and minimum Raspbian stuff enabled.

#### Distributed/multi-core synthesis

Due to the architecture of FAS, distributed sound synthesis is made possible by running multiple FAS instances on the same or different computer by distributing the pixels data correctly to each instances, on the same machine this only require a sufficient amount of memory.

This is the only way to exploit multiple cores on the same machine.

This need a relay program which will link each instances with the client and distribute each notes to instances based on a distribution algorithm.

A directly usable implementation with NodeJS of a distributed synthesis relay can be found [here](https://github.com/grz0zrg/fsynth/tree/master/fas_relay)

#### Frames drop

Frames drop happen if the client is too late sending its slices per frame (this is controlled by the `frames_queue_size` option parameter), different reasons can make that happen such as slow connectivity, client side issues like a slow client, etc.

When a frame is dropped, FAS hold the audio till a frame is received or the `max_drop` program option is reached, this ensure smooth audio even if the client has issues sending its frames, latency can be heard if too much frames are dropped however.

### Limitations

Only one client is supported at the moment (many can connect but it is not tested and may result in a big audio mess and likely a crash!), you can launch multiple servers instance on different port and feed it different data if you need a multi-client audio server

### What is sent

The server send the CPU load of the stream at regular interval (adjustable) to the client (double type).

### Offline rendering (planned)

FAS support real-time rendering of the pixels data, the pixels data is compressed on-the-fly into a single file, FAS can then do offline processing and be used again to convert the pixels data into an audio .flac file, this  ensure professional quality audio output.

### Future

The ongoing development is to improve synthesis methods and implement new type of synthesis like spectral with the help of the essentia framework (a C essentia wrapper is available) or Soundpipe library.

### OSC

FAS support OSC output of pixels data on the channel "/fragment" with data type "idff" and data (in order) "osc index", "osc frequency", "osc amplitude L value", "osc amplitude R value"

With OSC you can basically do whatever you want with the pixels data, feeding SuperCollider synths for example, sending the data as an OSC bundle is WIP.

## Technical implementation

The audio callback contain its own synth. data structure, the data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

There is a generic lock-free thread-safe commands queue for synth. parameters change (gain, oscillators etc.).

A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

Additive synthesis is wavetable-based.

Real-time resampling is done with a simple linear method, granular synthesis can also be resampled by using cubic interpolation method (uncomment the line in `constants.h`) which is slower than linear.

All synthesis algorithms (minus filters and [PolyBLEP](http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/)) are customs.

[Soundpipe library](https://github.com/PaulBatchelor/Soundpipe) modules are optionally used (Note : WIP), this provide some high quality and battle tested algorithms and fx

This program is tested with Valgrind and should be free of memory leaks.

## Packets description

To communicate with FAS with a custom client, there is only five type of packets to handle, the first byte of the packet is the packet identifier, below is the expected data for each packets

**Note** : synth settings packet must be sent before sending any frames, otherwise the received frames are ignored.

Synth settings, packet identifier 0 :
```c
struct _synth_settings {
    unsigned int h; // image/slice height
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
    unsigned int synthesis_method; // 0 = additive, 1 = spectral, 2 = granular, 3 = FM/PM, 4 = subtractive
    int env_type; // granular envelope type for this channel (there is 13 types of envelopes)
    double p1; // granular grain duration (min. bound) or subtractive filter drive
    double p2; // granular grain duration (max. bound)
};
```

for every channels.

Server actions, packet identifier 4 :

- At the moment, this just reload samples in the grains folder, this should be followed by a synth settings change to pre-compute grains tables.

## Build

Under Windows, [MSYS2](https://msys2.github.io/) with mingw32 is used and well tested.

Requirements :

 * [PortAudio](http://www.portaudio.com/download.html)
 * [liblfds](http://liblfds.org/)
 * [libwebsockets](https://libwebsockets.org/)
 * [liblo](http://liblo.sourceforge.net/)
 * libsamplerate
 * [Soundpipe](https://github.com/PaulBatchelor/Soundpipe) (Optional)

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
* Get latest [libsamplerate](http://www.mega-nerd.com/SRC/download.html)
  * uncompress, go into the directory "libsamplerate-0.1.9"
  * ./configure
  * copy the library found in "src/.libs/libsamplerate.a" to FAS root folder
* Get latest [Soundpipe](https://github.com/PaulBatchelor/Soundpipe)
  * make
  * "libsoundpipe.a" can now be found in the Soundpipe directory

Copy the \*.a into "fas" root directory then compile by using one of the rule below (recommended rule for Linux and similar is `release-static-o` **without** Soundpipe and `release-static-sp-o` **with** Soundpipe).

Recommended launch parameters with HiFiBerry DAC+ :
    ./fas --alsa_realtime_scheduling 1 --frames_queue_size 63 --sample_rate 48000 --device 2
Bit depth is fixed to 32 bits float at the moment.

**Note** : Advanced optimizations can be enabled when compiling (only -DFIXED_WAVETABLE at the moment, which will use a fixed wavetable length of 2^16 for fast phase index warping), bandwidth enhanced sines can also be disabled for lightning fast additive synthesis.

## Cross-compiling under Linux for Windows

The audio server was successfully cross-compiled under Windows (x86_64) with the Ubuntu package **mingw-w64** and the **win-cross-x86-64** makefile rule.

Most libraries will compile easily, some may take some workaround which are noted below, notably those which are using **cmake** (liblo, libwebsockets) and also libsndfile/libsamplerate.

The makefile rules get the libraries from a "cross" folder in the FAS root directory.

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

##### PortAudio with ASIO etc.

`./configure --build=x86_64 --host=x86_64-w64-mingw32 --with-jack --with-alsa -with-winapi=asio,directx,wdmks,wmme --with-asiodir=~/crosscompile/ASIOSDK2.3`

`make clean`

`make`

##### With liblo

`cmake -DCMAKE_TOOLCHAIN_FILE=~/toolchain.cmake ../cmake`

##### With libwebsockets

`cmake -DLWS_WITH_SSL=0 -DCMAKE_TOOLCHAIN_FILE=~/toolchain.cmake ..`

##### For libsndfile & libsamplerate

`./configure --build=x86_64 --host=x86_64-w64-mingw32`

## Makefile rules

Debug : **make**

Debug (with libessentia) : **make debug-essentia**

Debug (with Soundpipe) : **make debug-soundpipe**

Profile (benchmark) : **make profile**

Release : **make release**

Statically linked : **make release-static**

Statically linked and advanced optimizations : **make release-static-o**

Statically linked + Soundpipe and advanced optimizations : **make release-static-sp-o**

Statically linked with bandlimited-noise, advanced optimizations (default build) : **make release-bln-static-o**

Statically linked + Soundpipe with bandlimited-noise, advanced optimizations (default build) : **make release-bln-static-sp-o**

Statically linked, advanced optimizations and profiling: **make release-static-o-profile**

Statically linked (with libessentia) : **make release-essentia-static**

With MinGW (Statically linked) :  **make win-release-static**

With MinGW (Statically linked + advanced optimizations, default build) :  **make win-release-static-o**

With mingw-w64 package (Ubuntu) cross-compilation for Windows : **make win-cross-x86-64**

## Usage

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
 * --max_drop 32 **this allow smooth audio in the case of frames drop, allow 16 frames drop by default**
 * --render target.fs **real-time pixels-data offline rendering, this will save pixels data to "target.fs" file**
 * --render_convert target.fs **this will convert the pixels data contained by the .fs file to a .flac file of the same name**
 * --osc_out 0 **you can enable OSC output of notes by setting this argument to 1**
 * --osc_addr 127.0.0.1 **the OSC server address**
 * --osc_port 57120 **the OSC server port**
 * --grains_folder ./grains/
 * --granular_max_density 128 **this control how dense grains can be**
 * --waves_folder ./waves/
 * --rx_buffer_size 8192 **this is how much data is accepted in one single packet**
 * --port 3003 **the listening port**
 * --iface 127.0.0.1 **the listening address**
 * --device -1 **PortAudio audio device index (informations about audio devices are displayed when the app. start)**
 * --output_channels 2 **stereo pair**
 * --alsa_realtime_scheduling 0 **Linux only**
 * --frames_queue_size 7 **important parameter, if you increase this too much the audio will be delayed**
 * --commands_queue_size 16 **should be a positive integer power of 2**
 * --stream_load_send_delay 2 **FAS will send the stream CPU load every two seconds**
 * --samplerate_conv_type -1 **see [this](http://www.mega-nerd.com/SRC/api_misc.html#Converters) for converter type, this has impact on samples loading time, this settings can be ignored most of the time since FAS do real-time resampling, -1 skip the resampling step**

Self-signed certificates are provided in case you compile/run it with SSL. (Note: This is useless for many reasons and HTTP should _**ALWAYS**_ be the prefered protocol for online Fragment application, this is explained in [this issue](https://github.com/grz0zrg/fas/issues/1).)

**You can stop the application by pressing any keys while it is running.**

https://www.fsynth.com
