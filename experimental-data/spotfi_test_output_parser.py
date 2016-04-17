#!/usr/bin/python3

import sys

# Leading text before data of interest
DATA_FILE_HEADER = 'Running on data file:'
CLUSTER_NUMBER_HEADER = 'Cluster Properties for cluster '
NUM_CLUSTER_POINTS_HEADER = 'Num Cluster Points: '
AOA_VARIANCE_HEADER = 'AoA Variance: '
TOF_VARIANCE_HEADER = 'ToF Variance: '
AOA_MEAN_HEADER = 'AoA Mean '
TOF_MEAN_HEADER = 'ToF Mean: '
END_CUR_CLUSTER_HEADER = ' has formatting: '
END_CUR_DATA_FILE_HEADER = 'output_top_aoas'

CLUSTER_POINTS_KEY = 'num_cluster_points'
AOA_VARIANCE_KEY = 'aoa_variance'
TOF_VARIANCE_KEY = 'tof_variance'
AOA_MEAN_KEY = 'aoa_mean'
TOF_MEAN_KEY = 'tof_mean'

def parse_spotfi_test_output(file_name):
    data_file_data = dict()
    clusters_data = list()
    cur_cluster_data = dict()

    cur_data_file = None
    cur_num_cluster_points = None
    cur_aoa_variance = None
    cur_tof_variance = None
    cur_aoa_mean = None
    cur_tof_mean = None
    with open(file_name, 'r') as spotfi_test_output_file:
        for line in spotfi_test_output_file:
            line = line.strip()
            if line.find(DATA_FILE_HEADER) != -1:
                cur_data_file = line[len(DATA_FILE_HEADER):].strip()
            elif line.find(NUM_CLUSTER_POINTS_HEADER) != -1:
                cur_num_cluster_points = line[len(NUM_CLUSTER_POINTS_HEADER):].strip()
                cur_cluster_data[CLUSTER_POINTS_KEY] = cur_num_cluster_points
            elif line.find(AOA_VARIANCE_HEADER) !=- 1:
                cur_aoa_variance = line[len(AOA_VARIANCE_HEADER):].strip()
                cur_cluster_data[AOA_VARIANCE_KEY] = cur_aoa_variance
            elif line.find(TOF_VARIANCE_HEADER) != -1:
                cur_tof_variance = line[len(TOF_VARIANCE_HEADER):].strip()
                cur_cluster_data[TOF_VARIANCE_KEY] = cur_tof_variance
            elif line.find(AOA_MEAN_HEADER) != -1:
                cur_aoa_mean = line[len(AOA_MEAN_HEADER):].strip()
                cur_cluster_data[AOA_MEAN_KEY] = cur_aoa_mean
            elif line.find(TOF_MEAN_HEADER) != -1:
                cur_tof_mean = line[len(TOF_MEAN_HEADER):].strip()
                cur_cluster_data[TOF_MEAN_KEY] = cur_tof_mean
            elif line.find(END_CUR_CLUSTER_HEADER) != -1:
                clusters_data.append(cur_cluster_data.copy())
                cur_cluster_data = dict()
            elif line.find(END_CUR_DATA_FILE_HEADER) != -1:
                data_file_data[cur_data_file] = clusters_data
                clusters_data = list()
    return data_file_data

if __name__ == '__main__':
    data_file_data = parse_spotfi_test_output('test-localization-data-output.txt')
    file_names = {
            'los-test-heater.dat': -45,
            'los-test-desk-left.dat': -38,
            'los-test-desk-right.dat': -10,
            'los-test-printer.dat': -2,
            'los-test-nearby-long-bookshelf.dat': -11,
            'los-test-tall-bookshelf.dat': 14,
            'los-test-jennys-table.dat': 58
    }
    print('X = [')
    for key in data_file_data:
        #print(key)
        #print(data_file_data[key])
        for cluster in data_file_data[key]:
            sys.stdout.write(cluster[CLUSTER_POINTS_KEY])
            sys.stdout.write(', ')
            sys.stdout.write(cluster[AOA_VARIANCE_KEY])
            sys.stdout.write(', ')
            sys.stdout.write(cluster[TOF_VARIANCE_KEY])
            sys.stdout.write(', ')
            sys.stdout.write(cluster[TOF_MEAN_KEY])
            sys.stdout.write(';\n')
    print(']')

    print('Y = [')
    for key in data_file_data:
        #print(key)
        for cluster in data_file_data[key]:
            sys.stdout.write(str(round(float(cluster[AOA_MEAN_KEY]))))
            sys.stdout.write(', ')
            sys.stdout.write(str(file_names[key]))
            sys.stdout.write(';\n')
    print(']')
