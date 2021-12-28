#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>  /* for rand() */
#include <sys/time.h>  // for wallclock timing functions
#include <math.h>


#define NUMPOLLEN 1250000
#define FALLRATE 0.025
#define MAXTIME 5000
#define ROOT 0

// global vars
float vx[NUMPOLLEN], vy[NUMPOLLEN], vz[NUMPOLLEN];
float x[NUMPOLLEN], y[NUMPOLLEN], z[NUMPOLLEN];
int totalNumGround[MAXTIME], time[MAXTIME];

float summation(int*, int);
float summation_2var(int*, int*, int);


void initialise();
int num_procs, rank;

float wind(int n) {
    /* simple case: random float [-1,+1]
       - this cannot be random if we need to be able to
         repeat experiment and determine each (x,y) which wind() is used for
    */
    float velocity = -1.0 + 2.0 * exp(-((float)n / (float)NUMPOLLEN + 1.0));
    return velocity;
}

double slope(double x_time[MAXTIME], double y_pollenGround[MAXTIME]) {

    // share the work across the number of processes 
    /* split iterations over MPI processes */
    int times_per_proc = MAXTIME / num_procs;
    int undiv_time = MAXTIME % num_procs;

    int my_first_time = rank * times_per_proc;
    int my_last_time = my_first_time + times_per_proc - 1;

    if (rank != ROOT)
    {
    	my_first_time += undiv_time;
    	my_last_time += undiv_time;
    }
    else
    {
    	my_last_time += undiv_time;
    }

    double m;
    int i;
    double sumXY = 0, sumXY_local = 0;
    double sumX = 0, sumX_local = 0;
    double sumX2 = 0, sumX2_local = 0;
    double sumY = 0, sumY_local = 0;
    for (i = my_first_time; i <= my_last_time; i++) {
        sumXY_local = sumXY_local + x_time[i] * y_pollenGround[i];
        sumX_local = sumX_local + x_time[i];
        sumY_local = sumY_local + y_pollenGround[i];
        sumX2_local = sumX2_local + x_time[i] * x_time[i];
    }

    MPI_Reduce(&sumXY_local, &sumXY, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);
    MPI_Reduce(&sumX_local, &sumX, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);

    MPI_Reduce(&sumX2_local, &sumX2, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);

    MPI_Reduce(&sumY_local, &sumY, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);

    // if we are back at the root rank, compute "m"
    if (rank == 0)
    {
	    sumXY = sumXY / MAXTIME;
	    sumX = sumX / MAXTIME;
	    sumY = sumY / MAXTIME;
	    sumX2 = sumX2 / MAXTIME;
	    m = (sumXY - sumX * sumY) / (sumX2 - sumX * sumX);
	}
    return m;
}

double intercept(double x_time[MAXTIME], double y_pollenGround[MAXTIME]) {
    
    // share the work across the number of processes 
    /* split iterations over MPI processes */
	int times_per_proc = MAXTIME / num_procs;
    int undiv_time = MAXTIME % num_procs;

    int my_first_time = rank * times_per_proc;
    int my_last_time = my_first_time + times_per_proc - 1;

    if (rank != ROOT)
    {
    	my_first_time += undiv_time;
    	my_last_time += undiv_time;
    }
    else
    {
    	my_last_time += undiv_time;
    }
    double c;
    int i;
    double sumXY = 0, sumXY_local = 0;
    double sumX = 0, sumX_local = 0;
    double sumX2 = 0, sumX2_local = 0;
    double sumY = 0, sumY_local = 0;
    for (i = my_first_time; i <= my_last_time; i++) {
        sumXY_local = sumXY_local + x_time[i] * y_pollenGround[i];
        sumX_local = sumX_local + x_time[i];
        sumY_local = sumY_local + y_pollenGround[i];
        sumX2_local = sumX2_local + x_time[i] * x_time[i];
    }

    MPI_Reduce(&sumXY_local, &sumXY, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);
    MPI_Reduce(&sumX_local, &sumX, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);

    MPI_Reduce(&sumX2_local, &sumX2, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);

    MPI_Reduce(&sumY_local, &sumY, 1, MPI_DOUBLE, 
        	MPI_SUM, ROOT, MPI_COMM_WORLD);


    // if we are back at the root rank, compute "m"
    if (rank == 0)
    {
	    sumXY = sumXY / MAXTIME;
	    sumX = sumX / MAXTIME;
	    sumY = sumY / MAXTIME;
	    sumX2 = sumX2 / MAXTIME;
    	c = (sumX2 * sumY - sumXY * sumX) / (sumX2 - sumX * sumX);
    }
    return c;
}


int main(void) {


	
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int timestep = 0;
    int i;

    double x_time[MAXTIME] = {};
    double y_pollenGround[MAXTIME] = {};

    /* for timing */
    struct timeval wallStart, wallEnd;
    gettimeofday(&wallStart, NULL); // save start time in to variable 'wallStart'


    double t1 = MPI_Wtime();

    if (rank == ROOT)
    {
    	initialise();
    }
    MPI_Bcast(x, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(y, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(z, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(vx, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(vy, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(vz, NUMPOLLEN, MPI_FLOAT, ROOT, MPI_COMM_WORLD);


    int pollens_per_proc = NUMPOLLEN / num_procs;
    int undiv_poll = NUMPOLLEN % num_procs;

    int my_first_pollen = rank * pollens_per_proc;
    int my_last_pollen = my_first_pollen + pollens_per_proc - 1;

    if (rank != ROOT)
    {
    	my_first_pollen += undiv_poll;
    	my_last_pollen += undiv_poll;
    }
    else
    {
    	my_last_pollen += undiv_poll;
    }

    for (timestep; timestep < MAXTIME; timestep++) {


        for (i = my_first_pollen; i <= my_last_pollen; i++) 
        {
            if (z[i] > 0.0) {
                /*
                  update (x,y) via wind presuming unit timesteps
                  so change in velocity is result of wind()
                  thus can update change in position - independence over pollen particles
                */
                vx[i] += wind(i);
                vy[i] += wind(i);
                x[i] += vx[i];
                y[i] += vy[i];
                // z-wise particles only sink, due to gravity which pulls down
                vz[i] = -FALLRATE;
                z[i] += vz[i];


            }
        }

        // determine # on ground 
        totalNumGround[timestep] = 0;
        time[timestep] = timestep;
        for (i = my_first_pollen; i <= my_last_pollen; i++) {
            if (z[i] <= 0.0) totalNumGround[timestep]++;
        }

        int pollens_sum = 0;
        MPI_Allreduce(&totalNumGround[timestep], &pollens_sum, 1, MPI_INT, 
        	MPI_SUM, MPI_COMM_WORLD);
        totalNumGround[timestep] = pollens_sum;
        if (rank == ROOT)
        {
        	
        	printf("Timestep %d: %d particles on ground\n", timestep, totalNumGround[timestep]);
        }
        

        //Arrays to pass to the y = m x + c
        x_time[timestep] = timestep;
        y_pollenGround[timestep] = totalNumGround[timestep];


    } // end time stepping loop

    double t2 = MPI_Wtime();



      // output time
    gettimeofday(&wallEnd, NULL); // end time
    double wallSecs = (wallEnd.tv_sec - wallStart.tv_sec);           // just integral number of seconds
    double WALLtimeTaken = 1.0E-06 * ((wallSecs * 1000000) + (wallEnd.tv_usec - wallStart.tv_usec)); // and now with any microseconds
    
    if (rank == ROOT)
    {
    	printf("Pollen time: %g sec\n", t2 - t1 );
    	printf("%d pollen for %d timesteps Parallel CODE takes %f seconds\n\n", NUMPOLLEN, MAXTIME, WALLtimeTaken);
    }


    double t3 = MPI_Wtime();
    double m = slope(x_time, y_pollenGround);
    double c = intercept(x_time, y_pollenGround);
    double t4 = MPI_Wtime();
    if (rank == ROOT)
    {
    	
    	printf("y = %lf x + %lf\n\n", m, c);
    	printf("Time taken to compute slop and intercept: %g secs\n", t4 - t3 );
    	printf("Total time taken: %g secs\n", t4 - t1 );
    }


    MPI_Finalize();

    return 0;


} // main



void initialise() {
    /*
       you should NOT parallelise the workings inside this function
       since that may affect the ordering of the random numbers generated

       domain is [-100,100] in x,y
       random (x,y) within domain,
       random heights (z) from 0 to 200
       random initial vx,vy in range 0 to 1, vz fixed at 0
    */
    int i;
    for (i = 0; i < NUMPOLLEN; i++) {
        x[i] = -100.0 + 200.0 * (float)rand() / (float)RAND_MAX;
        y[i] = -100.0 + 200.0 * (float)rand() / (float)RAND_MAX;
        //    z[i] =  50.0 + 50.0*(float)rand()/(float)RAND_MAX;
        z[i] = 0.0 + 200.0 * (float)rand() / (float)RAND_MAX;
        vx[i] = (float)rand() / (float)RAND_MAX;
        vy[i] = (float)rand() / (float)RAND_MAX;
        vz[i] = 0.0;
    }

}