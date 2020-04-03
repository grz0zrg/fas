compiler = gcc
cppcompiler = g++
emcompiler = emcc -s WASM=1 -Oz -s ERROR_ON_UNDEFINED_SYMBOLS=0
x86-64-cross-compiler = /usr/bin/x86_64-w64-mingw32-gcc
source = main.c tools.c samples.c grains.c effects.c oscillators.c wavetables.c note.c usage.c lib/lodepng.c lib/Yin.c filters.c faust.c faust.h afSTFT/afSTFTlib.c afSTFT/fft4g.c afSTFT/vecTools.c
cpp_source = 
obj = main.o tools.o samples.o grains.o effects.o oscillators.o wavetables.o filters.o note.o usage.o Yin.o lodepng.o faust.o afSTFTlib.o fft4g.o vecTools.o
cpp_obj = 
cpp_options = -std=c++11
libs = liblfds711.a -lportaudio -lsamplerate -lwebsockets -lrt -lm -lasound -ljack -pthread -lsndfile
static_libs = liblfds711.a libportaudio.a libsamplerate.a libwebsockets.a -lz -lrt -lm -lstdc++ -lasound -ljack -pthread -lsndfile
soundpipe_libs = libsoundpipe.a
faust_libs = libfaust.a
ssl_libs = -lssl -lcrypto
osc_libs = liblo.so.7.3.0
win_ssl_libs = -lssl -lcrypto -lws2_32 -lgdi32
win_static_libs = liblfds711.a libwebsockets_static.a libportaudio.a -lm -lz -lws2_32
win_cross_libs = cross/liblfds711.a cross/libwebsockets_static.a cross/libportaudio.a cross/libzlib_internal.a cross/liblo.a cross/libsamplerate.a cross/libsndfile.a -lm -ldsound -lwinmm -lws2_32 -lstdc++ -lole32 -liphlpapi
compat_options = -U__STRICT_ANSI__
output = fas
standard_options = -std=c11 -pedantic -D_POSIX_SOURCE -DALSA_RT -DLFDS711
win_static_options = -static -static-libgcc
adv_optimization_options = -DFIXED_WAVETABLE
debug_options = -g -DDEBUG
include_path = -I lo -I inc -I inc/portaudio -I inc/soundpipe -I inc/faust -I lib
win_cross_include_path = -I lo -I inc -I inc/portaudio -I inc/soundpipe -I cross
release_options = -O2

all:
	$(compiler) $(source) $(include_path) ${debug_options} ${standard_options} $(libs) -o $(output)

web:
	$(emcompiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -DBANDLIMITED_NOISE ${standard_options} $(static_libs) -o $(output)

debug-o:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(libs) -o $(output)

debug-o-static:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(static_libs) -o $(output)

debug-static-sp-o:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(static_libs) ${soundpipe_libs} -DPARTIAL_FX -DWITH_SOUNDPIPE -o $(output)

debug-bln-static-sp-o:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} -DBANDLIMITED_NOISE -DWITH_SOUNDPIPE ${standard_options} $(static_libs) ${soundpipe_libs} -o $(output)

debug-soundpipe:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(static_libs) ${soundpipe_libs} -DWITH_SOUNDPIPE -o $(output)

debug-soundpipe-faust:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} ${faust_libs}  `llvm-config --libs --ldflags` $(static_libs) ${soundpipe_libs} -DMAGIC_CIRCLE -DPARTIAL_FX  -DWITH_SOUNDPIPE -DWITH_FAUST -o $(output)

profile:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} -DPROFILE $(libs) -o $(output)

release:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

release-osc:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} -DWITH_OSC $(libs) $(osc_lib) -o $(output)

release-static:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} $(static_libs) -o $(output)

release-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) -o $(output)

release-static-o-sse:
	$(compiler) -ftree-vectorize -msse2 -fopt-info-vec-optimized $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) -o $(output)

release-static-netjack-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -std=c11 -pedantic -D_POSIX_SOURCE $(static_libs) -o $(output)

release-static-osc-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} -DWITH_OSC $(static_libs) $(osc_libs) -o $(output)

release-static-sp-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) ${soundpipe_libs} -DPARTIAL_FX -DWITH_SOUNDPIPE -o $(output)

release-static-sp-faust-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} ${faust_libs}  `llvm-config --libs --ldflags` $(static_libs) ${soundpipe_libs} -DPARTIAL_FX -DWITH_SOUNDPIPE -DWITH_FAUST -o $(output)

release-static-sp-mc-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) ${soundpipe_libs} -DMAGIC_CIRCLE -DPARTIAL_FX -DWITH_SOUNDPIPE -o $(output)

release-static-sp-faust-mc-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} ${faust_libs}  `llvm-config --libs --ldflags` $(static_libs) ${soundpipe_libs} -DMAGIC_CIRCLE -DPARTIAL_FX -DWITH_SOUNDPIPE -DWITH_FAUST -o $(output)

release-bln-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -DBANDLIMITED_NOISE ${standard_options} $(static_libs) -o $(output)

release-bln-static-sp-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -DBANDLIMITED_NOISE -DWITH_SOUNDPIPE ${standard_options} $(static_libs) ${soundpipe_libs} -o $(output)

release-static-o-profile:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -DPROFILE ${standard_options} $(static_libs) -o $(output)

ssl-release-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) $(ssl_libs) -o $(output)

win-release-static:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} ${win_static_options} ${compat_options} $(win_static_libs) -o $(output)

win-release-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} ${win_static_options} ${adv_optimization_options} ${compat_options} $(win_static_libs) -o $(output)

ssl-win-release-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} ${win_static_options} ${adv_optimization_options} ${compat_options} $(win_static_libs) $(win_ssl_libs) -o $(output)

win-cross-x86-64:
	$(x86-64-cross-compiler) $(source) $(win_cross_include_path) ${release_options} ${compat_options} -DBANDLIMITED_NOISE ${adv_optimization_options} ${standard_options} $(win_cross_libs) -o fas.exe

32:
	$(compiler) -m32 $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

64:
	$(compiler) -m64 $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

run: all
	./fas

run-release: release-static
	./fas
