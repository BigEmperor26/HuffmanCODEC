
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
        files = [join(run_folder,f) for f in listdir(run_folder) if isfile(join(run_folder, f)) and not f.endswith('err') and not f.endswith('.json')]
        #print(files)
        for file in files:
            indexes = file.split('_')
            # print(indexes)
            index = [int(i) for i in indexes if i.isdigit()]
            # print(index)
            rank = index[-2]
            thread = index[-1]
            try:
                thread_data[rank]
            except KeyError:
                thread_data[rank] = OrderedDict()
            try:
                thread_data[rank][thread]
            except KeyError:
                thread_data[rank][thread] = OrderedDict()
                for i in range(rank):
                    data = OrderedDict()
                    # data['Frequencies']= {'CPU': 0.0, 'Wall': 0.0}
                    data['Tree']= {'CPU': 0.0, 'Wall': 0.0}
                    data['Decode']= {'CPU': 0.0, 'Wall': 0.0}
                    data['Flush']= {'CPU': 0.0, 'Wall': 0.0}
                    data['Overall']= {'CPU': 0.0, 'Wall': 0.0}
                    thread_data[rank][thread][i] = data
            
            with open (file, "r") as myfile:
                
                for line in myfile:
                    # if 'CPU' in line and 'frequencies' in line:
                    #     processor = int(line.split(' ')[0])
                    #     thread_data[rank][thread][processor]['Frequencies']['CPU'] += float(line.split(' ')[-1])
                    # elif 'Wall' in line and 'frequencies' in line:
                    #     processor = int(line.split(' ')[0])
                    #     thread_data[rank][thread][processor]['Frequencies']['Wall'] += float(line.split(' ')[-1])
                    if 'CPU' in line and 'tree' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Tree']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'tree' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Tree']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'decode' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Decode']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'decode' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Decode']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'flush' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Flush']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'flush' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Flush']['Wall'] += float(line.split(' ')[-1])
                    elif 'CPU' in line and 'Overall' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Overall']['CPU'] += float(line.split(' ')[-1])
                    elif 'Wall' in line and 'Overall' in line:
                        processor = int(line.split(' ')[0])
                        thread_data[rank][thread][processor]['Overall']['Wall'] += float(line.split(' ')[-1])
    # divide by number of runs
    for rank in thread_data:
        for thread in thread_data[rank]:
            for processor in thread_data[rank][thread]:
                for key in thread_data[rank][thread][processor]:
                    for time in thread_data[rank][thread][processor][key]:
                        thread_data[rank][thread][processor][key][time] /= len(run_folders)


    with open(join(file_folder,'decode_averaged_results.json'), 'w') as outfile:
        json.dump(thread_data, outfile,indent=4)

