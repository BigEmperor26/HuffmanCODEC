import matplotlib.pyplot as plt
import json
import re
import sys
from collections import OrderedDict
from os import listdir
from os.path import isfile, isdir, join

# for each file in a folder
# open the encded average results
# plot on a graph the results

path = sys.argv[1]

file_folders = [join(path,f) for f in listdir(path) if isdir(join(path, f))]
sorted_files = sorted(file_folders, key=lambda x: int(re.findall(r'\d+',x)[0]) if 'mb' in x else int(re.findall(r'\d+',x)[0])*1024)
print(sorted_files)
results_matrix_overall_Wall = []
for file_folder in file_folders:
    json_file = [join(file_folder,f) for f in listdir(file_folder) if isfile(join(file_folder, f)) and f.endswith('json')]
    thread_data = json.load(open(json_file[0]))
    temp_list = []
    for thread in thread_data:
        temp_list.append(thread_data[thread]['Overall']['Wall'])
    results_matrix_overall_Wall = temp_list


# plot the results
for i in range(len(results_matrix_overall_Wall)):
    plt.plot(results_matrix_overall_Wall[i], label='File size '+str(i+1))
# plt.plot(results_matrix_overall_Wall)