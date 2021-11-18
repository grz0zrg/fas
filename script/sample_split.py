# this is an example script to split sound files from a directory into small chunks so it can be used in a wavetable
# files can be almost whatever (automatically detected by from_file pydub function)
# output goes into "out" directory
# need python > 3.4 and the pydub library : pip3 install pydub
# usage : python sample_split.py input_directory
import os
import sys
from pathlib import Path
from pydub import AudioSegment

script_directory = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'out')

if not os.path.exists(script_directory):
    os.makedirs(script_directory)

def triangular_window (i, N):
    return 1 - abs(2 * (i - 0.5 * (N - 1)) / N)

print(sys.argv[1])
for root, dirs, files in os.walk(sys.argv[1]):
    for file in files:
        input_file = os.path.join(root, file)
        input_filename = Path(file).name.lstrip()

        print('processing "' + input_filename + '"')

        # load sample
        audio = None
        try:
            audio = AudioSegment.from_file(input_file)
        except Exception as e:
            print('AudioSegment.from_file failed: ' + input_file)
        if not audio:
            continue
        audio_samples = audio.get_array_of_samples()

        # get informations
        audio_len = len(audio_samples)
        chunk_len = 1358 # 44100 / C1_frequency
        chunk_count = round(audio_len / chunk_len)

        # split in chunks
        for i in range(0, chunk_count):
            start_index = i * chunk_len
            stop_index = start_index + chunk_len
            audio_chunk = audio._spawn(audio_samples[start_index:stop_index])

            # apply triangular window on the chunk to remove crackles
            chunk_samples = audio_chunk.get_array_of_samples()
            chunk_len = len(chunk_samples)
            for k in range(0, chunk_len):
                chunk_samples[k] = int(chunk_samples[k] * triangular_window(k, chunk_len))
            audio_chunk = audio._spawn(chunk_samples)
            #

            # save
            audio_chunk.export('out/' + input_filename + '_' + str(i).rjust(10, '0') + '.wav', format="wav")

print("done")