declare name "oberheimLPF";
declare description "Demonstration of the Oberheim Low-Pass Filter";
declare author "Eric Tarr";

import("stdfaust.lib");

Q = nentry("fs_p0",1,0.5,10,0.01);
normFreq = nentry("fs_p1",0.5,0,1,0.001) : si.smoo;

process = ve.oberheimLPF(normFreq,Q),ve.oberheimLPF(normFreq,Q);

