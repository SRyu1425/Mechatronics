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

#pass in data list and num of data points to avg
#return a filtered list of data points
def moving_avg(data, num_avg):
    filtered_data = [0] * (num_avg-1) #initialize first x indices with 0s

    while len(data) >= num_avg:
        #get the first x nums from the data and get avg
        avg = sum(data[:num_avg]) / float(num_avg)


        #add to the filtered lists
        filtered_data.append(avg)
        
        #delete the first data point in the data list
        del data[0]

    return filtered_data 

#pass in old data and filtered versions
#pass in avg_num 
#return plots
def fft(data, t, f_data, x):
    

    ############ original data ##########
    num_samples = len(t)
    total_time = t[-1] - t[0]
    Fs = int(num_samples / total_time)   # sample rate
    #print(Fs)

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
    plt.suptitle('MAF with ' + str(x) + ' data points averaged')  # Global title

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
x = 1000
t, d = parse_csv('sigA.csv')
fd = moving_avg(d.copy(), x)
fft(d, t, fd, x)

y = 1000
t, d = parse_csv('sigB.csv')
fd = moving_avg(d.copy(), y)
fft(d, t, fd, y)

z = 1
t, d = parse_csv('sigC.csv')
fd = moving_avg(d.copy(), z)
fft(d, t, fd, z)

a = 110
t, d = parse_csv('sigD.csv')
fd = moving_avg(d.copy(), a)
fft(d, t, fd,a)