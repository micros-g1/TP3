import scipy.signal as sgn
import scipy
import numpy as np
import matplotlib.pyplot as plt


## constantes FSK
theta = 0
f1 = 1.2e3
f2 = 2.2e3
fs = 12e3
T = 1 / f1
times = np.arange(0, T, 1 / fs)

## constantes demodulador
delay = 446e-6
D = int(delay/(1/fs))



## constantes filtro
Aa = 70
fp = 1.2e3
fa = 2*fp
N = 25
#N, beta = sgn.kaiserord(Aa, (fa-fp)/(fs/2))
taps = sgn.firwin(N, fp/(fs/2), window=("chebwin", 55))
for tap in taps:
	print('{0:.30e}'.format(tap))
w, h = sgn.freqz(taps, worN=8000)
plt.plot((w/np.pi)*(fs/2), 20*np.log10(np.absolute(h)), linewidth=2)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Gain')
plt.title('Frequency Response')
#plt.ylim(-0.05, 1.05)
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


def filt(x):
	prod = np.multiply(x, np.concatenate((np.zeros(D), x[:-D])))
	y = np.convolve(prod, taps, mode='same')
	return y


# x = fsk_word([0, 1, 0, 1, 0, 1, 0, 1])
# # x = fsk_word([0 for _ in range(8)])
# t = np.arange(0, 15*T, 1/fs)
# plt.plot(t[:len(x)], x, label="original")
# # y, prod = filt(x)
# # plt.plot(t[:len(prod)], np.concatenate((np.zeros(D), x[:-D])), label="delay")
# # plt.plot(t[:len(prod)], prod, label="producto")
# plt.plot(t[:len(y)], y, label="filtrado")
# plt.vlines([T*i for i in range(1, 11)], ymin=-2, ymax=2)
# #plt.vlines([delay], ymin=-2, ymax=2)
#
#
# comp = []
# i = 0
# while i < len(y):
# 	avg = 0
# 	n = int(T*fs)
# 	for _ in range(n):
# 		avg += y[i]
# 		i += 1
# 	avg /= n
# 	if avg > 0:
# 		comp += [0 for _ in range(n)]
# 	else:
# 		comp += [1 for _ in range(n)]
#
# plt.plot(t[:len(comp)], comp, label="comparador")
#
#
# plt.legend()
# plt.show()
