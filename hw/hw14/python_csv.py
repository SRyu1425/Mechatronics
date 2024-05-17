import csv

t = [] # column 0
data1 = [] # column 1

with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data1.append(float(row[1])) # second column

#for i in range(len(t)):
    # print the data to verify it was read
    #print(str(t[i]) + ", " + str(data1[i]))

num_samples = len(t)
total_time = t[-1] - t[0]
sample_rate = num_samples / total_time
print(num_samples, sample_rate) #10K times per sec, 10K hz

