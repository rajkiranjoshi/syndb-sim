# SyNDB Simulator

This repository hosts the packet-level simulator used in the evaluation of the NSDI 2021 paper titled [Debugging Transient Faults in Data Centers using Synchronized Network-wide Packet Histories](https://www.usenix.org/conference/nsdi21/presentation/kannan). This packet-level simulator is used to evaluate the proposed system, **SyNDB**, on a large-scale data center network.

## Compile

### Install Dependencies

The following *additional* C++ libraries are required:
* [libfmt-6.1.2](https://github.com/fmtlib/fmt/releases/tag/6.1.2)
* [libspdlog-1.8.2](https://github.com/gabime/spdlog/releases/tag/v1.8.2)

The simulator has been tested with the above library versions. You can always use higher versions. However, to avoid any possibility of running into API breakage, we recommend you to stick to the above versions. Other than these, the simulator uses the standard `libpthread` library.

Download the source code for the above dependencies. Build and install them as per respective instructions:
* [Building and installing libfmt](https://fmt.dev/latest/usage.html#building-the-library) (build a shared library)
* [Building and installing libspdlog](https://github.com/gabime/spdlog#install)

### Compilation

**Step 1:** Git clone this repo to your local Linux machine:
```
git clone https://github.com/rajkiranjoshi/syndb-sim.git
```

**Step 2:** Compile the simulator source code:
```
# Inside the syndb-sim directory
make
```
This will create the simulator binary `syndb-sim` in the same directory.

## Run
```
# Inside the syndb-sim directory
./syndb-sim
```
This will run the simulation as described in section 7.1.2 of the [paper](https://www.usenix.org/system/files/nsdi21-kannan.pdf). By default, the simulator binary will simulate a small k=4 fat tree network with 100G links for a total network run time of 10 milliseconds. See [Advanced Simulation](#advanced-simulation) to run simulation for other (large) configurations.

The simulation will produce a bunch of output files in the "data" subdirectory.

## Analyze

The simulation data is analyzed by a separate binary. 

### Build the data analysis binary 

Update the `PREFIX_FILE_PATH` and `PREFIX_STRING_FOR_DATA_FILES` macros in the `data-analysis/dataparser.hpp` to point to the output data and its (time-based) prefix. Now build the data analysis binary:
```
# Inside the syndb-sim directory
make analysis
```
This will produce the binary `syndb-analysis` in the same directory.

Finally, copy `preProcessData.sh` to the data folder and run it to process the simulation data.

```
# Inside the syndb-sim directory
cp ./preProcessData.sh ./path/to/data/file
cd ./path/to/data/file
./preProcessData.sh
```

### Analyze the simulation data
```
# Inside the syndb-sim directory
./syndb-analysis
```
This will run analysis on the data generated by the simulation. The analysis will print the following for each valid switch:
* ingress time for the trigger packet
* maximum number of common packets between the trigger switch and current switch
* correlation for the captured *p-records*
* common time window between the trigger switch but missing in the current switch window

## Advanced Simulation

The [paper](https://www.usenix.org/system/files/nsdi21-kannan.pdf) uses this simulator for: (a) validation (section 7, Figure 6), and (b) large-scale evaluation (section 7.1.2). To build the simulation/analysis binary for these experiment configurations, build the simulation binary with the following commands, respectively:
```
## For validation, inside the syndb-sim directory
make CONFIG=validation
# After simulation and updating macros in dataparser.hpp
make CONFIG=validation analysis

## For evaluation, inside the syndb-sim directory
make CONFIG=evaluation
# After simulation and updating macros in dataparser.hpp
make CONFIG=evaluation analysis
```
For the evaluation configuration, each simulation run requires ~4 GB of memory and produces 100's of GB of simulation data while requiring ~10 hours (depending on your CPU clock speed) to complete. Therefore, please ensure sufficient system requirements before running the simulator with the evaluation configuration.
