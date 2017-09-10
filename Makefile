compiler = gcc
cppcompiler = g++
source = main.c tools.c samples.c grains.c oscillators.c wavetables.c note.c usage.c Yin.c
cpp_source = essentia_wrapper.cpp
obj = main.o tools.o samples.o grains.o oscillators.o wavetables.o note.o usage.o Yin.o
cpp_obj = essentia_wrapper.o
cpp_options = -std=c++11
essentia_libs = libessentia.a -lfftw3 -lfftw3f
libs = liblfds711.a -lportaudio -lwebsockets -lrt -lm -lasound -ljack -pthread -lsndfile liblo.so.7.3.0
static_libs = liblfds711.a libportaudio.a libwebsockets.a -lz -lrt -lm -lasound -ljack -pthread -lsndfile liblo.so.7.3.0
ssl_libs = -lssl -lcrypto
win_ssl_libs = -lssl -lcrypto -lws2_32 -lgdi32
win_static_libs = liblfds711.a libwebsockets_static.a libportaudio.a -lm -lz -lws2_32
compat_options = -U__STRICT_ANSI__
output = fas
standard_options = -std=c11 -pedantic -D_POSIX_SOURCE
win_static_options = -static -static-libgcc
adv_optimization_options = -DFIXED_WAVETABLE
debug_options = -g -DDEBUG
include_path = -I lo -I inc
release_options = -O2

all:
	$(compiler) $(source) $(include_path) ${debug_options} ${standard_options} $(libs) -o $(output)

debug-o:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(libs) -o $(output)

debug-o-static:
	$(compiler) $(source) $(include_path) ${debug_options} ${adv_optimization_options} ${standard_options} $(static_libs) -o $(output)

debug-esentia:
	$(compiler) $(source) $(include_path) ${debug_options} ${standard_options} -c
	$(cppcompiler) $(cpp_source) -DWITH_ESSENTIA $(cpp_options) $(include_path) -c
	$(cppcompiler) $(obj) $(cpp_obj) $(static_libs) $(essentia_libs) -o $(output)

profile:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} -DPROFILE $(libs) -o $(output)

release:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

release-static:
	$(compiler) $(source) $(include_path) ${release_options} ${standard_options} $(static_libs) -o $(output)

release-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} ${standard_options} $(static_libs) -o $(output)

release-bln-static-o:
	$(compiler) $(source) $(include_path) ${release_options} ${adv_optimization_options} -DBANDLIMITED_NOISE ${standard_options} $(static_libs) -o $(output)

release-essentia-static:
	$(compiler) $(source) $(include_path) ${release_options} -DBANDLIMITED_NOISE ${adv_optimization_options} ${standard_options} -c
	$(cppcompiler) $(cpp_source) -DWITH_ESSENTIA $(cpp_options) $(include_path) ${release_options} -c
	$(cppcompiler) $(obj) $(cpp_obj) $(static_libs) $(essentia_libs) -o $(output)

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

32:
	$(compiler) -m32 $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

64:
	$(compiler) -m64 $(source) $(include_path) ${release_options} ${standard_options} $(libs) -o $(output)

run: all
	./fas

run-release: release-static
	./fas
