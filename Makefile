.KEEP_STATE:

all: seq omp

ENABLE_PNETCDF = no
PNETCDF_DIR    = $(HOME)/PnetCDF

CC             = gcc
OMPCC          = gcc

INCFLAGS    = -I.
OPTFLAGS    = -O2 -DNDEBUG
LDFLAGS     = $(OPTFLAGS)

ifeq ($(ENABLE_PNETCDF), yes)
INCFLAGS   += -I$(PNETCDF_DIR)/include
DFLAGS     += -D_PNETCDF_BUILT
LIBS       += -L$(PNETCDF_DIR)/lib -lpnetcdf
endif

CFLAGS      = $(OPTFLAGS) $(DFLAGS) $(INCFLAGS)


# please check the compile manual for the openmp flag
# Here, I am using gcc and the flag is -fopenmp
# If icc is used, it is -opnemp
OMPFLAGS    = -fopenmp

H_FILES     = kmeans.h

COMM_SRC = file_io.c util.c

#------   OpenMP version -----------------------------------------
OMP_SRC     = omp_main.c \
			  omp_kmeans.c

OMP_OBJ     = $(OMP_SRC:%.c=%.o) $(COMM_SRC:%.c=%.o)

ifeq ($(ENABLE_PNETCDF), yes)
OMP_OBJ    += pnetcdf_io.o
endif

omp_main.o: omp_main.c $(H_FILES)
	$(OMPCC) $(CFLAGS) $(OMPFLAGS) -c $*.c

omp_kmeans.o: omp_kmeans.c $(H_FILES)
	$(OMPCC) $(CFLAGS) $(OMPFLAGS) -c $*.c

omp: omp_main
omp_main: $(OMP_OBJ)
	$(OMPCC) $(LDFLAGS) $(OMPFLAGS) -o $@ $(OMP_OBJ) $(LIBS)

#------   sequential version -----------------------------------------
SEQ_SRC     = seq_main.c   \
              seq_kmeans.c \
              wtime.c

SEQ_OBJ     = $(SEQ_SRC:%.c=%.o) $(COMM_SRC:%.c=%.o)

$(SEQ_OBJ): $(H_FILES)

seq_main.o: seq_main.c $(H_FILES)
	$(CC) $(CFLAGS) -c $*.c

seq_kmeans.o: seq_kmeans.c $(H_FILES)
	$(CC) $(CFLAGS) -c $*.c

wtime.o: wtime.c
	$(CC) $(CFLAGS) -c $*.c

seq: seq_main
seq_main: $(SEQ_OBJ) $(H_FILES)
	$(CC) $(LDFLAGS) -o $@ $(SEQ_OBJ) $(LIBS)


clean:
	rm -rf *.o omp_main seq_main \
		bin2nc core* .make.state \
		Image_data/*.cluster_centres Image_data/*.membership \
		Image_data/*.cluster_centres.nc Image_data/*.membership.nc \
		artificial_datasets/*.cluster_centres artificial_datasets/*.membership

check: all
	# sequential K-means ---------------------------------------------------
	./seq_main -b -n 4 -i Image_data/color17695.bin
	./seq_main    -n 4 -i Image_data/color100.txt
	# OpenMP K-means using pragma atomic -----------------------------------
	./omp_main -b -n 4 -a -p 4 -i Image_data/color17695.bin
	./omp_main    -n 4 -a -p 4 -i Image_data/color100.txt

	# OpenMP K-means using array reduction ---------------------------------
	./omp_main -b -n 4 -p 4 -i Image_data/color17695.bin
	./omp_main    -n 4 -p 4 -i Image_data/color100.txt
