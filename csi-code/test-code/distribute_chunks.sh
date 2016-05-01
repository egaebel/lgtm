#!/bin/bash

chunk_of_cluster_number=$1
my_dir=/home/ugrads/ugrads1/e/egaebel/thesis-project/lgtm/csi-code/test-code

kill_all () {
    ssh ash -o ConnectTimeout=10 -t "pkill MATLAB"
	ssh beech -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh birch -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh boxelder -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh cedar -o ConnectTimeout=10 -t "pkill MATLAB"

    ssh chinkapin -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh dogwood -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh gum -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh hackberry -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh hickory -o ConnectTimeout=10 -t "pkill MATLAB"

    ssh hornbeam -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh linden -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh locust -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh maple -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh mulberry -o ConnectTimeout=10 -t "pkill MATLAB"

    ssh pawpaw -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh pine -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh sassafras -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh spruce -o ConnectTimeout=10 -t "pkill MATLAB"
    ssh sumac -o ConnectTimeout=10 -t "pkill MATLAB"
}

if [[ $chunk_of_cluster_number == 0 ]]; then
    kill_all
fi

if [[ $chunk_of_cluster_number == 1 ]]; then
    ssh ash -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 1"
    ssh beech -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 2"
    ssh birch -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 3"
    ssh boxelder -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 4"
    ssh cedar -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 5"
    echo "Chunk 1 jobs submitted to ash, beech, birch, boxelder, and cedar"
fi

if [[ $chunk_of_cluster_number == 2 ]]; then
    ssh chinkapin -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 6"
    ssh dogwood -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 7"
    ssh gum -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 8"
    ssh hackberry -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 9"
    ssh hickory -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 10"
    echo "Chunk 2 jobs submitted to chinkapin, dogwood, gum, hackberry, and hickory"
fi

if [[ $chunk_of_cluster_number == 3 ]]; then
    ssh hornbeam -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 11"
    ssh linden -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 12"
    ssh locust -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 13"
    ssh maple -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 14"
    ssh mulberry -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 15"
    echo "Chunk 3 jobs submitted to hornbeam, linden, locust, maple, and mulberry"
fi

if [[ $chunk_of_cluster_number == 4 ]]; then
    ssh pawpaw -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 16"
    ssh pine -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 17"
    ssh sassafras -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 18"
    ssh spruce -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 19"
    ssh sumac -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 20"
    echo "Chunk 4 jobs submitted to pawpaw, pine, sassafras, spruce, and sumac"
fi

if [[ $chunk_of_cluster_number == 5 ]]; then
    ssh pawpaw -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 21"
    ssh pine -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 22"
    ssh sassafras -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 23"
    ssh spruce -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 24"
    ssh sumac -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 25"
    echo "Chunk 5 jobs submitted to pawpaw, pine, sassafras, spruce, and sumac"
fi

if [[ $chunk_of_cluster_number == 6 ]]; then
    ssh hornbeam -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 26"
    ssh linden -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 27"
    ssh locust -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 28"
    ssh maple -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 29"
    ssh mulberry -t "pkill MATLAB; $my_dir/run_matlab_data_proc.sh 30"
    echo "Chunk 6 jobs submitted to hornbeam, linden, locust, maple, and mulberry"
fi
