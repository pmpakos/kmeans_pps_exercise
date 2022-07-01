#include <stdio.h>
#include <stdlib.h>

#include "kmeans.h"

/*----< euclid_dist_2() >----------------------------------------------------*/
/* square of Euclid distance between two multi-dimensional points            */
__inline static
float euclid_dist_2(int    numdims,  /* no. dimensions */
					float *coord1,   /* [numdims] */
					float *coord2)   /* [numdims] */
{
	int i;
	float ans=0.0;

	for (i=0; i<numdims; i++)
		ans += (coord1[i]-coord2[i]) * (coord1[i]-coord2[i]);

	return(ans);
}

/*----< find_nearest_cluster() >---------------------------------------------*/
__inline static
int find_nearest_cluster(int     numClusters, /* no. clusters */
						 int     numCoords,   /* no. coordinates */
						 float  *object,      /* [numCoords] */
						 float **clusters)    /* [numClusters][numCoords] */
{
	int   index, i;
	float dist, min_dist;

	/* find the cluster id that has min distance to object */
	index    = 0;
	min_dist = euclid_dist_2(numCoords, object, clusters[0]);

	for (i=1; i<numClusters; i++) {
		dist = euclid_dist_2(numCoords, object, clusters[i]);
		/* no need square root */
		if (dist < min_dist) { /* find the min and its array index */
			min_dist = dist;
			index    = i;
		}
	}
	return(index);
}

/*----< seq_kmeans() >-------------------------------------------------------*/
/* return an array of cluster centers of size [numClusters][numCoords]       */
int seq_kmeans(float **objects,      /* in: [numObjs][numCoords] */
			   int     numCoords,    /* no. features */
			   int     numObjs,      /* no. objects */
			   int     numClusters,  /* no. clusters */
			   float   threshold,    /* % objects change membership */
			   int    *membership,   /* out: [numObjs] */
			   float **clusters,             /* out: [numClusters][numCoords] */
			   int   is_output_timing)       /* whether to print progression */

{
	int      i, j, index, loop=0;
	int     *newClusterSize; /* [numClusters]: no. objects assigned in each
								new cluster */
	float    delta;          /* % of objects change their clusters */
	float  **newClusters;    /* [numClusters][numCoords] */
	double   timing;

	/* initialize membership[] */
	for (i=0; i<numObjs; i++) membership[i] = -1;

	/* need to initialize newClusterSize and newClusters[0] to all 0 */
	newClusterSize = (int*) calloc(numClusters, sizeof(int));
	assert(newClusterSize != NULL);

	newClusters    = (float**) malloc(numClusters *            sizeof(float*));
	assert(newClusters != NULL);
	newClusters[0] = (float*)  calloc(numClusters * numCoords, sizeof(float));
	assert(newClusters[0] != NULL);
	for (i=1; i<numClusters; i++)
		newClusters[i] = newClusters[i-1] + numCoords;

	if(is_output_timing)
		timing = wtime();
	
	do {
		delta = 0.0;
		for (i=0; i<numObjs; i++) {
			/* find the array index of nestest cluster center */
			index = find_nearest_cluster(numClusters, numCoords, objects[i],
										 clusters);

			/* if membership changes, increase delta by 1 */
			if (membership[i] != index) delta += 1.0;

			/* assign the membership to object i */
			membership[i] = index;

			/* update new cluster center : sum of objects located within */
			newClusterSize[index]++;
			for (j=0; j<numCoords; j++)
				newClusters[index][j] += objects[i][j];
		}

		/* average the sum and replace old cluster center with newClusters */
		for (i=0; i<numClusters; i++) {
			for (j=0; j<numCoords; j++) {
				if (newClusterSize[i] > 0)
					clusters[i][j] = newClusters[i][j] / newClusterSize[i];
				newClusters[i][j] = 0.0;   /* set back to 0 */
			}
			newClusterSize[i] = 0;   /* set back to 0 */
		}
			
		delta /= numObjs;

		printf("\r\tcurrently at loop %d", loop);
		fflush(stdout);
	} while (delta > threshold && loop++ < 500);

	if(is_output_timing){
		timing = wtime() - timing;
		printf("\n\tnloops = %2d (T = %7.4fs) (per loop = %7.4fs)\n", loop, timing, timing/loop);
	}

	free(newClusters[0]);
	free(newClusters);
	free(newClusterSize);

	return 1;
}
