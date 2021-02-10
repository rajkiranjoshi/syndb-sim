filesInDir=$(ls ./dump*switch*.txt)

for fileName in $filesInDir
do
   echo "Sorting $fileName"
   sort -n -k1 $fileName -o $fileName
done