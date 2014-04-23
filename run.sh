#$1: number of times to execute the tests. Default is 1.
ulimit -c unlimited
rm "core" -f
i="0"
max="1"
if [[ $1 ]];
then
max=$1
fi
make
until [[ $i == $max || -e "core"  ]];
do
./build/test/msp/test/msp-test 
i=$[$i+1]
done
if [[ -e "core" ]];
then
echo "FAILED at run nr." $i
else
echo "Succesfully ran" $max "times"
fi

