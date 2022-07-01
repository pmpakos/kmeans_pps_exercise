import numpy as np
import random
import time

def create_dataset(dataset_target_size, numCoords):    
    # dataset target = numObjs * numCoords * sizeof(float)
    # numObjs = dataset_target_size (in MB) / (numCoords*4 bytes per float) * (1024*1024)
    numObjs = int(np.floor((dataset_target_size*1024*1024)/(numCoords*4)))
    
    filename = 'synthetic_' + str(dataset_target_size) + '_' + str(numObjs) + '_' + str(numCoords) + '.txt'
    f=open(filename, 'w')

    start = time.time()
    for r in range(numObjs):
        curr_obj = ''

        for c in range(numCoords):
            rand_num = random.random()
            curr_obj += str(round(rand_num,4)) + ' '
        curr_obj += '\n'
        f.write(curr_obj)
    f.close()
    end = time.time()

    print(filename, 'in', end-start, 'seconds')

# numCoords_list = [2, 4, 8, 16, 32]
numCoords_list = [16, 32]
# dataset_target_size_list = [1, 8, 16, 64, 256, 1024]
dataset_target_size_list = [1024]

for dataset_target_size in dataset_target_size_list:
    for numCoords in numCoords_list:
        create_dataset(dataset_target_size, numCoords)
