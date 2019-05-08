#!/bin/bash

#SBATCH -n 1
#SBATCH -c 20
#SBATCH -p shared
#SBATCH --qos=shared_short
#SBATCH -C has2s
#SBATCH -t 00:00:45

module load intel

n=(2 4 6 7)
nombre=("1" "1op" "3a" "4")
hilos=(1 2 4 8 16)

icc -O0 1.c -o 1
icc -O2 1.c -o 1op
## gcc 2.c -o 2 -lm
icc -O0  3a.c -o 3a
## gcc 3b.c -o 3b -lm -msse3
icc -O0 -qopenmp 4.c -o 4

for i in "${nombre[@]}"
do
	echo "********* Ejecutando ejercicio $i ***********"
	for j in "${n[@]}"
	do
		echo "****** Parametro $j ******"
		mkdir -p ./resultados/$i
		if [[ "$i" == "4" ]]; then
			for k in "${hilos[@]}"
			do
				echo "***** Hilos: $k *****"
				./$i $j ./resultados/$i/resultados_$k.csv $k
			done
		else
			./$i $j ./resultados/$i/resultados.csv
		fi
	done
done

exit
