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
    
    python3 testes00.py $dir 1 | tee -a $dir2
    
else
    find=" "
    replace="_"
    data=${data//$find/$replace}

    size=${#data}
    size=$((size - 1))
    data=${data:0:$size}


    dir="TESTES/testes_"$data
    dir2=$dir"/log.txt"
    #echo $dir

    mkdir $dir
    cp src/cmake-build-debug/run $dir
    #cp parametrosIg.txt $dir
   
    echo $data
    
    #echo "Parametros IG:"
    #cat $dir/parametrosIg.txt
    
    python3 testes100.py $dir | tee $dir2
fi

#echo $size
