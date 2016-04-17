#/bin/bash

my_dir=/home/ugrads/ugrads1/e/egaebel/thesis-project/lgtm/csi-code/test-code

cd $my_dir
nohup matlab -nojvm nodisplay -nosplash -r "sampling_value_tests($1), exit" > experiment-output--chunk-"$1".out &
sleep 5
exit
