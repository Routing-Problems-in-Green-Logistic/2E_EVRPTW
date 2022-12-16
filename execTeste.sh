data=$(date)
find="-03"
replace=""
data=${data//$find/$replace}

n=$#

mkdir "TESTES"

if (($n==1))
then

    data=$1
    dir="TESTES/testes_"$data
    dir2=$dir"/log.txt"
    echo "Continuacao teste: "$data
    
    python3 testesNew.py $dir 1 | tee -a $dir2
    
else
    find=" "
    replace="_"
    data=${data//$find/$replace}

    size=${#data}
    size=$((size - 4))
    data=${data:0:$size}


    dir="TESTES/testes_"$data
    dir2=$dir"/log.txt"
    #echo $dir

    mkdir $dir
    cp src/cmake-build-debug/run $dir

    echo $data
    
    python3 testesNew.py $dir | tee $dir2
fi

#echo $size
