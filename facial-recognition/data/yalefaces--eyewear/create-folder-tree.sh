#/bin/bash

jpeg=".jpg"

for file in *
do
	echo "operating on $file"
	if [[ $file == .* ]] || [[ $file == *\.sh ]] || [[ $file == "Readme.txt" ]]
	then
		echo "caught $file with continue..."
		continue
    fi
	convert $file $file$jpeg
	rm $file
	file=$file$jpeg
    dot_index=`expr index "$file" '\.'`
	dot_index=`expr $dot_index - 1`
	folder_name=`expr substr $file 1 $dot_index`
	if ! [[ -e $folder_name ]]
	then 
		mkdir $folder_name
	fi
	mv $file $folder_name	
done
