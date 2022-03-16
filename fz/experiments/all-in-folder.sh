echo "first is timeout $1 , 2rd is config (toy value to run all)  $2 , and 3rth argument is the seed $3 and 4rth is fzn FOLDER $4"

k=$(ls -1 $4/*fzn)

echo " START Folder"
echo $k

echo " .. "

for VARIABLE in $k
do
	echo $VARIABLE
	#The recommended configurtation here is toy
	./seed-run.sh $1 $2 $3  $VARIABLE 
done

#./mistral-fz --recommended 22 --time_limit 3 --seed 55  --print_sta benchmark/i1-o1.fzn
