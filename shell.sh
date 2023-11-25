contor=0
if test $# -eq 1
then
	while IFS= read -r line
	do
		if grep -Eq '^[A-Z][[A-Za-z0-9 ,!.?]*[.!?]$' <<< "$line" && ! grep -Eq ',si' <<< "$line"
		then
		contor=`expr $contor + 1`;

		fi
	done
	echo $contor
else
	echo "Usage "
fi
