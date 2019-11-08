import scipy.signal as sgn
import scipy
import numpy as np
import matplotlib.pyplot as plt


def gen_filter_coeffs(coeffs, len_coeffs):
	coeffs = coeffs.astype(dtype=float)
	aux_string = np.array2string(coeffs, separator=', ')
	aux_string = '{' + aux_string[1:len(aux_string) - 1] + ' };'
	with open("filter.h", "w") as text_file:
		text_file.write('//File generated using filtro.py\n\n')
		text_file.write('#define FILTER_SIZE	' + str(len_coeffs) + '\n\n')
		text_file.write('const float filter_coeffs[FILTER_SIZE] = ' + aux_string + '\n\n')


# constantes FSK
theta = 0
f1 = 1.2e3
f2 = 2.2e3
fs = 12e3
T = 1 / f1
times = np.arange(0, T, 1 / fs)

# constantes demodulador
delay = 446e-6
D = int(delay/(1/fs))

# constantes filtro
Aa = 70
fp = 1.2e3
fa = 2*fp
N = 25
# N, beta = sgn.kaiserord(Aa, (fa-fp)/(fs/2))
taps = sgn.firwin(N, fp/(fs/2), window=("chebwin", 55))

gen_filter_coeffs(taps, N)
# taps = sgn.firwin(N, fp/(fs/2), window='hann')
# print(taps)
sistema = sgn.dlti(taps, [1] + [0 for i in range(len(taps)-1)])
tout, step = sgn.dstep(sistema)

step = step[0]
step = [step[i][0] for i in range(len(step))]
plt.plot(tout, step)
plt.show()

# for tap in taps:
# 	print('{0:.30e}'.format(tap))
w, h = sgn.freqz(taps, worN=8000)
plt.plot((w/np.pi)*(fs/2), 20*np.log10(np.absolute(h)), linewidth=2)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Gain')
plt.title('Frequency Response')
# plt.ylim(-0.05, 1.05)
plt.grid(True)
plt.show()


def fsk_word(byte):
	x = fsk_bit(0)  # start
	parity = 1
	for bit in byte:
		parity ^= bit
		x += fsk_bit(bit)

	x += fsk_bit(parity)
	x += fsk_bit(1)  # stop

	return x


def fsk_bit(bit):
	global theta
	f = f1 if bit else f2
	theta0 = theta + 2 * np.pi * f / fs
	x = list(np.sin(2*np.pi*f*times + theta0))
	theta = theta0 + 2*np.pi*f*times[-1]
	while theta > 2*np.pi:
		theta -= 2*np.pi
	return x