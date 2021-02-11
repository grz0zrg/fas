#include "usage.h"

void print_usage() {
    printf("Usage: fas [list_of_parameters]\n");
    printf(" possible parameters with associated default value:\n");
    printf("  --i\n");
    printf("  --sample_rate %u\n", FAS_SAMPLE_RATE);
    printf("  --frames %u\n", FAS_FRAMES_PER_BUFFER);
    printf("  --noise_amount %f\n", FAS_NOISE_AMOUNT);
    //printf("  --wavetable %u\n", FAS_WAVETABLE);
    printf("  --smooth_factor %f\n", FAS_SMOOTH_FACTOR);
    printf("  --ssl %u\n", FAS_SSL);
    printf("  --deflate %u\n", FAS_DEFLATE);
    printf("  --rx_buffer_size %u\n", FAS_RX_BUFFER_SIZE);
    printf("  --port %u\n", FAS_PORT);
    printf("  --max_drop %u\n", FAS_MAX_DROP);
    printf("  --render my_session\n");
    printf("  --render_width %u\n", FAS_RENDER_WIDTH);
    printf("  --max_instruments %u\n", FAS_MAX_INSTRUMENTS);
    printf("  --max_channels %u\n", FAS_MAX_CHANNELS);
    //printf("  --render_convert main.fs\n");
    printf("  --iface 127.0.0.1\n");
    printf("  --input_device -1\n");
    printf("  --device -1\n");
    printf("  --grains_dir ./grains/\n");
    printf("  --waves_dir ./waves/\n");
    printf("  --impulses_dir ./impulses/\n");
    printf("  --faust_gens_dir ./faust/generators/\n");
    printf("  --faust_effs_dir ./faust/effects/\n");
    printf("  --faust_libs_dir ./faustlibraries\n");
    printf("  --granular_max_density %u\n", FAS_GRANULAR_MAX_DENSITY);
    printf("  --stream_infos_send_delay %u\n", FAS_STREAM_INFOS_SEND_DELAY);
    printf("  --input_channels %u\n", FAS_INPUT_CHANNELS);
    printf("  --output_channels %u\n", FAS_OUTPUT_CHANNELS);
    printf("  --frames_queue_size %u\n", FAS_FRAMES_QUEUE_SIZE);
    printf("  --commands_queue_size %u\n", FAS_COMMANDS_QUEUE_SIZE);
    printf("  --samplerate_conv_type %u\n", SRC_SINC_MEDIUM_QUALITY);
}
