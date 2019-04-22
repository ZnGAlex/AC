#!/bin/bash

n=(2 4 6 7)
nombre=("1" "2" "3" "4")

gcc 1.c -o 1 -lm
gcc 2.c -o 2 -lm
gcc 3.c -o 3 -lm -msse3
gcc 4.c -o 4 -lm -fopenmp -msse3

for i in "${nombre[@]}"
do
	echo "********* Ejecutando ejercicio $i ***********"
	for j in "${n[@]}"
	do
		echo "Parametro $j"
		./$i $j resultados$i.csv
	done
done

exit