# kmeans_pps_exercise

___ original version [here](http://users.ece.northwestern.edu/~wkliao/Kmeans/index.html) ___

### Compile
Run `make` and build `seq_main`, `omp_main`. Only openmp version is important for now (`mpi` will be checked later).

### Run
See `run.sh` file. Variables are : 

* number of threads (pinning by default enabled, no hyperthreading)
* number of cluster centers
* version to run (sequential, omp/atomic, omp/array reduction)
* tested datasets/files

### Where to run
sandman ( accessible through scirouter -> `qsubx sandman` )

sandman info :

* Intel(R) Xeon(R) CPU E5-4620 0 @ 2.20GHz
* Sockets : 4
* Cores per socket : 8
* Threads per core : 2
* L3 cache : 16 MB
* NUMA nodes : 4
	* NUMA node0 CPU(s):     0-7,32-39
	* NUMA node1 CPU(s):     8-15,40-47
	* NUMA node2 CPU(s):     16-23,48-55
	* NUMA node3 CPU(s):     24-31,56-63

### Ready to run datasets
Several datasets that I downloaded (perhaps converted from csv or nc formats)

### Candidate datasets
Downloaded all available datasets from [here](http://cucis.ece.northwestern.edu/projects/DMS/MineBenchDownload.html). Need to convert them to .txt format (with spaces between attribute values)

Extra : Check [https://archive.ics.uci.edu/ml/datasets.php](https://archive.ics.uci.edu/ml/datasets.php) too?

### netcdf to txt converter
Python script in this directory to extract data from `.nc` files. Work in progress, nothing to see there.


### TODO 
* Fix I/O timing in `omp_main.c`. Something is reported wrong there.
* Find large size datasets (size determined by cache size).
* Collect first results with seq and omp implementations. Compare different size datasets and cluster-centers.
