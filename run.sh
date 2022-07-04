#!/bin/bash

# Maximum number of the machine's cores.
max_cores=64

# Cores / Threads to utilize. Use spaces to define a set of different thread numbers to benchmark.
# cores='1 2 4 8 16 32 64'
cores='64'
# cores='32'
# cores='16'
# centers='4 16 64 256 1024'
# centers='64'
centers='4'

# Use hyperthreading.
hyperthreading=0
# hyperthreading=1

if [[ $hyperthreading == 1 ]]; then
	max_cores=$((2*max_cores))
	cores="$cores $max_cores"
fi

# GOMP_CPU_AFFINITY pins the threads to specific cpus, even when assigning more cores than threads.
# e.g. with 'GOMP_CPU_AFFINITY=0,1,2,3' and 2 threads, the threads are pinned: t0->core0 and t1->core1.
if [[ $hyperthreading == 1 ]]; then
	affinity=''
	for ((i=0;i<max_cores/2;i++)); do
		affinity="$affinity,$i,$((i,max_cores/2+i))"
	done
	affinity="${affinity:1}"
	export GOMP_CPU_AFFINITY="$affinity"
	printf "cpu affinities: %s\n" "$affinity"
else
	export GOMP_CPU_AFFINITY="0-$((max_cores-1))"
fi

# Encourages idle threads to spin rather than sleep.
# export OMP_WAIT_POLICY='active'
# Don't let the runtime deliver fewer threads than those we asked for.
# export OMP_DYNAMIC='false'

input_root=./datasets/
input_root2=./artificial_datasets/
input_files=(
	# "$input_root"/color17695.txt
	# "$input_root"/edge17695.txt
	# "$input_root"/texture17695.txt
	# "$input_root"/RETRO_edg_anthro_ACIDS_2000.0.5x0.5.txt
	# "$input_root"/Frogs_MFCCs.txt
	# "$input_root"/winequality.txt

	"$input_root2"/synthetic_1_8192_32.txt
	# "$input_root2"/synthetic_1_16384_16.txt
	# "$input_root2"/synthetic_1_32768_8.txt
	# "$input_root2"/synthetic_1_65536_4.txt
	# "$input_root2"/synthetic_1_131072_2.txt
	"$input_root2"/synthetic_8_65536_32.txt
	# "$input_root2"/synthetic_8_131072_16.txt
	# "$input_root2"/synthetic_8_262144_8.txt
	# "$input_root2"/synthetic_8_524288_4.txt
	# "$input_root2"/synthetic_8_1048576_2.txt
	"$input_root2"/synthetic_16_131072_32.txt
	# "$input_root2"/synthetic_16_262144_16.txt
	# "$input_root2"/synthetic_16_524288_8.txt
	# "$input_root2"/synthetic_16_1048576_4.txt
	# "$input_root2"/synthetic_16_2097152_2.txt
	"$input_root2"/synthetic_64_524288_32.txt
	# "$input_root2"/synthetic_64_1048576_16.txt
	# "$input_root2"/synthetic_64_2097152_8.txt
	# "$input_root2"/synthetic_64_4194304_4.txt
	# "$input_root2"/synthetic_64_8388608_2.txt
	"$input_root2"/synthetic_256_2097152_32.txt
	# "$input_root2"/synthetic_256_4194304_16.txt
	# "$input_root2"/synthetic_256_8388608_8.txt
	# "$input_root2"/synthetic_256_16777216_4.txt
	# "$input_root2"/synthetic_256_33554432_2.txt
	"$input_root2"/synthetic_1024_8388608_32.txt
	# "$input_root2"/synthetic_1024_16777216_16.txt
	# "$input_root2"/synthetic_1024_33554432_8.txt
	# "$input_root2"/synthetic_1024_67108864_4.txt
	# "$input_root2"/synthetic_1024_134217728_2.txt
)

progs=(
	# "sequential"
	# "omp(atomic)"
	"omp(array-reduction)"
)

bench()
{
	declare args=("$@")
	declare prog="${args[0]}"
	declare prog_args=("${args[@]:1}")
	declare c
	declare t

	for c in $centers; do
		if [ "$prog" = "sequential" ]; then
			# sequential
			./seq_main -i "${prog_args[@]}" -c $c -o
			mv "${prog_args[@]}".cluster_centres "${prog_args[@]}".n$c.sequential.cluster_centres
			mv "${prog_args[@]}".membership "${prog_args[@]}".n$c.sequential.membership
		
		else

			for t in $cores; do
				export OMP_NUM_THREADS="$t"

				if [ "$prog" = "omp(atomic)" ]; then
					# omp_atomic
					./omp_main -a -i "${prog_args[@]}" -p $t -c $c -o
					mv "${prog_args[@]}".cluster_centres "${prog_args[@]}".n$c.omp_atomic_t$t.cluster_centres
					mv "${prog_args[@]}".membership "${prog_args[@]}".n$c.omp_atomic_t$t.membership
				fi

				if [ "$prog" = "omp(array-reduction)" ]; then
					# omp_array_reduction
					./omp_main -i "${prog_args[@]}" -p $t -c $c -o
					mv "${prog_args[@]}".cluster_centres "${prog_args[@]}".n$c.omp_array_reduction_t$t.cluster_centres
					mv "${prog_args[@]}".membership "${prog_args[@]}".n$c.omp_array_reduction_t$t.membership
				fi
			done

		fi
	done
}

prog_args=("${input_files[@]}")

for a in "${prog_args[@]}"; do
	echo "File: $a"
	for p in "${progs[@]}"; do
		echo "program: $p"
		bench $p "$a"
		echo '--------------------------------------------'
	done
	echo '----------------------------------------------------------------------------------------'

done
