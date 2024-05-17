import csv
import matplotlib.pyplot as plt
import numpy as np
import csv

#pass in a csv file name
#return data and time lists
def parse_csv(signal):
    t = [] # column 0
    data = [] # column 1

    with open(signal) as f:
        # open the csv file
        reader = csv.reader(f)
        for row in reader:
            # read the rows 1 one by one
            t.append(float(row[0])) # leftmost column
            data.append(float(row[1])) # second column

    return t, data

#pass in data list and a&b parameters
#return a filtered list of data points and time
def IIR(data, a, b):
    new_average = [] #filtered data using a and b params
    new_average = [data[0]]  # Initialize with the first data point

    #iterate over data list, starting from the second data point
    for i in range(1, len(data)):
        new_average.append(a * new_average[i-1] + b * data[i])

    return new_average



#pass in old signal, time and filtered signal
#pass in a b params
#return plots
def fft(data, t, f_data, a, b):
    

    ############ original data ##########
    num_samples = len(t)
    total_time = t[-1] - t[0]
    Fs = int(num_samples / total_time)   # sample rate

    # Ensure that t and data are numpy arrays for compatibility with FFT functions
    t = np.array(t)
    data = np.array(data)
    
    y = data # the data to make the fft from
    n = len(y) # length of the signal
    k = np.arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range
    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]


    ############ filtered data ##########
    num_samples = len(t)
    total_time = t[-1] - t[0]
    Fs = int(num_samples / total_time)   # sample rate

    # Ensure that t and data are numpy arrays for compatibility with FFT functions
    t = np.array(t)
    f_data = np.array(f_data)
    
    fy = f_data # the data to make the fft from
    fn = len(fy) # length of the signal
    fk = np.arange(fn)
    fT = fn/Fs
    ffrq = fk/fT # two sides frequency range
    ffrq = ffrq[range(int(fn/2))] # one side frequency range
    fY = np.fft.fft(fy)/fn # fft computing and normalization
    fY = fY[range(int(fn/2))]


    #### plot results ####
    fig, (ax1, ax2) = plt.subplots(2, 1)
    plt.suptitle('IIR with Weights a = ' + str(a) +  ' and b = ' + str(b))  # Global title

    ax1.plot(t,fy,'r',  label = 'Filtered', zorder = 2) # plotting the filtered (red)
    ax1.plot(t,y,'k', label= 'Unfiltered', zorder = 1) # plotting the filtered (black)
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax1.legend()

    ax2.loglog(frq,abs(Y),'k', label= 'Unfiltered') # plotting the filtered (black)
    ax2.loglog(ffrq,abs(fY),'r', label = 'Filtered') # plotting the filtered (red)
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.legend()

    plt.tight_layout()
    plt.show()

##### main code #####
a, b = .999, .001
t, d = parse_csv('sigA.csv')
fd= IIR(d.copy(), a, b)
fft(d, t, fd, a, b)

a, b = .999, .001
t, d = parse_csv('sigB.csv')
fd= IIR(d.copy(), a, b)
fft(d, t, fd, a, b)

a, b = .1, .9
t, d = parse_csv('sigC.csv')
fd= IIR(d.copy(), a, b)
fft(d, t, fd, a, b)


a, b = .99, .01
t, d = parse_csv('sigD.csv')
fd= IIR(d.copy(), a, b)
fft(d, t, fd, a, b)