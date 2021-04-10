# SyNDB Simulator

This repository hosts the packet-level simulator used in the evaluation of the NSDI 2021 paper titled [Debugging Transient Faults in Data Centers using Synchronized Network-wide Packet Histories](https://www.usenix.org/conference/nsdi21/presentation/kannan). This packet-level simulator evaluates the proposed system, **SyNDB**, on a large-scale datacenter network.

## Install

### Dependencies

For compiling and running the simulator, the following *additional* C++ libraries are required:
* [libfmt-6.1.2](https://github.com/fmtlib/fmt/releases/tag/6.1.2)
* [libspdlog-1.8.2](https://github.com/gabime/spdlog/releases/tag/v1.8.2)

The library versions are the ones with which the simulator has been tested. You can always use higher versions. However, to avoid any possibility of running into API breakage, we recommend you to stick to the above version numbers. Other than these, the simulator uses the standard `libpthread` library.

### Compilation

**Step 0:** Download the source code for the above dependencies. Build and install them as per respective instructions:
* [Building and installing libfmt](https://fmt.dev/latest/usage.html#building-the-library)
* [Building and installing libspdlog](https://github.com/gabime/spdlog#install)

**Step 1:** Git clone this repo to your local Linux machine.

**Step 2:** Compile the simulator source code by running `make` inside the syndb-sim directory. This will create the simulator binary `syndb-sim` in the same directory.

## Run
By default, the simulator binary will simulate a small k=4 fat tree network with 100G links for a total network time of 1 second. See [Advanced Simulation](#advanced-simulation) to run simulation for other configurations.

To run the simulator, simply execute the binary `syndb-sim` in the syndb-sim directory. It will run the simulation as described in section 7.1.2 of the [paper](https://www.usenix.org/conference/nsdi21/presentation/kannan), just on a smaller k=4 fat-tree topology.

All the simulation data is dumped inside the subdirectory "data".

## Analyze

The analysis on the simulation data is performed by a separate binary. 

### Build the data analysis binary 
```
# Inside the syndb-sim directory
make analysis
```
This will produce a binary `syndb-analysis` that would be used for analyzing the simulation data.

### Analyze the simulation data

TODO: Nishant to add to this section.


## Advanced Simulation

TODO: Raj to add about running full scale simulations and reproducing results from the paper. 




