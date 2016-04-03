#/bin/bash

matlab -nojvm nodisplay -nosplash -r "sampling_value_tests($1), exit" > experiment-output--chunk-"$1".out
