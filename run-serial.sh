#!/bin/bash -l

# Specify the current working directory as the location for executables/files
# This is the default setting.
#SBATCH -D ./

# Export the current environment to the compute node
# This is the default setting.
#SBATCH --export=ALL

# Specific course queue
#SBATCH -p course

# 4 cores for auto-par version
#SBATCH -c 20
#SBATCH --exclusive

# load modules
## intel compiler
module load compilers/intel/2019u5 

# INTEL no-opt
echo INTEL no-opt
icc -O0 pollen-serial.c -o icc-serial.out
time ./icc-serial.out
echo '-------'