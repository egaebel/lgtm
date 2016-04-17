#!/bin/bash

chunk_of_cluster_number=$1
my_dir=/home/ugrads/ugrads1/e/egaebel/thesis-project/lgtm/csi-code/test-code

if [[ $chunk_of_cluster_number == 1 ]]; then
    ssh ash -t "$my_dir/run_matlab_data_proc.sh 1"
    ssh beech -t "$my_dir/run_matlab_data_proc.sh 2"
    ssh birch -t "$my_dir/run_matlab_data_proc.sh 3"
    ssh boxelder -t "$my_dir/run_matlab_data_proc.sh 4"
    ssh cedar -t "$my_dir/run_matlab_data_proc.sh 5"
    echo "Chunk 1 jobs submitted to ash, beech, birch, boxelder, and cedar"
fi

if [[ $chunk_of_cluster_number == 2 ]]; then
    ssh chinkapin -t "$my_dir/run_matlab_data_proc.sh 6"
    ssh dogwood -t "$my_dir/run_matlab_data_proc.sh 7"
    ssh gum -t "$my_dir/run_matlab_data_proc.sh 8"
    ssh hackberry -t "$my_dir/run_matlab_data_proc.sh 9"
    ssh hickory -t "$my_dir/run_matlab_data_proc.sh 10"
    echo "Chunk 2 jobs submitted to chinkapin, dogwood, gum, hackberry, and hickory"
fi

if [[ $chunk_of_cluster_number == 3 ]]; then
    ssh hornbeam -t "$my_dir/run_matlab_data_proc.sh 11"
    ssh linden -t "$my_dir/run_matlab_data_proc.sh 12"
    ssh locust -t "$my_dir/run_matlab_data_proc.sh 13"
    ssh maple -t "$my_dir/run_matlab_data_proc.sh 14"
    ssh mulberry -t "$my_dir/run_matlab_data_proc.sh 15"
    echo "Chunk 3 jobs submitted to hornbeam, linden, locust, maple, and mulberry"
fi

if [[ $chunk_of_cluster_number == 4 ]]; then
    ssh pawpaw -t "$my_dir/run_matlab_data_proc.sh 16"
    ssh pine -t "$my_dir/run_matlab_data_proc.sh 17"
    ssh sassafras -t "$my_dir/run_matlab_data_proc.sh 18"
    ssh spruce -t "$my_dir/run_matlab_data_proc.sh 19"
    ssh sumac -t "$my_dir/run_matlab_data_proc.sh 20"
    echo "Chunk 4 jobs submitted to pawpaw, pine, sassafras, spruce, and sumac"
fi

