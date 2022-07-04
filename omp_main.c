#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strtok() */
#include <sys/types.h>  /* open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     /* getopt() */

#include <omp.h>
int      _debug;
#include "kmeans.h"

#ifdef _PNETCDF_BUILT
#include <mpi.h>
#include <pnetcdf.h>
float** pnetcdf_read(char*, char*, int*, int*, MPI_Comm);
int     pnetcdf_write(char*, int, int, int, int, float**, int*, int, MPI_Comm, int verbose);
#endif

/*---< usage() >------------------------------------------------------------*/
static void usage(char *argv0, float threshold) {
	char *help =
		"Usage: %s [switches] -i filename -n num_clusters\n"
		"       -i filename    : file containing data to be clustered\n"
		"       -c num_clusters: number of clusters (K must > 1)\n"
		"       -s size        : size of examined dataset\n"
		"       -n num_coords  : number of coordinates (must be > num_clusters)\n"
		"       -t threshold   : threshold value (default %.4f)\n"
		"       -a             : perform atomic OpenMP pragma (default no)\n"
		"       -o             : output timing results (default no)\n"
		"       -q             : quiet mode\n"
		"       -d             : enable debug mode\n"
		"       -h             : print this help information\n";
	fprintf(stderr, help, argv0, threshold);
	exit(-1);
}

/*---< main() >---------------------------------------------------------------*/
int main(int argc, char **argv) {
	int     opt;
	extern char   *optarg;
	extern int     optind;
	int     i, j, nthreads, verbose;
	int     isBinaryFile, is_perform_atomic, is_output_timing;
	int     do_pnetcdf;

	int     numClusters, numCoords, numObjs;
	int    *membership;    /* [numObjs] */
	char   *filename;
	char   *var_name;
	float **objects;       /* [numObjs][numCoords] data objects */
	float **clusters;      /* [numClusters][numCoords] cluster center */
	float   dataset_size, threshold;
	double  timing, io_timing, io_timing_read, io_timing_write, clustering_timing, timer;

	#ifdef _PNETCDF_BUILT
	MPI_Init(&argc, &argv);
	#endif

	/* some default values */
	_debug            = 0;
	verbose           = 1;
	nthreads          = 0;
	numClusters       = 0;
	threshold         = 0.001;
	numClusters       = 0;
	isBinaryFile      = 0;
	is_output_timing  = 0;
	is_perform_atomic = 0;
	filename          = NULL;
	do_pnetcdf        = 0;
	var_name          = NULL;

	while ( (opt=getopt(argc,argv,"p:i:n:t:c:v:s:adoqh"))!= EOF) {
		switch (opt) {
			case 'i': filename=optarg;
					  break;
			case 'c': numClusters = atoi(optarg);
					  break;
			case 't': threshold=atof(optarg);
					  break;
			case 's': dataset_size=atof(optarg);
					  break;
			case 'n': numCoords=atoi(optarg);
					  break;
			case 'p': nthreads = atoi(optarg);
					  break;
			case 'a': is_perform_atomic = 1;
					  break;
			case 'd': _debug = 1;
					  break;
			case 'o': is_output_timing = 1;
					  break;
			case 'q': verbose = 0;
					  break;
			case 'h':
			default: usage(argv[0], threshold);
					  break;
		}
	}
	numObjs = (int) (dataset_size*1024*1024)/(numCoords*sizeof(float));
	printf("dataset_size = %lf\tnumObjs = %d\tnumCoords = %d\n", dataset_size, numObjs, numCoords);

	if (filename == 0 || numClusters <= 1) usage(argv[0], threshold);

	// #ifndef _PNETCDF_BUILT
	// if (do_pnetcdf) {
	// 	printf("Error: PnetCDF feature is not built\n");
	// 	exit(1);
	// }
	// #endif

	/* set the no. threads if specified in command line, else use all
	   threads allocated by run-time system */
	if (nthreads > 0)
		omp_set_num_threads(nthreads);

	if (is_output_timing) {
		io_timing_read  = omp_get_wtime();
	}

	/* read data points from file ------------------------------------------*/
	// #ifdef _PNETCDF_BUILT
	// if (do_pnetcdf)
	// 	objects = pnetcdf_read(filename, var_name, &numObjs, &numCoords, MPI_COMM_WORLD);
	// else
	// #endif
	
	// objects = file_read(isBinaryFile, filename, &numObjs, &numCoords);
	objects = dataset_generation(numObjs, numCoords);
	if (objects == NULL) exit(1);

	if (numObjs < numClusters) {
		printf("Error: number of clusters must be larger than the number of data points to be clustered.\n");
		free(objects[0]);
		free(objects);
		return 1;
	}

	/* allocate a 2D space for clusters[] (coordinates of cluster centers)
	   this array should be the same across all processes                  */
	clusters    = (float**) malloc(numClusters *             sizeof(float*));
	assert(clusters != NULL);
	clusters[0] = (float*)  malloc(numClusters * numCoords * sizeof(float));
	assert(clusters[0] != NULL);
	for (i=1; i<numClusters; i++)
		clusters[i] = clusters[i-1] + numCoords;

	// printf("selecting the first %d elements as initial centers\n", numClusters);
	/* copy the first numClusters elements in feature[] */
	for (i=0; i<numClusters; i++)
		for (j=0; j<numCoords; j++)
			clusters[i][j] = objects[i][j];

	/* check initial cluster centers for repeatition */
	if (check_repeated_clusters(numClusters, numCoords, clusters) == 0) {
		printf("Error: some initial clusters are repeated. Please select distinct initial centers\n");
		return 1;
	}

	if (_debug) {
		printf("Sorted initial cluster centers:\n");
		for (i=0; i<numClusters; i++) {
			printf("clusters[%d]=",i);
			for (j=0; j<numCoords; j++)
				printf(" %6.2f", clusters[i][j]);
			printf("\n");
		}
	}

	if (is_output_timing) {
		timing            = omp_get_wtime();
		io_timing_read    = timing - io_timing_read;
		printf("I/O time (read phase): %.4f\n", io_timing_read);
		clustering_timing = timing;
	}      

	/* start the core computation -------------------------------------------*/
	/* membership: the cluster id for each data object */
	membership = (int*) malloc(numObjs * sizeof(int));
	assert(membership != NULL);

	timer = omp_kmeans(is_perform_atomic, objects, numCoords, numObjs, numClusters, threshold, membership, clusters, is_output_timing);

	free(objects[0]);
	free(objects);

	if (is_output_timing) {
		timing            = omp_get_wtime();
		clustering_timing = timing - clustering_timing;
		io_timing_write   = timing;
	}       

	/* output: the coordinates of the cluster centres ----------------------*/
	#ifdef _PNETCDF_BUILT
	if (do_pnetcdf)
		pnetcdf_write(filename, 1, numClusters, numObjs, numCoords, clusters, membership, numObjs, MPI_COMM_SELF, verbose);
	else
	#endif

	file_write(filename, numClusters, numObjs, numCoords, clusters, membership, verbose);

	free(membership);
	free(clusters[0]);
	free(clusters);

	/*---- output performance numbers ---------------------------------------*/
	if (is_output_timing) {
		io_timing_write = omp_get_wtime() - io_timing_write;
		io_timing       = io_timing_read + io_timing_write;

		// printf("\nPerforming **** Regular Kmeans  (OpenMP) ----");
		// if (is_perform_atomic)
		//     printf(" using atomic pragma ******\n");
		// else
		//     printf(" using array reduction ******\n");

		// printf("Number of threads = %d\n", omp_get_max_threads());
		// printf("Input file:     %s\n", filename);
		// printf("numObjs       = %d\n", numObjs);
		// printf("numCoords     = %d\n", numCoords);
		// printf("dataset       = %f MB\n", sizeof(float)*numObjs*numCoords*1.0/(1024*1024));
		// printf("numClusters   = %d\n", numClusters);
		// printf("threshold     = %.4f\n", threshold);
		// printf("I/O time      = %10.4f sec\n", io_timing);
		// printf("Computation   = %10.4f sec\n", clustering_timing);

		if (is_perform_atomic)
			printf("omp_atomic,");
		else
			printf("omp_array_reduction,");
		printf("%d,", omp_get_max_threads());
		printf("%d,", numObjs);
		printf("%d,", numCoords);
		printf("%f,", sizeof(float)*numObjs*numCoords*1.0/(1024*1024));
		printf("%d,", numClusters);
		printf("%.4f,", io_timing);
		printf("%.4f\n\n", clustering_timing);
	}

	#ifdef _PNETCDF_BUILT
	MPI_Finalize();
	#endif

	return(0);
}
