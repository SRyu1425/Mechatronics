import matplotlib.pyplot as plt
import numpy as np

#fast fourier transform - converts a signal, time dataset to signal, frequency
#computes the magnitude of different frequencies in a signal.
#algorithm allows us to plot which frequencies have the greatest amplitudes, to figure out where to filter
#the highest frequency that can show up = half the sampling frequency

#should not interpolate between these points because the frequency spectrum might not be smooth
#for example is the sampling frequency was 50 hz, then 100 hz signal will not have shown up
#between the sampled points, you don't know the behavior b/c higher freq signals will not have shown up

dt = 1.0/10000.0 # 10kHz - intervals between samples
t = np.arange(0.0, 1.0, dt) # 10s
# a constant plus 100Hz and 1000Hz
s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25


# # Calculate the sample rate
# num_samples = len(t)
# total_time = t[-1] - t[0]
# sample_rate = num_samples / total_time


Fs = 10000 # sample rate
Ts = 1.0/Fs; # sampling interval
ts = np.arange(0,t[-1],Ts) # time vector
y = s # the data to make the fft from
n = len(y) # length of the signal
k = np.arange(n)
T = n/Fs
frq = k/T # two sides frequency range
frq = frq[range(int(n/2))] # one side frequency range
Y = np.fft.fft(y)/n # fft computing and normalization
Y = Y[range(int(n/2))]

fig, (ax1, ax2) = plt.subplots(2, 1)
ax1.plot(t,y,'b')
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq,abs(Y),'b') # plotting the fft
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
plt.show()