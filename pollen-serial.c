#include <stdio.h>
#include <stdlib.h>  /* for rand() */
#include <sys/time.h>  // for wallclock timing functions


#define NUMPOLLEN 1250000
#define FALLRATE 0.025
#define MAXTIME 5000

// global vars
float vx[NUMPOLLEN], vy[NUMPOLLEN], vz[NUMPOLLEN];
float x[NUMPOLLEN], y[NUMPOLLEN], z[NUMPOLLEN];
int totalNumGround[MAXTIME], time[MAXTIME];

float summation(int *, int);
float summation_2var(int *, int *, int);


void initialise();

float wind(int n) {
/* simple case: random float [-1,+1] 
   - this cannot be random if we need to be able to 
     repeat experiment and determine each (x,y) which wind() is used for
*/
  float velocity = -1.0 + 2.0*exp(-((float)n/(float)NUMPOLLEN+1.0));
  return velocity;
}

double slope(double x_time[MAXTIME], double y_pollenGround[MAXTIME]){
    double m;
    int i;
    double sumXY=0;
    double sumX=0;
    double sumX2=0;
    double sumY=0;
    for(i=0;i<MAXTIME;i++){
        sumXY=sumXY+x_time[i]*y_pollenGround[i];
        sumX=sumX+x_time[i];
        sumY=sumY+y_pollenGround[i];
        sumX2=sumX2+x_time[i]*x_time[i];
    }
    sumXY=sumXY/MAXTIME;
    sumX=sumX/MAXTIME;
    sumY=sumY/MAXTIME;
    sumX2=sumX2/MAXTIME;
    m=(sumXY-sumX*sumY)/(sumX2-sumX*sumX);
    return m;
}

double intercept(double x_time[MAXTIME], double y_pollenGround[MAXTIME]){
    double c;
    int i;
    double sumXY=0;
    double sumX=0;
    double sumX2=0;
    double sumY=0;
    for(i=0;i<MAXTIME;i++){
        sumXY=sumXY+x_time[i]*y_pollenGround[i];
        sumX=sumX+x_time[i];
        sumY=sumY+y_pollenGround[i];
        sumX2=sumX2+x_time[i]*x_time[i];
    }
    sumXY=sumXY/MAXTIME;
    sumX=sumX/MAXTIME;
    sumY=sumY/MAXTIME;
    sumX2=sumX2/MAXTIME;
    c=(sumX2*sumY-sumXY*sumX)/(sumX2-sumX*sumX);
    return c;
}


int main(void) {
  int timestep=0;
  int i;

  double x_time[MAXTIME] = {};
  double y_pollenGround[MAXTIME] = {};

  /* for timing */
  struct timeval wallStart, wallEnd;
  gettimeofday(&wallStart, NULL); // save start time in to variable 'wallStart'

  initialise();

  for (timestep; timestep<MAXTIME; timestep++) {
    for (i=0; i<NUMPOLLEN; i++) {
      if(z[i] > 0.0) {
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
    totalNumGround[timestep]=0;
    time[timestep]=timestep;
    for (i=0; i<NUMPOLLEN; i++) {
      if(z[i] <= 0.0) totalNumGround[timestep]++;
    }
    printf("Timestep %d: %d particles on ground\n", timestep, totalNumGround[timestep]);

    
    //Arrays to pass to the y = m x + c
    x_time[timestep] = timestep;
    y_pollenGround[timestep] = totalNumGround[timestep];
    

  } // end time stepping loop

    // output time
  gettimeofday(&wallEnd, NULL); // end time
  double wallSecs = (wallEnd.tv_sec - wallStart.tv_sec);           // just integral number of seconds
  double WALLtimeTaken = 1.0E-06 * ((wallSecs*1000000) + (wallEnd.tv_usec - wallStart.tv_usec)); // and now with any microseconds
  printf("%d pollen for %d timesteps SERIAL CODE takes %f seconds\n\n", NUMPOLLEN, MAXTIME, WALLtimeTaken);
  
  double m=slope(x_time,y_pollenGround);
  double c=intercept(x_time,y_pollenGround);
  printf("y = %lf x + %lf\n\n",m,c);

    
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
  for (i=0; i<NUMPOLLEN; i++) {
    x[i] = -100.0 + 200.0*(float)rand()/(float)RAND_MAX;
    y[i] = -100.0 + 200.0*(float)rand()/(float)RAND_MAX;
    //    z[i] =  50.0 + 50.0*(float)rand()/(float)RAND_MAX;
    z[i] =  0.0 + 200.0*(float)rand()/(float)RAND_MAX;
    vx[i] = (float)rand()/(float)RAND_MAX;
    vy[i] = (float)rand()/(float)RAND_MAX;
    vz[i] = 0.0;
  }

}
