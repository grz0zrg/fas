compiler = gcc
source = main.c
libs = liblfds710.a -lportaudio -lwebsockets -lrt -lm -lasound -ljack -pthread
static_libs = liblfds710.a libportaudio.a libwebsockets.a -lz -lrt -lm -lasound -ljack -pthread
output = fas
standard_options = -std=c11 -pedantic -D_POSIX_SOURCE
debug_options = -g -DDEBUG
release_options = -O2

all:
	$(compiler) $(source) ${debug_options} ${standard_options} $(libs) -o $(output)

release:
	$(compiler) $(source) ${release_options} ${standard_options} $(libs) -o $(output)

release-static:
	$(compiler) $(source) ${release_options} ${standard_options} $(static_libs) -o $(output)

32:
	$(compiler) -m32 $(source) ${release_options} ${standard_options} $(libs) -o $(output)

64:
	$(compiler) -m64 $(source) ${release_options} ${standard_options} $(libs) -o $(output)

run: all
	./fas

run-release: release-static
	./fas
