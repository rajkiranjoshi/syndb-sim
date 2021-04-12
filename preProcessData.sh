filesInDir=$(ls ./dump*switch*.txt)

for fileName in $filesInDir
do
   echo "Sorting $fileName"
   sort -n -r -k1 $fileName -o $fileName
done

filesInDir=$(ls ./dump*sourceDestination.txt)
for fileName in $filesInDir
do
   echo "Sorting $fileName"
   sort -n -k1 $fileName -o $fileName -T /home/nishant
done