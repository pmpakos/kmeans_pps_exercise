#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strtok() */
#include <sys/types.h>  /* open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     /* read(), close() */
#include <errno.h>
extern int errno;

#include "kmeans.h"

#define MAX_CHAR_PER_LINE 512

/*---< file_read() >---------------------------------------------------------*/
float** file_read(// int   isBinaryFile,  /* flag: 0 or 1 */
				  char *filename,      /* input file name */
				  int  *numObjs,       /* no. data objects (local) */
				  int  *numCoords)     /* no. coordinates */
{
	float **objects;
	int     i, j, len;
	ssize_t numBytesRead;

	// if (isBinaryFile) {  /* input file is in raw binary format -------------*/
	// 	int infile;
	// 	if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
	// 		fprintf(stderr, "Error: no such file (%s)\n", filename);
	// 		return NULL;
	// 	}
	// 	numBytesRead = read(infile, numObjs,    sizeof(int));
	// 	assert(numBytesRead == sizeof(int));
	// 	numBytesRead = read(infile, numCoords, sizeof(int));
	// 	assert(numBytesRead == sizeof(int));
	// 	if (_debug) {
	// 		printf("File %s numObjs   = %d\n",filename,*numObjs);
	// 		printf("File %s numCoords = %d\n",filename,*numCoords);
	// 	}

	// 	/* allocate space for objects[][] and read all objects */
	// 	len = (*numObjs) * (*numCoords);
	// 	objects    = (float**)malloc((*numObjs) * sizeof(float*));
	// 	assert(objects != NULL);
	// 	objects[0] = (float*) malloc(len * sizeof(float));
	// 	assert(objects[0] != NULL);
	// 	for (i=1; i<(*numObjs); i++)
	// 		objects[i] = objects[i-1] + (*numCoords);

	// 	numBytesRead = read(infile, objects[0], len*sizeof(float));
	// 	assert(numBytesRead == len*sizeof(float));

	// 	close(infile);
	// }
	// else {  /* input file is in ASCII format -------------------------------*/
	FILE *infile;
	char *line, *ret;
	int   lineLen;

	if ((infile = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error: no such file (%s)\n", filename);
		return NULL;
	}

	/* first find the number of objects */
	lineLen = MAX_CHAR_PER_LINE;
	line = (char*) malloc(lineLen);
	assert(line != NULL);

	(*numObjs) = 0;
	while (fgets(line, lineLen, infile) != NULL) {
		/* check each line to find the max line length */
		while (strlen(line) == lineLen-1) {
			/* this line read is not complete */
			len = strlen(line);
			fseek(infile, -len, SEEK_CUR);

			/* increase lineLen */
			lineLen += MAX_CHAR_PER_LINE;
			line = (char*) realloc(line, lineLen);
			assert(line != NULL);

			ret = fgets(line, lineLen, infile);
			assert(ret != NULL);
		}

		if (strtok(line, " \t\n") != 0)
			(*numObjs)++;
	}
	rewind(infile);
	if (_debug) printf("lineLen = %d\n",lineLen);

	/* find the no. coordinates of each object */
	(*numCoords) = 0;
	while (fgets(line, lineLen, infile) != NULL) {
		if (strtok(line, " \t\n") != 0) {
			/* ignore the id (first coordiinate): numCoords = 1; */
			while (strtok(NULL, " ,\t\n") != NULL) (*numCoords)++;
			break; /* this makes read from 1st object */
		}
	}
	// at first, txt files had ID of line as first argument
	// after removing it, need to increase numCoords by 1
	// to read all of them later
	(*numCoords)++;

	rewind(infile);
	if (_debug) {
		printf("File %s numObjs   = %d\n",filename,*numObjs);
		printf("File %s numCoords = %d\n",filename,*numCoords);
	}

	/* allocate space for objects[][] and read all objects */
	len = (*numObjs) * (*numCoords);
	objects    = (float**)malloc((*numObjs) * sizeof(float*));
	assert(objects != NULL);
	objects[0] = (float*) malloc(len * sizeof(float));
	assert(objects[0] != NULL);
	for (i=1; i<(*numObjs); i++)
		objects[i] = objects[i-1] + (*numCoords);

	i = 0;
	/* read all objects */
	while (fgets(line, lineLen, infile) != NULL) {
		char *token = strtok(line, " \t\n");
		if(token==NULL)
			continue;
		
		for (j=0; j<(*numCoords); j++) {
			objects[i][j] = atof(token);
			token = strtok(NULL, " ,\t\n");
			if (_debug && i == 0) /* print the first object */
				printf("object[i=%d][j=%d]=%f\n",i,j,objects[i][j]);
		}
		i++;
	}
	assert(i == *numObjs);

	fclose(infile);
	free(line);
	// }

	return objects;
}

/*---< read_n_objects() >-----------------------------------------------------*/
// int read_n_objects(int     isBinaryFile,  /* flag: 0 or 1 */
// 				   char   *filename,      /* input file name */
// 				   int     numObjs,       /* no. objects */
// 				   int     numCoords,     /* no. coordinates */
// 				   float **objects)       /* [numObjs][numCoords] */
// {
// 	int i, j, len;

// 	if (isBinaryFile) {  /* using MPI-IO to read file concurrently */
// 		int infile;
// 		if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
// 			fprintf(stderr, "Error: open file %s (err=%s)\n",filename,strerror(errno));
// 			return 0;
// 		}
// 		/* read and discard the first 2 integers, numObjs and numCoords */
// 		read(infile, &i, sizeof(int));
// 		read(infile, &i, sizeof(int));

// 		/* read the objects */
// 		read(infile, objects[0], numObjs * numCoords * sizeof(float));

// 		close(infile);
// 	}
// 	else {  /* input file is in ASCII format -------------------------------*/
// 		FILE *infile;
// 		char *line, *ret;
// 		int   lineLen;

// 		if ((infile = fopen(filename, "r")) == NULL) {
// 			fprintf(stderr, "Error: open file %s (err=%s)\n",filename,strerror(errno));
// 			return 0;
// 		}

// 		/* first find the max length of each line */
// 		lineLen = MAX_CHAR_PER_LINE;
// 		line = (char*) malloc(lineLen);
// 		assert(line != NULL);

// 		while (fgets(line, lineLen, infile) != NULL) {
// 			/* check each line to find the max line length */
// 			while (strlen(line) == lineLen-1) {
// 				/* this line read is not complete */
// 				len = strlen(line);
// 				fseek(infile, -len, SEEK_CUR);

// 				/* increase lineLen */
// 				lineLen += MAX_CHAR_PER_LINE;
// 				line = (char*) realloc(line, lineLen);
// 				assert(line != NULL);

// 				ret = fgets(line, lineLen, infile);
// 				assert(ret != NULL);
// 			}
// 		}
// 		rewind(infile);

// 		/* read numObjs objects */
// 		for (i=0; i<numObjs; i++) {
// 			fgets(line, lineLen, infile);
// 			if (strtok(line, " \t\n") == NULL) continue;
// 			for (j=0; j<numCoords; j++)
// 				objects[i][j] = atof(strtok(NULL, " ,\t\n"));
// 		}
// 		fclose(infile);
// 		free(line);
// 	}
// 	return 1;
// }

/*---< file_write() >---------------------------------------------------------*/
int file_write(char      *filename,     /* input file name */
			   int        numClusters,  /* no. clusters */
			   int        numObjs,      /* no. data objects */
			   int        numCoords,    /* no. coordinates (local) */
			   float    **clusters,     /* [numClusters][numCoords] centers */
			   int       *membership,   /* [numObjs] */
			   int        verbose)
{
	FILE *fptr;
	int   i, j;
	char  outFileName[1024];

	/* output: the coordinates of the cluster centres ----------------------*/
	sprintf(outFileName, "%s.cluster_centres", filename);
	// if (verbose)
	// 	printf("Writing coordinates of K=%d cluster centers to file \"%s\"\n", numClusters, outFileName);
	fptr = fopen(outFileName, "w");
	for (i=0; i<numClusters; i++) {
		fprintf(fptr, "%d ", i);
		for (j=0; j<numCoords; j++)
			fprintf(fptr, "%f ", clusters[i][j]);
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	/* output: the closest cluster centre to each of the data points --------*/
	sprintf(outFileName, "%s.membership", filename);
	// if (verbose)
	// 	printf("Writing membership of N=%d data objects to file \"%s\"\n", numObjs, outFileName); 
	fptr = fopen(outFileName, "w");
	for (i=0; i<numObjs; i++)
		fprintf(fptr, "%d %d\n", i, membership[i]);
	fclose(fptr);

	printf("Wrote coordinates of %d cluster centers and membership of %d data objects\n", numClusters, numObjs);

	return 1;
}

/*---< dataset_generation() >-------------------------------------------------*/
float** dataset_generation(int numObjs, int numCoords)
{
	float **objects;
	int i, j;

	/* allocate space for objects[][] and read all objects */
	objects = (float**)malloc(numObjs * sizeof(float*));
	assert(objects != NULL);
	for(i=0; i<numObjs; i++)
		objects[i] = (float*) malloc(numCoords * sizeof(float));
	assert(objects[0] != NULL);

	// random values that will be generated will be between 0 and 10
	float val_range = 10;
	for(i=0; i<numObjs; i++){
		// objects[i] = objects[i-1] + numCoords;
		for (j=0; j<numCoords; j++) {
			objects[i][j] = ((float)rand()/(float)(RAND_MAX)) * val_range;
			if (_debug && i == 0) /* print the first object */
				printf("object[i=%d][j=%d]=%f\n",i,j,objects[i][j]);
		}
	}

	return objects;
}