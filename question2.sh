line=$(cat temp.txt | cut -d$'\t' -f 2 | grep -n -w "1252" | cut -d ":" -f 1)
line=$((line + 1000000))
sed -n "$line"p temp.txt
