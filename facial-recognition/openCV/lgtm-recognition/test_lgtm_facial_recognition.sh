#!/bin/bash

./lgtm_facial_recognition haarcascades/haarcascade_frontalface_default.xml 1 yalefaces-only-9.csv 9 -20 10
exit $?
