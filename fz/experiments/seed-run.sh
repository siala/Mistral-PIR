echo "first is timeout $1 , 2rd is config (toy value to run all)  $2 , and 3rth argument is the seed $3 and 4rth is fzn $4"

k=$(echo "$4" |  sed  's|'/'|''|g' )

#echo $k

#echo results/$1-$2-$k.txt


./mistral-fz --time_limit $1 --recommended $2  --seed $3  $4 > output-results/$1-$2-$3-$k.txt 
