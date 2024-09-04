## Block matrix benchmarks

Benchmarking different implementations of block matrices for implicitly 
coupled equations:

- OpenCFD OpenFOAM: using trivial ICSFoam and Petsc implementations
- Foam-Extend's block coupled system

## Run the benchmarks

> [!WARNING]
> Most things are **PARTIALLY** compiled here. This is done for the sole purpose
> of benchmarking block matrix implementations, and hence not suitable for
> production use.

### [Recommended] Use the provided Apptainer container

```bash
# Install a recent Apptainer version, eg. on Ubuntu:
wget https://github.com/apptainer/apptainer/releases/download/v1.3.3/apptainer_1.3.3_amd64.deb
sudo apt install ./apptainer_1.3.3_amd64.deb
# Get the container [in a temporary location]
cd /tmp
apptainer pull bmb.sif oras://ghcr.io/foamscience/block-matrices-benchmark
# Setup foamUT for executing the benchmarks
export FOAM_FOAMUT=/tmp/ut
export CATCH_TIMEOUT=300
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT
cd $FOAM_FOAMUT
# What comes next is exactly the same as if things were installed locally
# notice in container commands, skip the $ sign
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
apptainer run /tmp/bmb.sif "cp -r \$REPO_ROOT/tests \$FOAM_FOAMUT/tests/blockMatrices"
apptainer run /tmp/bmb.sif "cp -r \$REPO_ROOT/cases \$FOAM_FOAMUT/cases"
# Run the benchmarks, currently there is no parallel support (ICSFoam)
apptainer run /tmp/bmb.sif "wclean tests/blockMatrices; ./Alltest --no-parallel --benchmark-samples 20"
```

For Foam-Extend, there is no customization needed, hence you can always run this
on your own machine, use any Foam-Extend container:
```bash
#---- THE BENCHMARKS With Foam-Extend (in another terminal)
source <path/to/foam/extend/5.0/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel --benchmark-samples 20  # Runs Extend tests
```

### Use your local machine

Making this work on your machine is a little bit involved as you have to patch OpenFOAM (v2112) and RheoTool.
The git patch files are tracked in the [patches](pactches)
folder. For an example on how to install/compile the needed libraries, you can look at the
[definition file](build/container.def) of the apptainer container.

A few differences are recommended though from the container environment:

```bash
export FOAM_INSTALL_PATH=~/OpenFOAM/openfoam-v2112
# ----- Git clone, patch and compile OpenFOAM as in the definition file
# ----- source $FOAM_INSTALL_PATH/etc/bashrc
# ----- Git clone and compile relevant libs from ICSFoam
# ----- Git clone and patch rheotool
# ----- Install Petsc and Eigen, compile relevant libs of rheotool
# ----- Set your environment as show in %environment section of the definition file

# Setup foamUT and run benchmarks
export FOAM_FOAMUT=/tmp/ut
export CATCH_TIMEOUT=300
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
ln -s $REPO_ROOT/tests $FOAM_FOAMUT/tests/blockMatrices
ln -s $REPO_ROOT/cases $FOAM_FOAMUT/cases

#---- THE BENCHMARKS With OpenCFD's OpenFOAM
cd $FOAM_FOAMUT
wclean tests/blockMatrices
./Alltest --no-parallel --benchmark-samples 20  # Runs ICSFoam tests + RheoTool

#---- THE BENCHMARKS With Foam-Extend (in another terminal)
source <path/to/foam/extend/5.0/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel --benchmark-samples 20  # Runs Extend tests
```

## Benchmark results

```
benchmark name                       samples       iterations    est run time
                                     mean          low mean      high mean
                                     std dev       low std dev   high std dev

Native implementations of coupled 
equations (ESI/ICSFoam)
                                     ***337.773 ms    335.628 ms    341.222 ms 
                                     6.15132 ms    4.08663 ms    8.66035 ms

PetSc implementation of coupled                                                
equations
                                     **149.339 ms    147.605 ms    151.148 ms 
                                     4.0121 ms     2.59951 ms    6.26408 ms

Native implementations of coupled 
equations (Foam-Extend 5.0)
                                     *70.4255 ms    70.3079 ms    70.5685 ms 
                                     295.163 us    217.929 us    388.545 us
```
What we care about is the mean execution time for a single matrix solution run.
Foam-Extend proves to be the best performing.

> These results were obtained on a machine with:
> ```
> kernel: 6.8.0-101041-tuxedo
> cpu: 8-core AMD Ryzen 7 5800H with 4MiB L2 and 16MiB L3 caches - max speed of 4463 MHz
> ```

## Detailed description

Two simple transport equations are coupled through their source term (for simpler implementations).
The benchmark runs `.solve()` method on the coupled system of equations a number of times (20 as suggested
above, but this is a user choice)  and records the average wall time taken to come out of the solve function.

> Currently, there is no check on the correctness of the results mainly it's hard to compare results from
> two OpenFOAM forks within a single unit-test.

The setup of the coupled system is identical throughout all tests, and all tests are ran on the same OpenFOAM
case, reactively called [theCase](cases/theCase). All implementations use GMRES with identical settings since
it's the only one available on all benchmarked frameworks.

The `T` and `Ts` fields (to be transported) are initialized a sine wave.

Also, to avoid hyper verbosity of linear solvers, the `Info` streams are set in failing state before each test.

## Next steps

1. Parallel scalability
1. Compare best-performing linear solvers from each framework?
