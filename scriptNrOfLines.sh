cnt=0
if test $# -eq 1
then
	cnt=$( echo "$1" | wc -l)
	echo "$cnt"
else
	echo "Usage ./scriptNrOfLines.sh <text>"
fi
