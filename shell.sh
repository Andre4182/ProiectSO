contor=0
if test $# -eq 1
then
	if grep -Eq '[A-Za-z0-9]' <<< "$1"
	then 
		while IFS= read -r line
		do
			if grep -Eq '^[A-Z][A-Za-z0-9 ,!.?]*[.!?]$' <<< "$line" && ! grep -Eq ',si' <<< "$line" && grep -Eq "$1" <<< "$line"
			then
				contor=$(expr "$contor" + 1)

			fi
		done
		echo $contor
	else 
		echo "The given argument should be an alphanumeric character!";
	fi
	
else
	echo "Usage ./shell.sh <character>"
fi
