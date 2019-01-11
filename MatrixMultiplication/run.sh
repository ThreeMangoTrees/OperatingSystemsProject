#! /bin/bash
cc generateMat.c -o generateMat && ./generateMat
echo -e "\n"
echo -e "\tMatrix computation using Single thread Begins at : $(date +"%T")"
start=$(date +"%s")
cc prog1.c -o prog1 && ./prog1
end=$(date +"%s")
echo -e "\tSingle thread computation completed at : $(date +"%T")"
diff=$(($end-$start))
echo -e "\tTime taken by single threaded program is $diff seconds\n"

echo -e "\tMatrix computation using Multi thread Begins at : $(date +"%T")"
start=$(date +"%s")
cc prog2.c -pthread -w -o prog2 && ./prog2
end=$(date +"%s")
echo -e "\tMulti thread program ended at : $(date +"%T")"
diff=$(($end-$start))
echo -e "\tTime taken by multithreaded program is $diff seconds"
