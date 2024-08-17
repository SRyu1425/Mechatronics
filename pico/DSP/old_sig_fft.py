import matplotlib.pyplot as plt
import numpy as np
import csv


#pass in what signal you want to make fft for
#create 2 plots (signal, time + signal, frequency)
def fft(signal):
    t = [] # column 0
    data1 = [] # column 1

    with open(signal) as f:
        # open the csv file
        reader = csv.reader(f)
        for row in reader:
            # read the rows 1 one by one
            t.append(float(row[0])) # leftmost column
            data1.append(float(row[1])) # second column

    num_samples = len(t)
    total_time = t[-1] - t[0]
    Fs = int(num_samples / total_time)   # sample rate


    dt = 1.0/Fs #time between sample rates

    # Ensure that t and data1 are numpy arrays for compatibility with FFT functions
    t = np.array(t)
    data1 = np.array(data1)
           
    # Ts = dt; # sampling interval
    # ts = np.arange(0,t[-1],Ts) # time vector
    y = data1 # the data to make the fft from
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


# call the function on all the files
fft('sigA.csv')
fft('sigB.csv')
fft('sigC.csv')
fft('sigD.csv')




