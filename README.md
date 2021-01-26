Fragment Audio Server
=====

Oscillator bank / spectral synthesizer built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth), a [web-based collaborative & graphical audio synthesizer](https://www.fsynth.com)

This program should compile on most platforms!

Table of Contents
=================

* [<a href="https://www.fsynth.com">Fragment Synthesizer</a>](#fragment-:-additive/spectral/granular/pm-synthesizer)
   * [About FAS](#about)
      * [Pixels-based](#pixels-based)
      * [Additive synthesis](#additive-synthesis)
      * [Granular synthesis](#granular-synthesis)
      * [Sampler](#sampler)
      * [Subtractive synthesis](#subtractive-synthesis)
      * [PM synthesis](#pm-synthesis)
      * [Wavetable synthesis (WIP)](#wavetable-synthesis)
      * [Physical modelling](#physical-modelling)
      * [Spectral synthesis](#spectral-synthesis)
      * [Bandpass synthesis](#bandpass-synthesis)
      * [Formant synthesis](#formant-synthesis)
      * [Phase Distorsion synthesis](#phase-distorsion-synthesis)
      * [String resonator synthesis](#string-resonator-synthesis)
      * [Modal synthesis](#modal-synthesis)
      * [Input](#input)
      * [Modulation](#modulation)
      * [Faust](#faust)
      * [Samples map](#samples-map)
      * [Effects](#effects)
      * [Performances](#performances)
         * [Raspberry PI](#raspberry-pi)
         * [Distributed/multi-core synthesis](#distributed/multi-core-synthesis)
         * [Frames drop](#frames-drop)
      * [Limitations](#limitations)
      * [What is sent](#what-is-sent)
      * [Offline rendering (planned)](#offline-rendering-(planned))
      * [Jack](#jack)
   * [Technical Implementation](#technical-implementation)
   * [Packets description](#packets-description)
   * [Building FAS](#build)
   * [Cross-compiling under Linux for Windows](#cross-compiling-under-linux-for-windows)
   * [Makefile rules](#makefile-rules)
   * [Usage](#usage)
   * [Credits](#credits)

## About

Fragment Audio Server (FAS) is a high performance pixels-based graphical audio synthesizer implemented as a WebSocket server with the C language (C11).

One can see this as a limitless bank of generators / filters.

The versatility of its sound engine allow a wide variety of synthesis methods (aka instruments) to produce sounds:

* additive / spectral with per partial effects (bitcrush, phase distorsion, waveshaping, fold, convolve)
* phase modulation (PM/FM)
* granular (asynchronous / synchronous)
* subtractive synthesis
* physical modelling (models : Karplus-strong, droplet)
* wavetable synthesis

There is a second type of synthesis methods (or modifiers) which use any synthesis methods from above as input:

* second-order band-pass Butterworth filter (bandpass filter bank)
* formant synthesis (formant filter bank)
* string resonator synthesis (complex filter bank similar to Karplus-Strong)
* modal synthesis (resonant filter bank)
* phase distorsion

There is a third method which can do both; modify or synthesize sounds :

* spectral (via STFT)

There is also the [Faust](https://faust.grame.fr/) option which allow to import / write custom generators and effects built out of Faust DSP specification language and load them at runtime to extend FAS dynamically with custom DSP code.

Other type of synthesis (Linear Arithmetic Synthesis, Vector synthesis, Walsh, stacked waves etc.) may be supported out of the box easily by a combination of the methods above.

There is also an input instrument which just play input audio so amplitude envelope, effects or second synthesis type can be applied.

There is also a modulation instrument which can be used to modulate fx or instruments.

Multiple instruments can be used at the same time with dedicated virtual output channels (which can be routed to any device output channels), the maximum number of instruments / virtual channels can be changed on launch.

FAS is focused on **real-time performances**, being **cross-platform** and **pixels-based**.

This project was built for the [Fragment Synthesizer](https://github.com/grz0zrg/fsynth) client, a [web-based graphical audio / spectral collaborative synthesizer](https://www.fsynth.com)

The most pixels-adapted synthesis methods are (in order) additive/spectral, wavetable, granular/PM/Physical modelling; re-synthesis is possible with all of them.

Requirement : by default this synth require ~1 Gb RAM (~2 Gb RAM to be safe) which enable a maximum of 24 instruments / virtual channels with a frames queue size of 3, the memory requirement can be lowered greatly by lowering the amount of instruments / virtual channels or the frame queue size parameter (a frame queue size of 1 would divide the requirement by two but would potentially introduce hiccups).

### Pixels-based

Unlike other synthesizers, the notes data format understood by FAS is entirely pixels-based, the notes data format can be

- **8-bit RGBA**
- **32-bit float RGBA**

The RGBA data collected is 1px wide with an user-defined height, the height is mapped to frequencies with an user-defined logarithmic frequency map. Multiple 1px slices are defined as separate instruments.

FAS collect the slices RGBA data over WebSocket (streaming) at an user-defined rate (commonly 60 or 120 Hz), convert the RGBA data to a suitable internal data structure and produce sounds in real-time.

It can be said that FAS/Fragment is a generic image-synth (also called graphical audio synthesizer): any RGBA images may be used to produce an infinite variety of sounds by streaming bitmap data to FAS.

Each note events carry amplitude level (stereo) in RED and GREEN component plus additional synthesis parameters in BLUE and ALPHA channels.

With a light wrapper its architecture can also be used as a generic synth right out of the box; just deal with RGBA notes.

As a fun side note FAS can be considered as a basis to build a simple [DAW](https://en.wikipedia.org/wiki/Digital_audio_workstation) due to architecture similarities.

### Specifications

Here is some architectural specifications as if it were made by a synth. manufacturer :

* polyphonic; unlimited number of voices (depend on input data height parameter)
* multitimbral; unlimited number of timbres / parts / instruments with **configurable virtual output channel**
* distributed architecture; more than one instance can run on same machine / a network with independent processing of voice / part, example : [FAS relay](https://github.com/grz0zrg/fsynth/tree/master/fas_relay)
* driven by pixels data over the wire; this synth can be used as a basis for other softwares and has about no limitations as-is, it is typically used with a client that implement higher order features like MIDI such as [Fragment client](https://github.com/grz0zrg/fsynth)
 * envelopes ? has it all due to stream based architecture, you can build any types (ADSR etc.) with any interpolation scheme (linear / exp etc.)
* multiple sound engine; additive / spectral, sample-based, subtractive, wavetable, physical modeling, frequency modulation, spectral, filters bank, phase distorsion...
* virtual channels: instruments -> virtual channels -> output device channels
* allow to extend the sound engine at runtime with user-defined generators and effects written with [Faust](https://faust.grame.fr/) DSP specification language
* high quality stereophonic audio with low latency and multiple input / output (with Jack)
* fully microtonal / spectral (oscillator-bank concept for every instruments)
* unlimited effects slot per virtual channel (24 by default but adaptable); reverb, convolution, comb, delay, filters, phaser... 30 high quality effects type provided by Soundpipe are available (most of them stereophonic)
* per partial effect slot for additive synthesis
* per voice filtering for subtractive synthesis with one multi mode filter
  * per voice effects is limited by RGBA note data, this may be seen as a limitation since only one multi mode filter per voice is allowed
* highly optimized real-time architecture; run on low-power embedded hardware such as Raspberry
* events resolution can be defined as you wish and can be changed dynamically (typically display refresh rate; 60 Hz but you can go above that through a real-time parameter)
* cross-platform; run this about anywhere !

Note : "Unlimited" is actually an architectural term, in real conditions it is limited by the available processing power, amount of memory available, frequency resolution is also limited by slice height so it may be out of tune with low resolution! (just like analog but still more precise!)

### Additive synthesis

Additive synthesis is a mean to generate sounds by adding sine waves together, it is an extremely powerful type of sound synthesis able to reproduce any waveforms in theory.

When compiled with `PARTIAL_FX` defined there is a fx slot available **per partial**, the effect type must be provided as an integer value from the BLUE component, the following effects are available :

* 0: none
* 1: bitcrush (Bitdepth / Sampling rate : B component [0, 1] / A component [0, 1])
* 2: tanh waveshaping (Wave 1 / Wave 2 : B component [0, 1] / A component [0, 1))
* 3: noise (B added white noise factor to sine wave phase, maximum defined by command-line parameter)

Any combination of these can be applied to each partials with real-time parameters change. This feature may allow to easily add character to the additive sound.

Partials effects can be disabled to speed up additive synthesis (or if FAS doesn't use the Soundpipe library).

Additive synthesis use either magic circle algorithm or a wavetable, speed depend on architecture, magic circle algorithm is recommended. (note : noise is not available with magic circle)

#### RGBA interpretation

##### Stereophonic

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | see above |
|          A | see above |

### Granular synthesis

Granular synthesis is a mean to generate sounds by using small grains of sounds blended together and forming a continuous stream.

FAS grains source are audio files (.wav, .flac or any formats supported by [libsndfile](https://github.com/erikd/libsndfile)) automatically loaded into memory from the "grains" directory by default.

Both asynchronous and synchronous granular synthesis is implemented and can be used at the same time.

Granular synthesis implementation is less optimal than additive synthesis but has good performances.

The granular synthesis algorithm is also prototyped in JavaScript (one channel only) and can be tried step by step in a browser by opening the `lab/granular/algorithm.html`

All granular synthesis parameters excluding density and envelope type can be changed in real-time without crackles.

#### Window type

The grains window/envelope type is defined as a channel dependent settings, FAS allow the selection of 13 envelopes, they can be visualized in a browser by opening the `lab/envs.html` file.

#### RGBA interpretation

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | Sample index bounded to [0, 1] (cyclic) and grains density when > 2 |
|          A | Grains start index bounded [0, 1] (cyclic), grains start index random [0, 1] factor when > 1, play the grain backward when negative |

### Sampler

Granular synthesis with grain start index of 0 and min/max duration of 1/1 can be used to trigger samples as-is like a regular sampler, samples are loaded from the `grains` directory.

### Subtractive synthesis

Subtractive synthesis start from harmonically rich waveforms which are then filtered.

Fast and use PolyBLEP anti-aliased waveforms.

Without Soundpipe there is one high quality low-pass filter (Moog type) implemented.

With Soundpipe there is many filters type to chose from (see channel settings): moog, diode, korg 35, lpf18...

Be careful as some filters may have unstability issues with some parameters! (note : they are all checked against safety but there could bugs...)

There is three type of band-limited waveforms : sawtooth, square, triangle

There is also a noise waveform and additional brownian / pink noise with Soundpipe.

#### RGBA interpretation

| Components | Interpretations                          |
| ---------: | :--------------------------------------- |
|          R | Amplitude value of the LEFT channel      |
|          G | Amplitude value of the RIGHT channel     |
|          B | filter cutoff multiplier; the cutoff is set to the fundamental frequency, 1.0 = cutoff at fundamental frequency |
|          A | filter resonance [0, 1] & waveform selection on integral part (0.x, 1.x, 2.x etc) |

### PM synthesis

Phase modulation (PM) is a mean to generate sounds by modulating the phase of an oscillator (carrier) from another oscillator (modulator), it is very similar to frequency modulation (FM).

PM synthesis in Fragment use a simple algorithm with one carrier and one modulator with filtered feedback.

Carrier and modulator oscillator can be a sine wave or an arbitrary wavetable (from `waves` directory) set by p0 and p1 channel settings parameter. (with -1 indicating the bundled sine wavetable)

Modulator amplitude and frequency can be set with B and A channel, modulator feedback amount can be set with integral part of B channel.

PM synthesis is one of the fastest method to generate sounds and is able to do re-synthesis.

The typical index of modulation of standard FM synths can be computed by doing : modulator frequency / carrier frequency

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the LEFT channel    |
|          G | Amplitude value of the RIGHT channel   |
|          B | Fractionnal part : Modulator amplitude, Integer part : Modulator feedback level [0,65536) |
|          A | Modulator frequency                    |

### Wavetable synthesis

Wavetable synthesis is a sound synthesis technique that employ arbitrary periodic waveforms in the production of musical tones or notes.

Wavetable synthesis use single cycle waveforms / samples loaded from the `waves` directory. Wave lookup is monophonic.

The implementation is similar to PPG synths with linear interpolation (sampling & wave change) but no oversampling. (may alias)

Interpolation between waves can be enabled / disabled (PPG like) at note time trough A fractional part. ( > 0 enabled interpolation)

Specific wave can be read by having similar start & end value (which will thus act as an offset) with a wavetable speed set to 0.

The speed at which the table is read can be set with the fractional part of the blue channel, the table will be read in reverse order if the value is negative.

Every note-on trigger a wavetable reset from current settings (wavetable position), this can be disabled by using channel settings p0, a value of 1 will reset the wavetable every notes and a value of 0 disable it.

Wavetable synthesis is fast and provide rich sounds.

Note : FAS does not really have multiple 'wavetables' as it load every waves into a big continuous wavetable but the different wavetables are ordered (by directory then by filename) when loaded so that each loaded waves are contiguous.

Note : The wavetable can be used as a sampler as long as the input file is exported into small chunks (with some kind of windowing to remove crackles), this can be done easily with a small script or some software. An example Python script `sample_split.py` is available in the scripts directory.

#### RGBA interpretation

| Components | Interpretations                                              |
| ---------: | :----------------------------------------------------------- |
|          R | Amplitude value of the LEFT channel                          |
|          G | Amplitude value of the RIGHT channel                         |
|          B | Start wave selection on integral part & wavetable speed on fractional |
|          A | End wave selection on integral part & wave interpolation on / off on fractional |

### Spectral synthesis

Spectral synthesis produce sounds by modifying (mode 0) any input channel / generate (mode 1) in frequency domain via a [Short-time Fourier transform](https://en.wikipedia.org/wiki/Short-time_Fourier_transform) with overlap add method.

Since FAS use a fixed window size of up to 1024 bins will be differently mapped and thus some frequencies of the vertical axis may fall onto the same bin.

To get the corresponding bin one can use this formula : `frequency / (sample_rate / 2) / window_size`

Channel settings are used to change spectral parameters :

 * p0 : source channel / instrument
 * p1 : window size (power of two; 32 up to 1024; might introduce a delay)
 * p2 : mode (0 or 1)
 * p3 : source mode (0 for channel or 1 for instrument)

Mode is a parameter which select how the frequency domain changes will be applied

 * mode 0 (re-synthesis) : incoming data is used as a factor of the input data bins (polar form)
 * mode 1 (synthesis) : incoming data is directly placed into the corresponding bin, input channel is unused, it is faster because a FFT step is discarded (note : some frequencies may fall into the same bin due to differences in how frequencies are mapped)

Typical example may be a vocoder (which can be done using bandpass as well) and more generally cross synthesis (combining characteristics of different sounds), some effects involving phases may be done as well.

While the actual implementation work nicely it may change in the future to incorporate more features.

#### RGBA interpretation

| Components | Interpretations                                              |
| ---------: | :----------------------------------------------------------- |
|          R | Magnitude factor of the LEFT channel (depend on mode)        |
|          G | Magnitude factor of the RIGHT channel                        |
|          B | Phase factor of the LEFT channel                             |
|          A | Phase factor of the RIGHT channel                            |

### Physical modelling

Physical modelling synthesis refers to sound synthesis methods in which the waveform of the sound to be generated is computed using a mathematical model, a set of equations and algorithms to simulate a physical source of sound, usually a musical instrument.

Physical modelling in Fragment use models, Karplus-Strong string synthesis is implemented out of the box.

Water droplet and bar model is also available if compiled with Soundpipe.

#### Karplus-Strong

The initial noise source is filtered by a low-pass moog filter type or a string resonator (when compiled with Soundpipe; this provide higher quality output)

This is a fast method which generate pleasant string-like sounds.

#### Droplet

Integral part of blue / alpha component correspond to the first / second resonant frequency (main resonant frequency is tuned to current vertical pixel position), fractional part of blue component correspond to damping factor and amount of energy to add back for the alpha component.

Number of 'tubes' can be changed from instrument parameter 3.
Period of time over which all sound is stopped can be changed from instrument parameter 4. (non-realtime)

#### Metal bar

Approximate metal bar being struck, integral part of blue component correspond to decay, integral part of alpha component correspond to strike spatial width (normalized into [0,1000] range), fractional part of the blue component is the scanning spped of the output location, fractional part of the alpha component is the position along bar that strike occurs.

Boundary condition at left end of bar can be changed through intrument parameter 3. (non-realtime)
Boundary condition at right end of bar can be changed through intrument parameter 4. (non-realtime)

Normalized strike velocity can be changed from instrument parameter 5.

#### RGBA interpretation (Karplus-Strong)

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the LEFT channel    |
|          G | Amplitude value of the RIGHT channel   |
|          B | Noise wavetable cutoff lp filter / fractional part : stretching factor       |
|          A | Noise wavetable res. lp filter / feedback amount with Soundpipe        |

### Bandpass synthesis

Only available with Soundpipe.

Specific type of synthesis which use a canvas-mapped bank of bandpass filters (second-order Butterworth), each activated filters use an user-defined channel or instrument as source.

It can be used with rich form of synthesis (subtractive etc.) as a spectrum sculpt tool (vocoding etc.)

Bandwidth can be adjusted individually through alpha channel value which is a factor of current bank gap.

As a speed example ~300 filters can be enabled at the same time with ~6 subtractive oscillators as input on an i7 6700 with a single FAS instance (96000kHz)

Fractional part of the blue channel can be used to target a channel (> 0) or an instrument (= 0)

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the input LEFT channel    |
|          G | Amplitude value of the input RIGHT channel   |
|          B | integral part : source channel / instrument index   |
|          A | bandwidth factor : a value of 1 mean a bandwidth of current bank above + below gap |

### Formant synthesis

Only available with Soundpipe.

Specific type of synthesis which use a canvas-mapped bank of formant filters, each activated formant filters use an user-defined channel as source. It can be used to mimic speech.

It is similar to bandpass mode with a different algorithm.

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the input LEFT channel    |
|          G | Amplitude value of the input RIGHT channel   |
|          B | integral part : source channel index / fractional part : Impulse response attack time (in seconds)       |
|          A | Impulse reponse decay time (in seconds)        |

### Phase Distorsion synthesis

Only available with Soundpipe.

Specific type of synthesis which use an user-defined source channel or instrument as input and produce waveform distorsion as output.

Fractional part of the blue channel can be used to target a channel (> 0) or an instrument (= 0)

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the input LEFT channel    |
|          G | Amplitude value of the input RIGHT channel   |
|          B | integral part : source channel / instrument index       |
|          A | Amount of distorsion [-1, 1]        |

### String resonator synthesis

Only available with Soundpipe.

Specific type of synthesis which use a canvas-mapped bank of string resonator, each activated filters use an user-defined channel or instrument as source. It produce sounds similar to physical modelling / modal synthesis.

A list of frequencies for several instruments are available [here](http://www.csounds.com/manual/html/MiscModalFreq.html)

A high feedback gain will create a slower decay and a more pronounced resonance.

As an easy first step a noisy sound such as one produced with subtractive synthesis may be used.

Fractional part of the blue channel can be used to target a channel (> 0) or an instrument (= 0)

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the input LEFT channel    |
|          G | Amplitude value of the input RIGHT channel   |
|          B | integral part : source channel / instrument index       |
|          A | feedback gain; typically > 0.9        |

### Modal synthesis

Only available with Soundpipe.

Specific type of synthesis which use a canvas-mapped bank of resonant filters, each activated resonant filters use an user-defined channel or instrument as source. It produce sounds similar to physical modelling.

A list of frequencies for several instruments are available [here](http://www.csounds.com/manual/html/MiscModalFreq.html)

A high Q factor will make the sound more "resonant".

As an easy first step a noisy sound such as one produced with subtractive synthesis may be used.

Note : Due to stabilization filter bank frequency will be tresholded when it match that condition : (samplerate / filter_frequency) < pi

Fractional part of the blue channel can be used to target a channel (> 0) or an instrument (= 0)

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the input LEFT channel    |
|          G | Amplitude value of the input RIGHT channel   |
|          B | integral part : source channel / instrument index       |
|          A | Q factor of the resonant filter        |

### Input

This just play an input channel. Typically used in conjunction with formant / modal / bandpass / pd synthesis and effects.

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | Amplitude value of the LEFT channel    |
|          G | Amplitude value of the RIGHT channel   |
|          B | integral part : source channel index       |
|          A | Unused        |

### Modulation

This is a special type of synthesis which does not output any sounds.

It is instead used to provide fx / instrument modulation.

Instrument settings :

* p0 : modulation mode
  * 0 : Effects
  * 1 : Instrument settings
* p1 : Target channel or instrument
* p2 : Target fx slot or instrument parameter
* p3 : Target fx parameter (effects mode only)
* p4 : Easing method (interpolation between values)
  * 0 : linear
  * 1 to 3 : quadratic ease in/out/in out
  * 4 to 6 : cubic ease in/out/in out
  * 7 to 9 : quartic ease in/out/in out
  * 10 to 12 : quintic ease in/out/in out
  * 13 to 15 : sine ease in/out/in out
  * 16 to 18 : circular ease in/out/in out 
  * 19 to 21 : exponential ease in/out/in out 
  * 22 to 24 : elastic ease in/out/in out 
  * 25 to 26 : back ease in/out/in out 
  * 27 to 29 : bounce ease in/out/in out
  * any others value : no interpolation

This is provided as a shortcut solution to extend modulation options (modulation can also be done flexibly through instrument / fx synth commands), main disadvantage is the usage of an instrument slot which will increase the amount of data transmitted (thus bandwidth usage may increase and performance may degrade)

Simple use case would be to modulate filters cutoff / resonance parameter or wavetable selection for FM/PM.

Note : Parameters which require re-allocation (eg. convolution file, delay comb time) cannot be modulated.

#### RGBA interpretation

| Components | Interpretations                        |
| ---------: | :------------------------------------- |
|          R | > 0 to modulate    |
|          G | > 0 to modulate   |
|          B | unused       |
|          A | Modulation value / wave data        |

### Faust

[Faust](https://faust.grame.fr/) is embedded (when compiled with `WITH_FAUST`) and allow to dynamically extend FAS bank generators and effects with custom one written with the Faust DSP specification language.

Faust DSP focused language is simple and intuitive to learn and produce highly optimized effects and generators. Faust documentation is available [here](https://faust.grame.fr/doc/manual/index.html)

FAS look and load any Faust DSP code (*.dsp) at startup in the `faust/generators` and `faust/effects` directories. FAS can also reload Faust code dynamically when the appropriate ACTION packet is received.

All Faust DSP generators will be registered into the special instrument type Faust, instrument settings parameter 0 can then be used to switch between generators, generators with two inputs also work in this case the blue integer part will be used to select the source channel / instrument and its fractional part to switch between channel (> 0) / instrument mode.

All Faust DSP effects will be registered into the special effect type Faust, the first effect parameter can then be used to switch between effects.

Generators code will be hooked to the synthesis part of the sound engine while effects code will be hooked to the fx chain part.

Some generators and effects already exist and extend FAS with bandpass filters bank and so on...

FAS to Faust DSP parameters can be specified through [nentry](https://faust.grame.fr/doc/manual/#nentry-primitive) interface primitive and are used to transfer note / initial generator data.

Here is a list of usable Faust **generators** `nentry` key :

Generator data (when FAS oscillators bank is initialized; depend on canvas settings) :

* `fs_frequency` : bank generator frequency
* `fs_bw` : bank generator bandwidth

Note data :

* `fs_r` : RED
* `fs_b` : BLUE
* `fs_g` : GREEN
* `fs_a` : ALPHA

Those can be usefull to detect note-on events thus acting as trigger (when both equals to 0) :

* `fs_pr` : PREVIOUS RED
* `fs_pg` : PREVIOUS GREEN

Instrument data :

* `fs_p0` : parameter 1
* `fs_p1` : parameter 2
* `fs_p2` : parameter 3
* `fs_p3` : parameter 4

Here is simple example of a stereo Faust generator which add a bandlimited pulse wave oscillator to the bank with controllable L/R duty cycle through BLUE and ALPHA channels :

```
import("stdfaust.lib");

freq = nentry("fs_freq",440,0,96000,1);
b = nentry("fs_b",0,0,1,0.01) : si.smoo;
a = nentry("fs_a",0,0,1,0.01) : si.smoo;
process = os.pulsetrain(freq, b),os.pulsetrain(freq, a);
```

Here is a list of usable Faust **effects** `nentry` key :

* `fs_p0` : parameter 0
* `fs_p1` : parameter 1
* `fs_p2` : parameter 2
* `fs_p3` : parameter 3
* `fs_p4` : parameter 4
* `fs_p5` : parameter 5
* `fs_p6` : parameter 6
* `fs_p7` : parameter 7
* `fs_p8` : parameter 8
* `fs_p9` : parameter 9

All FAS pre-defined algorithms can be rewritten as Faust DSP code which mean that one could produce a light version of FAS with all the pre-defined algorithms removed and only make use of custom Faust DSP code.

Note : Faust DSP code cannot be used to extend available synthesis methods which mean that using Faust to extend per partial effects or add filters to subtractive synthesis is not possible.

### Samples map

Each samples loaded from the `grains` directory are processed, one of the most important process is the sample pitch mapping, this process try to gather informations or guess the sample pitch to map it correctly onto the user-defined image height, the guessing algorithm is in order :

1. from the filename, the filename should contain a specific pattern which indicate the sample pitch such as `A#4` or an exact frequency between "#" character such as `flute_#440#.wav`
2. same as above but with file path (note : may take the first pattern found on nested directories with multiple patterns)
3. with Yin pitch detection algorithm, this method work ok most of the time but can be inaccurate, depend on the sample content and yin parameters which are actually fixed right now, only awailable when compiled with `WITH_AUBIO`

The `waves` directory should only contain single cycle waveforms, the pitch is automatically detected from the sample length / samplerate informations.

Supported file formats are available [here](http://www.mega-nerd.com/libsndfile/) (libsndfile)

### Effects

This synthesizer support unlimited (user-defined maximum at compile time) number of effects chain per channels with bypass support, all effects (phaser, comb, reverb, delay...) come from the Soundpipe library which is thus required for effects usage.

Convolution effect use impulses response which are audio files loaded from the `impulses` directory (mono / stereo), free high quality convolution samples from real world places can be found [here](https://openairlib.net/).

Most effects are stereophonic with dry/wet controls, some may still appear with monophonic settings because their parameters are not yet mapped for stereo but they are still computed in stereo, most delay / reverb parameters are available in stereo which is usefull to build effects such as stereo width (stereo widening) which are a combination of different reverb / delay effect with different parameters for L/R channels.

### Performances

This program is tailored for performances, it is memory intensive (about 512mb is needed without samples and 8 instruments max, about 1 Gb with few samples, about 2.5 Gb with samples and 32 instruments max, memory requirement will have a major increase when `max_instrument` and frame queue size command line argument is increased), all real-time things are pre-allocated or pre-computed with zero real-time allocations.

FAS should be compiled with Soundpipe for best performance / high quality algorithms; for example subtractive moog filter see 3x speed improvement compared to the standalone algorithm.

FAS should also be compiled with Faust which may provide high quality / performance algorithms, using a huge number of generators and effects may vastly affect memory requirements however.

A fast, reliable, low latency Gigabit connection is recommended.

Whole bank height RGBA data for each used instruments is sent as-is by the client and this data is transformed when received so there may be lots of data to transfer, this is by design to not add additional processing on the client side, it is recommended to use gzip compression to reduce data size. (command-line parameter) Maybe the transform should happen on the client side to get rid of heavy bandwidth requirements...

Poor network transfer rate limit the number of instruments / the frequency resolution (frame height) / number of events to process per seconds, a Gigabit connection is good enough for most usage, for example with a theorical data rate limit of 125MB/s and without packets compression (`deflate` argument) it allow a configuration of 8 instruments with 1000px height slices float data at 60 fps without issues and beyond that (2000px / 240fps or 16 instruments / 1000 / 240fps), 8-bit data could also be used to go beyond that limit through Gigabit. This can go further with packets compression at the price of processing time.

Poor network latency may heavily limit the events rate especially if it is not on the same machine, can be solved by reducing data size or reducing amount of instruments / frame height / fps.

#### Raspberry PI

FAS was tested on a [Raspberry Pi 3B](https://www.raspberrypi.org/) with a [HifiBerry](https://www.hifiberry.com/) DAC for example, ~500 additive synthesis (wavetable) oscillators can be played simultaneously on the Raspberry Pi with four cores and minimum Raspbian stuff enabled, it can probably go beyond by using the magic circle algorithm.

It was also tested on [NapoPI NEO 2](https://www.friendlyarm.com/index.php?route=product/product&product_id=180) and [NanoPI Fire 3](https://www.friendlyarm.com/index.php?route=product/product&product_id=206) boards.

#### Distributed/multi-core synthesis

Due to the architecture of FAS, distributed sound synthesis is made possible by running multiple FAS instances on the same or different computer by distributing the pixels data correctly to each instances, on the same machine this only require a sufficient amount of memory.

This is the only way to exploit multiple cores on the same machine.

This need a relay program which will link each server instances with the client and distribute each events to instances based on a distribution algorithm.

A directly usable implementation with NodeJS of a distributed synthesis relay can be found [here](https://github.com/grz0zrg/fsynth/tree/master/fas_relay)

This feature was successfully used with cheap small boards clusters of [NapoPI NEO 2](https://www.friendlyarm.com/index.php?route=product/product&product_id=180) and [NetJack](https://github.com/jackaudio/jackaudio.github.com/wiki/WalkThrough_User_NetJack2) in a setup with 10 quad-core ARM boards + i7 (48 cores) running, linked to the NetJack driver, it is important that the relay program run on a powerfull board with (most importantly) a good Gigabit Ethernet controller to reduce latency issues.

Note : All instruments which use input instrument or channel are computed on each instances, this probably kill any performance gain when using alot of these instruments.

#### Frames drop

Frames drop happen if the client is too late sending its slices per frame (this is controlled by the `frames_queue_size` option parameter), different reasons can make that happen such as slow connectivity, client side issues like a slow client, etc.

When a frame is dropped, FAS hold the audio till a frame is received or the `max_drop` program option is reached, this ensure smooth audio even if the client has issues sending its frames, latency can be heard if too much frames are dropped however.

### Limitations

Only one client is supported at the moment, the server will refuse any more connection if one client is connected, you can launch multiple servers instance on different port and feed it different data if you need a multi-client audio server

### What is sent

The server send the CPU load of the stream as a percentage at regular interval (adjustable) and stream latency (ms) to the client :

```c
struct _stream_infos {
  int packet_id; // 0
  int stream_load; // [0, 100]
  double stream_latency; // ms
}
```

### Offline rendering (WIP)

TODO

### Future

The ongoing development is to add support for offline rendering, improve Faust integration / add more Faust *.dsp, have the option to use OSC instead of Websockets.

There is also minor architectural / cleanup work to do.
There is also continuous work to do on improving analysis / synthesis algorithms.

### Jack

FAS can use Jack instead of PortAudio (see [Build](#build)), this is recommended under Linux / embedded projets for maximum audio efficiency / reliability.

Using Jack allow unlimited output / input channels so get rid of the PortAudio device channels limitation.

Use `--output_channels` and `--input_channels` command-line arguments to configure Jack input / output ports.

Note : FAS ports are not connected automatically. (`qjackctl` program can be used to connect ports)

## Technical implementation

The architecture is done so there is **zero memory allocation** done in the audio callback, there is also **zero locks mechanism**, all communications between main thread and audio callback is done using a mixture of lock-free algorithms with freelist to avoid memory allocations.

Only one memory allocation is done in real-time in the network thread to assemble fragmented packets, some non-realtime actions such as samples reload and global synth settings change like bank height / remap do memory allocation.

The audio thread has two state *pause* and *play* (with smooth transition) which are handled through an atomic type.
It also has a transitionary flush state which make sure no data is being used on the audio thread and pause it.

The audio thread contain its own synth. data structure defined globally as `curr_synth`, when the audio thread is paused `curr_synth` can be accessed anywhere otherwise the audio thread has an exclusive access.

RGBA frames coming from the network are converted into a notes list (an array) which just tell which oscillator from the oscillator bank will be enabled during the *note time*.
Once processed the notes list is made available to the audio thread by pushing it into a lock-free ring buffer which ensure thread safety.

There is a simple "sync" mechanism which compute time between frames and accumulate it, skipping any frames below computed *note time* (computed from FPS parameter), this is a good enough solution but may have some small latency edge cases due to network latency.

A free list data structure is used for efficient notes data reuse; the program use a pre-allocated pool of notes buffer. This is actually the most memory hungry part because all is pre-allocated and the allocation size depend on slice height times `fas_max_instruments` parameter times note structure times frames queue size parameter... could probably be optimized by reducing note structure size as there is alot of pre-defined stuff made for convenience and readability.

The audio thread make the decision to consume notes data at audio sample level based on a computed *note time* which is defined by the FPS parameter configurable from synth. settings, once a note is consumed it is pushed back to the notes pool.

When notes data are not available the audio thread will continue with the latest one during the next *note time*, this ensure smooth audio but may introduce some delay.
When notes data are not available from some amount of time defined by the max drop parameter the audio will simply stop abruptely, this may indicate performance issues.

Most audio output critical changes use linear interpolation to ensure smooth audio in all conditions, this interpolation is computed once per audio frame and occur in-between note events. A linear interpolation smooth factor can be applied to sharpen the transitions.

Sound synthesis is processed with minimal computation / branching, values which depend on note parameters change are pre-computed per-instrument / oscillator in a dedicated processing block outside synthesis block, notes change and associated parameters happen at sample accurate note time level defined by the FPS parameter configurable from synth. settings.

There is a generic thread-safe (altough not really lock-free) commands queue for synth. parameters change (gain, etc.) which is used to pass data from the network thread to the audio callback.

Some non-critical real-time change command relative to synthesis / effects parameters (like spectral window size) will trigger a short pause due to allocation being performed in the network thread while the audio thread is paused.

There is a stream watcher thread which just check the audio callback state and inform whenever it is dropped. (due to xrun etc.)

Parameters are generally bounded for filters to ensure stability. (altough there may be some unstable cases left)

Additive synthesis is wavetable-based, a [magic circle](https://github.com/ccrma/chugins/blob/master/MagicSine/MagicSine.cpp) based sine generator is also available when `MAGIC_SINE` is enabled, this may be faster on some platforms.

Spectral synthesis use [afSTFT](https://github.com/jvilkamo/afSTFT) library which is bundled

Real-time resampling is done with a linear method, granular synthesis can also be resampled by using cubic interpolation method (uncomment the line in `constants.h`; slower than linear).

All synthesis algorithms (minus [PolyBLEP](http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/) and Soundpipe provided) are customs.

This program is regularly checked with Valgrind and should be free of memory leaks.

## Packets description

To communicate with FAS with a custom client, there is only six type of packets to handle, the **first byte of the packet is the packet identifier** with 7 bytes padding, below is the expected data for each packets

**Note** : bank settings packet must be sent before sending any frames, otherwise the received frames are ignored. Bank settings packet is a mandatory packet before producing any sounds.

**Note** : When using a custom client incoming packets data can be debugged by compiling FAS in debug mode, all packets / data changes are printed on standard output.

Bank settings, packet identifier 0 (audio may be paused for a short amount of time):
```c
struct _bank_settings {
    unsigned int h; // image/slice height
    unsigned int octave; // octaves count
    unsigned int data_type; // the frame data type, 0 = 8-bit, 1 = float
    double base_frequency;
};
```

Frame data, packet identifier 1 (real-time) :
```c
struct _frame_data {
    unsigned int instruments; // instruments count
    unsigned int padding; // 4 bytes padding
    // Note : the expected data length is computed by : (4 * (_synth_settings.data_type * sizeof(float)) * _synth_settings.h) * fas_max_instruments
    // Note : expected data length is the maximum amount of data which can be received, generally only the used instruments data will be sent
    // Example of amount of data with one instrument used (L/R) and a 8-bit image with height of 400 pixels : (4 * sizeof(unsigned char) * 400)
    void *rgba_data;
};
```

Synth settings, packet identifier 2 (real-time):

```c
struct _cmd_synth_settings {
  // target parameter
  // 0 : the rate at which events are processed (roughly correspond to the data stream rate / monitor refresh rate aka FPS, default to 60)
  // 1 : synth stereo gain [0, 1), default to 0.05
  unsigned int target;
  unsigned int padding; // 4 bytes padding

  double value; // target value
};
```

Synth channels settings, packet identifier 3 (real-time) :

```c
struct _cmd_chn_settings {
    unsigned int chn; // target channel
    
    // target parameter
    //  0 : wether this channel is muted or not, a muted channel will not output any sounds but is still available as an input, this is useful for methods that use an input channel as source
    //  1 : device output channel : for max. performance the value should be set to -1 to indicate to skip this channel processing when there is no instruments bound to this channel
    unsigned int target;

    double value; // target value
};
```

Synth channels fx settings, packet identifier 4 (real-time) :

```c
struct _cmd_chn_fx_settings {
    unsigned int chn; // target channel
    unsigned int slot; // target fx slot (up to FAS_MAX_FX_SLOTS found in constants.h)
    // target parameter
    // 0 : fx id, a value of -1 indicate the end of the fx chain, mapping can be found in constants.h (FX_CONV etc.)
    // 1 : bypass fx
    // 2 to up to FAS_MAX_FX_PARAMETERS found in constants.h : fx parameters (mapping is not yet documented but can be found easily in updateEffectParameter function of effects.c, non-realtime parameters can also be found in other functions)
    unsigned int target;
    unsigned int padding; // 4 bytes padding
    double value; // target value
};
```

Note : The fx chain is handled linearly and must be managed by the client, creation of a fx slot is done by sending the fx id slot value then sending the stop slot value (which will have a fx id value of -1), deletion of a slot is handled automatically when an existing slot receive a fx id value of -1 then all slots after that value will be shifted down the chain by 1.

Note : Some effect parameters may actually need an effect initialization which will pause audio for a short amount of time. Example : Convolution impulse index; delay max del; comb filter loop time

Instrument settings, packet identifier 6 (real-time) :

```c
struct _cmd_instrument_settings {
    unsigned int instrument; // target instrument
    
    // target parameter
    //  0 : synthesis method, mapping can be found in constants.h (FAS_ADDITIVE etc.)
    //  1 : wether this instrument is muted or not, a muted instrument will not output any sounds but is still available as an input, this is useful for methods that use an input instrument as source
    //  2 : Output virtual channel index (must be within fas_max_channels to work)
    //  3 : parameter
    //        Granular : granular envelope type for this instrument (there is 13 types of envelopes)
    //        Subtractive : filter type  (require Soundpipe)
    //        Physical modelling : Physical model type (require Soundpipe)
    //        Spectral : input channel
    //        Faust : Generator ID
    //  4 : parameter
    //        Granular : grain duration (min. bound)
    //        Spectral : window size
    //        Physical modelling droplet : Number of tubes
    //        Physical modelling bar : Boundary condition at left end of bar
    //        Faust : fs_p0
    //  5 : parameter
    //        Granular : grain spread (max. bound)
    //        Spectral : mode
    //        Physical modelling droplet : deattack parameter
    //        Physical modelling bar : Boundary condition at right end of bar
    //        Faust : fs_p1
    //  6 : parameter
    //        Granular : grain spread
    //        Physical modelling bar : Normalized strike velocity
    //        Faust : fs_p2
    //        Spectral : source mode
    //  7 : parameter
    //        Faust : fs_p3
    unsigned int target;

    double value; // target value
};
```

Note : Some instrument parameters may actually need an initialization which will pause audio for a short amount of time. Example : Spectral window size, Physical modelling drop deattack, Physical modelling bar boundary condition

Server actions, packet identifier 5 (audio may be paused for a short amount of time on any reload actions otherwise it is real-time):

```c
struct _synth_action {
    // 0 : reload samples in the grains directory
    // 1 : note re-trigger (to reinitialize oscillators state on note-off, mostly used for Karplus-Strong / Wavetable, real-time)
    // 2 : reload Faust generators
    // 3 : reload Faust effects
    // 4 : pause audio
    // 5 : resume audio
    // 6 : reload waves
    // 7 : reload impulses
    unsigned char type; // + 7 bytes padding
    unsigned int instrument; // only for re-trigger action; target instrument
    unsigned int note; // only for re-trigger action; target note (height y index)
};
```

## Build

FAS make use of the [CMake](https://cmake.org/) build system.

Under Windows, [MSYS2](https://msys2.github.io/) with mingw32 is used and well tested. Since the build system now use cmake and Windows build is not yet tested there may be minor issues.

Requirements :

 * [PortAudio](http://www.portaudio.com/download.html) or [Jack](https://jackaudio.org/)
 * [liblfds](http://liblfds.org/)
 * [libwebsockets](https://libwebsockets.org/)
 * [aubio](https://www.aubio.org/)
 * [libsndfile](https://github.com/erikd/libsndfile)
 * [libsamplerate](https://github.com/erikd/libsamplerate)
 * [liblo](http://liblo.sourceforge.net/) (Optional)
 * [Soundpipe](https://github.com/PaulBatchelor/Soundpipe) (Optional)
 * [Faust](https://github.com/grame-cncm/faust) (Optional)
   * Faust depend on LLVM as well

FAS also make use of [tinydir](https://github.com/cxong/tinydir) [lodepng](https://github.com/lvandeve/lodepng) and [afSTFT](https://github.com/jvilkamo/afSTFT) (all of them bundled)

Compiling requirements for Ubuntu/Raspberry Pi/Linux with PortAudio :

 * Get latest [PortAudio v19 package](http://www.portaudio.com/download.html)
   * sudo apt-get install libasound-dev jackd qjackctl libjack-jackd2-dev
   * uncompress, go into the directory
   * ./configure
   * make clean
   * make
   * sudo make install
 * Get latest [liblfds 7.1.1 package](http://liblfds.org/)
   * uncompress into **lib** directory, go into the directory "liblfds711"
   * go into the directory "build/gcc_gnumake"
   * make
 * Get latest [libwebsockets 2.2.x package](https://libwebsockets.org/) from github
   * sudo apt-get install cmake zlib1g-dev
   * go into the libwebsockets directory
   * mkdir build
   * cd build
   * cmake .. -DLWS_WITH_SSL=0 -DLWS_WITHOUT_CLIENT=1
   * make
   * sudo make install
* Get latest [libsamplerate](http://www.mega-nerd.com/SRC/download.html)
  * you may need to specify the build type on configure (example for NanoPi NEO2 / Raspberry PI : `./configure --build=arm-linux-gnueabihf`)
  * you may need to install libfftw : `sudo apt-get install libfftw3-dev`
  * uncompress, go into the directory "libsamplerate-0.1.9"
  * ./configure
  * make
  * sudo make install
* Get latest [Soundpipe](https://github.com/PaulBatchelor/Soundpipe)
  * make
  * sudo make install
* Get latest [Faust](https://github.com/grame-cncm/faust)
  * sudo apt-get install cmake llvm libmicrohttpd-dev
  * make all
  * sudo make install
* For automatic pitch detection of sample files get aubio from your package manager (can also be installed from sources but instructions have yet to be written)
* libsndfile can be installed through package manager

Some dependencies can also be installed through the operating system packages manager. (may have some issues with deflate option and some libwebsockets packages, this is resolved by compiling libwebsockets 2.2.x, may also have issues compiling Faust, usually solved by installing a specific version of LLVM)

On ARM64 you must use liblfds 7.2.0 (by passing `-DLIBLFDS720` to cmake) which is provided in the lib directory, this is a not yet released version and potentially unstable for anything else, it is only provided to provide FAS under ARM64 platforms and is not guaranteed to work for anything else.

To compile liblfds720 (ARM64 only):

* cd lib/liblfds7.2.0/src/liblfds720/build
* make

Once all dependencies are installed one can run `cmake` followed by `make` in the build directory :

`cd build && cmake -DCMAKE_BUILD_TYPE=Release && make`

`fas` binary should then be available in the `bin` directory

FAS can then be installed with `sudo make install` in the build directory

FAS will load grains / waves / impulses first by checking `/usr/local/share/fragment/` default install path (specifically `grains` `waves` `impulses` directories) and when they are not available will look into the binary directory.

FAS will load Faust files first by checking `/usr/local/share/fragment/` default install path (specifically `faust/generators` `faust/effects` directories) and when they are not available will look into the binary directory.

Recommended launch parameters with HiFiBerry DAC+ :
    ./fas --frames_queue_size 63 --sample_rate 48000 --device 2
Bit depth is fixed to 32 bits float at the moment.

## CMake options

There is some cmake build options available to customize features :

 * `-DLIBLFDS720` : Use the provided liblfds 7.2.0 library (for ARM64 support)
 * `-DWITH_JACK` : Use Jack driver instead of PortAudio (may be faster)
 * `-DWITH_FAUST` : Use Faust
 * `-DWITH_SOUNDPIPE` : Use Soundpipe
 * `-DWITH_AUBIO` : Use automatic pitch detection
 * `-DMAGIC_CIRCLE` : Use additive synthesis magic circle oscillator (may be faster than wavetable on some platforms; no bandlimited noise for per partial effects)
 * `-DPARTIAL_FX`: Use additive synthesis per partial effects
 * `-DINTERLEAVED_SAMPLE_FORMAT` : Use interleaved sample format
 * `-DUSE_NEON` : Use NEON instructions (optimizations; ARM platforms)
 * `-DUSE_SSE` : Use SSE instructions (optimizations; Desktop platforms)
 * `-DUSE_DOUBLE` : Use double precision for all internal computations (note : must probably compile dependencies such as Soundpipe and Faust as well when it is defined)

By default FAS build with `-DWITH_FAUST -DWITH_AUBIO -DWITH_SOUNDPIPE -DMAGIC_CIRCLE -DPARTIAL_FX -DINTERLEAVED_SAMPLE_FORMAT`

## Usage

You can tweak this program by passing parameters to its arguments, for command-line help : **fas --h**

A wxWidget user-friendly launcher is also available [here](https://github.com/grz0zrg/fas_launcher) (Note : may not support all options)
 
Usage: fas [list_of_parameters]
 * --i **print audio device infos**
 * --sample_rate 44100
 * --noise_amount 0.1 **the maximum amount of band-limited noise to add (wavetables only)**
 * --frames 512 **audio buffer size**
 * --wavetable_size 8192 **no effects if built with advanced optimizations option**
 * --smooth_factor 1.0 **this is the samples interpolation factor between frames, a high value will sharpen sounds attack / transitions (just like if the stream rate / FPS was higher), a low value will smooth it (audio will become muddy)**
 * --max_instruments 24 **this is the maximum amount of instruments that can be used, may increase memory consumption significantly**
 * --max_channels 24 **this is the maximum amount of virtual channels that can be used, may increase memory consumption significantly**
 * --ssl 0
 * --deflate 0 **network data compression (add additional processing)**
 * --max_drop 60 **this allow smooth audio in the case of frames drop, allow 60 frames drop by default which equal to approximately 1 sec.**
 * --render target.fs **real-time pixels-data offline rendering, this will save pixels data to "target.fs" file**
 * --render_convert target.fs **this will convert the pixels data contained by the .fs file to a .flac file of the same name**
 * --grains_dir ./grains/
 * --granular_max_density 128 **this control how dense grains can be (maximum)**
 * --waves_dir ./waves/
 * --impulses_dir ./impulses/
 * --faust_gens_dir ./faust/generators
 * --faust_effs_dir ./faust/effects
 * --rx_buffer_size 8192 **this is how much data is accepted in one single packet**
 * --port 3003 **the listening port**
 * --iface 127.0.0.1 **the listening address**
 * --device -1 **PortAudio audio output device index or full name (informations about audio devices are displayed when the app. start)**
 * --input_device -1 **PortAudio audio input device index or full name (informations about audio devices are displayed when the app. start)**
 * --output_channels 2 **stereo pair**
 * --input_channels 2 **stereo pair**
 * --frames_queue_size 3 **important parameter, if you increase this too much the audio might be delayed and the memory requirement will increase significantly**
 * --commands_queue_size 512 **should be a positive integer power of 2**
 * --stream_infos_send_delay 2 **FAS will send the stream infos every two seconds**
 * --samplerate_conv_type -1 **see [this](http://www.mega-nerd.com/SRC/api_misc.html#Converters) for converter type, this has impact on samples loading time, this settings can be ignored most of the time since FAS do real-time resampling, -1 skip the resampling step**

**You can stop the application by pressing any keys while it is running on Windows or by sending SIGINT (Ctrl+C etc.) under Unix systems.**

https://www.fsynth.com


## Credits

* [L'audionumérique 3°ed by Curtis Road](http://www.audionumerique.com/)
