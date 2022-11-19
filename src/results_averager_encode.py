
# argv[1] = path to the directory containing the results
# argv[2] = path to the directory where the averaged results will be saved

import sys
from collections import OrderedDict
from os import listdir
from os.path import isfile, isdir, join
import json

path = sys.argv[1]

file_folders = [join(path,f) for f in listdir(path) if isdir(join(path, f))]
#print(file_folders)
for file_folder in file_folders:
    thread_data = {}
    run_folders = [join(file_folder,f) for f in listdir(file_folder) if isdir(join(file_folder, f)) and f.startswith('run_')]
    #print(run_folders)
    for run_folder in run_folders:
        files = [join(run_folder,f) for f in listdir(run_folder) if isfile(join(run_folder, f)) and not f.endswith('err')]
        #print(files)
        for file in files:
            indexes = file.split('_')
            index = [int(i) for i in indexes if i.isdigit()]
            thread = index[0]
            try:
                thread_data[thread]
            except KeyError:
                data = OrderedDict()
                data['Frequencies']= {'CPU': 0.0, 'Wall': 0.0}
                data['Tree']= {'CPU': 0.0, 'Wall': 0.0}
                data['Encode']= {'CPU': 0.0, 'Wall': 0.0}
                data['Flush']= {'CPU': 0.0, 'Wall': 0.0}
                data['Overall']= {'CPU': 0.0, 'Wall': 0.0}
                thread_data[index[0]]=data
            with open (file, "r") as myfile:
                for line in myfile:
                    if 'CPU' in line and 'frequencies' in line:
                        thread_data[thread]['Frequencies']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'frequencies' in line:
                        thread_data[thread]['Frequencies']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'tree' in line:
                        thread_data[thread]['Tree']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'tree' in line:
                        thread_data[thread]['Tree']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'encode' in line:
                        thread_data[thread]['Encode']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'encode' in line:
                        thread_data[thread]['Encode']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'flush' in line:
                        thread_data[thread]['Flush']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'flush' in line:
                        thread_data[thread]['Flush']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'Overall' in line:
                        thread_data[thread]['Overall']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'Overall' in line:
                        thread_data[thread]['Overall']['Wall'] += float(line.split(' ')[-1])

    for thread in thread_data:
        for key in thread_data[thread]:
            for time in thread_data[thread][key]:
                thread_data[thread][key][time] /= len(run_folders)


    with open(join(file_folder,'encode_averaged_results.json'), 'w') as outfile:
        json.dump(thread_data, outfile,indent=4)

