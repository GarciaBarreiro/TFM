
[![pipeline status Master](https://gitlab.citius.usc.es/lidar/rule-based-classifier-cpp/badges/master/pipeline.svg)](https://gitlab.citius.usc.es/lidar/rule-based-classifier-cpp/commits/master)
[![pipeline status Dev](https://gitlab.citius.usc.es/lidar/rule-based-classifier-cpp/badges/dev/pipeline.svg)](https://gitlab.citius.usc.es/lidar/rule-based-classifier-cpp/commits/dev)

# TFM

## Background

LiDAR (Light and Ranging Detection) technology has now become the quintessential technique for collecting geospatial 
data from the earth's surface. This code implements a method for point cloud partitioning based on distributed memory
and MPI (Message Passing Interface) technology.

Original project: https://gitlab.citius.usc.es/lidar/rule-based-classifier.
		
## Install

#### Cloning the project
```bash
git clone https://github.com/GarciaBarreiro/TFM.git
cd TFM
```

#### Dependencies
- Eigen, Armadillo (requires Armadillo 14 or greater. If it isn't available in the repositories, skip that package) and OpenMPI
  - Ubuntu
      ```bash
      sudo apt install libeigen3-dev libarmadillo-dev openmpi-bin openmpi-common openssh-client openssh-server libopenmpi1.3 libopenmpi-dbg libopenmpi-dev
      ```
  - ArchLinux
      ```bash
      sudo pacman -S eigen openmpi
      git clone https://aur.archlinux.org/armadillo.git lib/armadillo
      (cd armadillo && makepkg -si --noconfirm)
      ```

If using Environment Modules, load available required dependencies. For example, in the supercomputer FinisTerrae III, the next command should be executed:
```bash
module load cesga/2020 gcc openmpi/4.0.5_ft3
```
 
The following commands must be executed in the root folder of the project.

- LASTools:
    ```bash
    wget https://lastools.github.io/download/LAStools_221128.zip && unzip LAStools_221128.zip -d ./lib && rm LAStools_221128.zip
    ```
- LASlib:
    ```bash
    (cd lib/LAStools && cmake . && make)
    ```
- MathGeoLib:
    ```bash
    (cd lib && git clone https://github.com/juj/MathGeoLib && cd MathGeoLib && cmake -DCMAKE_BUILD_TYPE=Release . && make)
    ```
- Range-v3:
    ```bash
    (cd lib && git clone https://github.com/ericniebler/range-v3 && cd range-v3 && cmake -DCMAKE_BUILD_TYPE=Release . && make)
    ```
- If Armadillo version available in the repositories isn't 14.0 or greater:
    ```bash
    (cd lib && wget https://sourceforge.net/projects/arma/files/armadillo-14.4.2.tar.xz && tar -xJf armadillo-14.4.2.tar.xz && rm -r armadillo-14.4.2.tar.xz && mv armadillo-14.4.2/ armadillo/ && cd armadillo/ && cmake . && make)
    ```

## Usage

There is one way to compile the project:

##### CMakeLists

In the project directory, just execute
  ```bash
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release ..
  make
  ```

This creates the executable `build/tfm`.

#### Execution

If using slurm, create the following script:
```bash
#!/bin/bash
#SBATCH -t 00:20:00         # Run time (hh:mm:ss) - 20 min
#SBATCH --mem-per-cpu=4G    # Memory per core demanded

srun ${path_to_binary_executable} -i data/ptR_18C.las -r 2
```

And then send the job with:
```bash
sbatch -n ${MPI_nodes} -c ${threads_per_node} ${script_name}
```

If using mpiexec:
```bash
mpiexec ${path_to_binary_executable} -i data/ptR_18C.las -r search_radius [-o output_dir]
```

## Authorship
Grupo de Arquitectura de Computadores (GAC)  
Centro Singular de Investigación en Tecnologías Inteligentes (CiTIUS)  
Universidad de Santiago de Compostela (USC)  

Maintainers: 
- Miguel Yermo García ([miguel.yermo@usc.es](mailto:miguel.yermo@usc.es))
- Silvia Rodríguez Alcaraz ([silvia.alcaraz@usc.es](mailto:silvia.alcaraz@usc.es))
