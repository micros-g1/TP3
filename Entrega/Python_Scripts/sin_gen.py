import matplotlib.pyplot as plt
import numpy as np

fs = 100    # sample rate
freqs = {2: [], 4: []}   # the frequencies of the signals

x = np.arange(int(fs))    # the points on the x axis for plotting

for freq in freqs.keys():
    y = 4095 * np.sin(2*np.pi*freq * (x/fs))      # compute the value (amplitude) of the sin wave at the for each sample
    y = y.astype(dtype=int)
    aux_string = np.array2string(y, separator=', ')
    aux_string = '{' + aux_string[1:len(aux_string) - 1] + ' };'
    freqs[freq] = aux_string

with open("sinusoidals.h", "w") as text_file:
    text_file.write('//File generated using sin_gen.py\n\n')

    for freq in freqs.keys():
        text_file.write("const uint16_t sin_wave_freq_" + str(freq)+'_fs_' + str(fs) + '[] = ' + freqs[freq] + '\n\n')
