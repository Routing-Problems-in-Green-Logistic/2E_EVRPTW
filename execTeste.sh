data=$(date)
find="-03"
replace=""
data=${data//$find/$replace}

find=" "
replace="_"
data=${data//$find/$replace}

size=${#data}
size=$((size - 4))
data=${data:0:$size}
dir="TESTES/testes_"$data
#echo $dir

mkdir $dir
cp src/cmake-build-debug/run $dir

echo $data + '\n'

python3 testesNew.py $dir

#echo $size
