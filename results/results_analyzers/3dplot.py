import matplotlib.pyplot as plt
import json
import re
import sys
from collections import OrderedDict
from os import listdir
from os.path import isfile, isdir, join
import pandas as pd
import numpy as np
from matplotlib import cm
# for each file in a folder
# open the encded average results
# plot on a graph the results

#path = sys.argv[1]
# path = '../results/synthetic_encode_barrier'
path = 'results/synthetic_encode_barrier'
plt.rcParams.update({'font.size': 10})
def json_to_pd(path):
    file_folders = [join(path,f) for f in listdir(path) if isdir(join(path, f))]
    sorted_files = sorted(file_folders, key=lambda x: int(re.findall(r'\d+',x)[0]) if 'mb' in x else int(re.findall(r'\d+',x)[0])*1024)
    print(sorted_files)
    results_matrix_encode_Wall_barrier = []
    results_matrix_overall_Wall_barrier = []
    threads = []
    file_sizes = [re.findall(r'\d+',x)[0] + ' MiB' if 'mb' in x else re.findall(r'\d+',x)[0] + ' GiB' for x in sorted_files]
    print(file_sizes)
    for file_folder in sorted_files:
        json_file = [join(file_folder,f) for f in listdir(file_folder) if isfile(join(file_folder, f)) and f.endswith('json')]
        thread_data = json.load(open(json_file[0]))
        temp_list = []
        temp_all_list = []
        for thread in sorted(thread_data,key= lambda x: int(x)):
            if int(thread) not in threads:
                threads.append(int(thread))
            #time = thread_data[thread]['Frequencies']['Wall']
            #time += thread_data[thread]['Tree']['Wall']
            time = thread_data[thread]['Encode']['Wall']
            temp_list.append(time)
            temp_all_list.append(thread_data[thread]['Overall']['Wall'])
        results_matrix_encode_Wall_barrier.append(temp_list)
        results_matrix_overall_Wall_barrier.append(temp_all_list)
    
    return np.matrix(results_matrix_encode_Wall_barrier), np.matrix(results_matrix_overall_Wall_barrier), threads, file_sizes

def tex_bolder(tex_input,minimum=True):
    st = tex_input
    splitted = st.split('\n')
    for j,line in enumerate(splitted):
        if j>3 and j<len(splitted)-2:
            if minimum:
                current_max=float('inf')
            else:
                current_max=float('-inf')
            for i,item in enumerate(line.split('&')):
                if 'MiB' not in item and 'GiB' not in item:
                    itemed = item.split(' ')
                    for it in itemed:
                        try:
                            if minimum:
                                current_max = min(current_max, float(it))
                            else:
                                current_max = max(current_max, float(it))
                        except:
                            pass
            splitted[j] = line.replace(str(current_max), '\\textbf{'+str(current_max)+'}')
    return '\n'.join(splitted)

encode_wall_barrier, overall_wall_barrier, threads, file_sizes = json_to_pd(path)
# print(threads)
# df_encode = pd.DataFrame(encode_wall_barrier, columns=threads, index=file_sizes)
# print(tex_bolder(df_encode.to_latex(index=True,float_format="%.3f"),minimum=True))

# df_overall = pd.DataFrame(overall_wall_barrier, columns=threads, index=file_sizes)
# print(tex_bolder(df_overall.to_latex(index=True,float_format="%.3f"),minimum=True))

# # effiecincies
# matrix_enc_eff = 1/((encode_wall_barrier/encode_wall_barrier[:,0]))/np.matrix(threads)
# df_overall_efficiency = pd.DataFrame(matrix_enc_eff, columns=threads, index=file_sizes)
# # display(df_overall_efficiency)
# print(df_overall_efficiency.to_latex(index=True,float_format="%.3f"))

# matrix_overall_eff = 1/((overall_wall_barrier/overall_wall_barrier[:,0]))/np.matrix(threads)
# df_encode_efficiency = pd.DataFrame(matrix_overall_eff, columns=threads, index=file_sizes)
# print(df_encode_efficiency.to_latex(index=True,float_format="%.3f"))

matrix_overall_eff = 1/(encode_wall_barrier/encode_wall_barrier[:,0])
df_encode_efficiency = pd.DataFrame(matrix_overall_eff, columns=threads, index=file_sizes)
#plt.figure(figsize=(12, 5))
fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
print(file_sizes)
file_sizes_mb = [int(re.findall(r'\d+',x)[0]) if 'MiB' in x else int(re.findall(r'\d+',x)[0])*1024 for x in file_sizes]
x,y = np.meshgrid(np.asarray([1,2,3,4,5,6,7,8,9,10,11,12,13],dtype=float), np.asarray([1,2,3,4,5,6,7,8,9],dtype=float))
z = matrix_overall_eff
print(x.shape)
print(y.shape)
print(z.shape)

surf = ax.plot_surface(x,y,z, cmap=cm.coolwarm,
                       linewidth=0, antialiased=False)
# plt.ylim(1,10*1024)
# plt.xlim(1,64)
plt.ylabel('file sizes')
plt.xlabel('threads')
ax.set_zlabel('speedup')
# x_ticks = range(threads)
plt.xticks(np.asarray([1,2,3,4,5,6,7,8,9,10,11,12,13],dtype=float), threads)
print(file_sizes_mb)
plt.yticks(np.asarray([1,2,3,4,5,6,7,8,9],dtype=float), file_sizes_mb)
# plot the results
# print(threads,matrix_overall_eff[-1].T)
# for i,size in zip(range(len(matrix_overall_eff)),file_sizes):
#     # if size == '500 MiB':
#     #     plt.plot(threads,matrix_overall_eff[i].T, label='Ours, file size '+size,marker='o')
#     plt.plot(threads,matrix_overall_eff[i].T, label='File size '+size,marker='o')
# # enemy = np.asarray([21.667,11.522,5.868,4.018,2.892,2.633 ,2.344,1.733 ,1.654,1.769 ,1.801,1.792 ,1.65])
# # enemy = 1/(enemy/21.667)
# # plt.plot(threads,enemy, label='Online 7, file size 500 MiB',marker='o')
# # plt.plot(threads,threads,linestyle='dashed', label='Ideal',color='grey')
# plt.plot(threads,threads,linestyle='dashed', label='Ideal',color='grey')
# plt.title('Encode Speedup')
# plt.plot(results_matrix_overall_Wall)
plt.legend(loc='upper left',fontsize=14,ncol=2)
plt.show()