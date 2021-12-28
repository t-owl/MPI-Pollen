# Overview

This Project knowledge of MPI programming to parallelise and extend a given serial code.

The serial code is a naïve model of pollen blowing in the wind and slowly falling to the ground. We initialise NUMPOLLEN pollen particles to random (x,y,z) and for each of MAXTIME timesteps we apply a given wind function and a force proportional to gravity and count how many pollen particles have hit the ground in timestep t, `totalNumGround[t]`.

The main aim of this project is to implement an efficient MPI code that gives the correct prediction of how many pollen particles with have fallen to the ground after 10,000 timesteps, while having a good time performance.

# Parallel Code

In order to make the code parallel the following steps were followed:

 First it was a matter of identifying the parallel and serial regions of the code; the independent iterations, the independent variables, etc. Then, decide how to split the data among the number of ranks, this involved to think how much data needs to be communicated and how often. Next was to set up tests, by splitting the program into different functions as well as reducing the problem size, this help to have a better understanding of the code and also to create a more efficient code. Finally once the programs worked correctly independently  it was a matter of putting everything together and scaling up the problem size. 

The parallelisation of the code is divided into two main parts, one part handles the simulation of pollen falling into the ground (Part 1), where as the other handles the computation of the best linear fit line (Part 2). 

**Part 1**. The process with rank 0 acts as the root / master, this process coordinates with the other processes to perform the job. The `root ` process initialises the data and it then **broadcasts** the data to all other processes. Once each process has received the data each process decides on which part of the data they need to perform processing, the amount of data they have to work in is decided in the overhead by the division of the tasks and the number of process available. Once results are computed then they are accumulated using **reduction (`Reduce/Allreduce`)** process, e.g. if four processes have computed the local sum based on their part of data then we will accumulate them into one grand sum by using reduction.



# Set up & Run 

### Serial solution

#### Set up

To start with the serial solution was a modified version of the original `pollen.c` file, the modified version added the two functions two calculate the Y = mX+c formula. The Intel compiler module was needed as this was a simple c file. In order to compile and run the file a batch script was created to communicate with the `Slurm` system of `barkla`, this file contained the commands for **loading the Intel** module, and because the aim of this experiment is to explore and discuss parallelism all compilations should be done without the compiler optimisation, therefore the **`-O0` flag** was used, finally to time the time taken by the program, the `time` function was used this output the time taken at the end of the output.

```bash
## intel compiler
module load compilers/intel/2019u5 

# INTEL no-opt
echo INTEL no-opt
icc -O0 pollen-serial.c -o icc-serial.out
time ./icc-serial.out
echo '-------'
```

#### Run

To run the bash file all it was needed in this case was the following command

```
sbatch run-serial.sh 
```

This would run the script described in the set up section, and create a `slurm` file as an output.

### Parallel solution

#### Set up

On the other hand the parallel solution took the serial solution and added mpi functions to parallelise the program. In this case the mpi Intel compiler was also needed to run the new mpi functions added to the code. In the same way a batch script was created. where this time two modules were added (the **intel c compiler** and the **mpi intel c compiler**), the program was compiled using the **`O0` flag** as with the serial solution to further discuss parallelism. Similarly to the serial solution the time was taken from the `time` function in the batch file. Finally if the number of processes is not specified the program will run in 1 process, to specify the number of process the -n flag needs to be called along with the number of processes e.g. -n 4. this will be further explain in the run section of parallel solution. 

```bash
# load modules
## intel compiler
module load compilers/intel/2019u5 
## intel mpi wrapper and run time
module load mpi/intel-mpi/2019u5/bin

# parallel using MPI
SRC=pollen-mpi.c
EXE=${SRC%%.c}.exe
echo compiling $SRC to $EXE

export numMPI=${SLURM_NTASKS:-1} # if '-n' not used then default to 1

mpiicc -O0 $SRC -o $EXE 

time mpirun -np ${numMPI} ./${EXE};echo


```

#### Run

In particular the mpi script needed the definition of the -n flag, this was done to declare the number of processes used, and this would determine on how the program was divided. to illustrate this two examples are presented. 

| Commands                | Description                                                  |
| ----------------------- | ------------------------------------------------------------ |
| sbatch -n 2 run-mpi.sh  | to start a batch job while parsing "2" as the number of processes |
| sbatch -n 32 run-mpi.sh | to start a batch job while parsing "32" as the number of processes |

## Best Linear fit 

In order to find the best linear fit for the data produce by the simulation, the formula given in the *overview document* was used, where x is the time and y is the number of pollen that has fallen into the ground.

The line that determines the best linear fit is **Y = mX+c**. (Mathsisfun.com, 2017)

So to find m and c we apply the following formulas:

<img src="https://render.githubusercontent.com/render/math?math=m=\frac{N\sum_{xy}-\sum_{x}\sum_{y}}{N\sum_{x^{2}}-(\sum_{x})^{2}}">

<img src="https://render.githubusercontent.com/render/math?math=c=\frac{\sum_{y}-m\sum_{x}}{N}">


In the implementation of the serial code two functions were created for each variable m and c, m being the slope of the line and c being the intercept (where the line intercepts the y axes), the two functions were created to keep a more organised code and follow good coding practices.

The code takes as input two arrays; one holds the total number of pollen that has fallen into the ground after each time-step as well, the other one holds the time-steps. with this two inputs the functions are able to find m and c, and it is finally outputted in the console.

The resulting formula for this experiment was: 

y = 156.105009 x - 475.977065

To check the correctness of this formula a graph with the pollen data points was plotted against the formula in excel **Y = mX+c**.

![](https://i.imgur.com/Dq1kMSs.png)

Due to the amount of data recorded i.e. 5000 data points, the data was spitted into 26 data points in excel, this was done to further improve the understanding of the resulted graph. (Refer to appendix A for full best linear fit data)

The line representing the best linear fit was drawn by taking the y value when x was equal to 0 and by taking the y value when x was equal to 5100.

Consequently as expected, the line of best fit goes along with the given data, confirming that the derived formula is correct.

### Prediction of pollen particles after 10,000 time steps

After confirming that the formula is correct we can then predict how may particles are on the ground after 10,000 time steps, we just need to replace the x value on the formula with 10,000, and the result will be the value of y (the number of pollen particles on the ground).

y = 156.105009 x - 475.977065

y = 156.105009 * 10000 - 475.977065

y = 1560574.113

the predicted number of particles after 10000 time steps is **1560574.113**



## Performance 

The performance of a parallel code is one of the priorities for this assignment, a good performance will show a good implementation of MPI functions.

The key aspects of performance are the speedup, efficiency and scalability.

Speed up is the ratio between the time taken in 1 core (or serial implementation) divided by the time taken in several cores (p)cores. i.e. the ideal speedup is going to be when running the program on the double number of cores, and the time takes half as long, so the speed up would be 2

Efficiency is the percentage of the speedup dived by the number of cores, so the ideal efficiency  would be 100%. Efficiency above 80% is considered good efficiency 

In this experiment the number of cores used to evaluate the performance were the following. Number of cores {1,2,3,4,8,16,19,32,40}, each value is twice that of its predecessor except for {3 and 19},this was to represent the speedup ratio, {3 and 19} were added to confirm the code handle  odd numbers and that this values did not affected the expected speed up factor.

![](https://i.imgur.com/dQ0kvBj.png)


In the graph bellow we can appreciate the speed up is going up in an almost linear way, this shows that a good handling of the overhead for communication and synchronisation between cores has been made, and the addition of more cores to the task is not affecting the performance drastically.

![](https://i.imgur.com/pcGlMwC.png)


Although the speed up graph looks linear, we can further understand the problem by looking at the efficiency graph. For the first 3 values we can see that initially we have good efficiency but after the 4th value the efficiency starts dropping off.

![](https://i.imgur.com/bYr5ApH.png)


After understanding the graphs, we can then decide on how many cores would be optimal to run the program on. The quickest run is with 40 cores which took 7.99 seconds, but we are down to 69% of efficiency, so as mentioned before efficiency above 80% is considered good efficiency, therefore if we want to make a good use of our parallel program and reduce time we might use 19 cores instead which would run on 13.75 seconds and be 84% efficient. (Refer to appendix B for full performance data)

By looking at this data we can conclude the program follows a **strong scaling** approach as the number of cores increases the time to the solution decreases proportionally.

To get the time measured `MPI_Wtime()` was used through the code. variable called {t1,t2,t3,t4} were used to time different parts of the code and by subtracting this values the running time of each section was found. 

**Accuracy**. Through all experiments the result for the **Y = mX+c** formula has been constant, `double` variables has been used across the code this means that is accurate to the 16 decimal place.
