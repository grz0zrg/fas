// example of a stereo pulse wave with independant duty cycle L/R control

import("stdfaust.lib");

freq = nentry("fs_freq",0,0,44100,0.01);
// blue channel
b = nentry("fs_b",0,0,1,0.01) : si.smoo;
// alpha channel
a = nentry("fs_a",0,0,1,0.01) : si.smoo;
process = os.pulsetrain(freq, b),os.pulsetrain(freq, a);
