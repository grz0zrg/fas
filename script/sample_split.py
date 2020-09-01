# this is an example script to split a sound file into small chunks so it can be used in a wavetable
# need the pydub library : pip3 install pydub
from pydub import AudioSegment

def triangular_window (i, N):
    return 1 - abs(2 * (i - 0.5 * (N - 1)) / N)

input_file = "female_french_numbers_1_10.wav"

# load sample
audio = AudioSegment.from_wav(input_file)
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
    audio_chunk.export('out/' + str(i).rjust(8, '0') + '.wav', format="wav")