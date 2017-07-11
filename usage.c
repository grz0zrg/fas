#include "usage.h"

void print_usage() {
    printf("Usage: fas [list_of_settings]\n");
    printf(" possible settings with associated default value:\n");
    printf("  --i\n");
    printf("  --sample_rate %u\n", FAS_SAMPLE_RATE);
    printf("  --frames %u\n", FAS_FRAMES_PER_BUFFER);
    printf("  --noise_amount %f\n", FAS_NOISE_AMOUNT);
    //printf("  --wavetable %u\n", FAS_WAVETABLE);
#ifndef FIXED_WAVETABLE
    printf("  --wavetable_size %u\n", FAS_WAVETABLE_SIZE);
#endif
    printf("  --fps %u\n", FAS_FPS);
    printf("  --ssl %u\n", FAS_SSL);
    printf("  --deflate %u\n", FAS_DEFLATE);
    printf("  --rx_buffer_size %u\n", FAS_RX_BUFFER_SIZE);
    printf("  --port %u\n", FAS_PORT);
    printf("  --iface 127.0.0.1\n");
    printf("  --device -1\n");
    printf("  --output_channels %u\n", FAS_OUTPUT_CHANNELS);
#ifdef __unix__
    printf("  --alsa_realtime_scheduling %u\n", FAS_REALTIME);
#endif
    printf("  --frames_queue_size %u\n", FAS_FRAMES_QUEUE_SIZE);
    printf("  --commands_queue_size %u\n", FAS_COMMANDS_QUEUE_SIZE);
}
