## Block matrix benchmarks

> [!CAUTION]
> No result "validity" checks are conducted, for now, I just have "good faith"
> that all implementations will give roughly the same solution. The benchmarked 
> coupled system is set to diverge so each linear solver works as much as it can.

Benchmarking different implementations of block matrices for implicitly 
coupled equations:

- OpenCFD OpenFOAM: using trivial ICSFoam and Petsc implementations
- Foam-Extend's block coupled system

## Run the benchmarks

> [!WARNING]
> Most things are **PARTIALLY** compiled here. This is done for the sole purpose
> of benchmarking block matrix implementations, hence probably not suitable for
> production use as-is.

### [Recommended] Use the provided Apptainer containers

```bash
# Get the container [in a temporary location]
cd /tmp
apptainer pull bmb.sif oras://ghcr.io/foamscience/block-matrices-benchmark.sif:latest
# Setup foamUT for executing the benchmarks
export FOAM_FOAMUT=/tmp/ut
export CATCH_TIMEOUT=1000
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT
cd $FOAM_FOAMUT
# What comes next is exactly the same as if things were installed locally
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
export PETSC_OPTIONS="-use_gpu_aware_mpi 0"
apptainer run /tmp/bmb.sif 'cp -r $REPO_ROOT/tests $FOAM_FOAMUT/tests/blockMatrices'
apptainer run /tmp/bmb.sif 'cp -r $REPO_ROOT/cases $FOAM_FOAMUT/cases'
# Run the benchmarks, eg. in serial:
apptainer run --nv /tmp/bmb.sif 'wclean tests/blockMatrices; ./Alltest --no-parallel -d yes'
# For more info check scripts/weak_scaling.py:
cd scripts
uv sync
uv run weak_scaling --help
```

For Foam-Extend, there is no customization needed, hence you can always run this
on your own machine, use any Foam-Extend container:
```bash
#---- THE BENCHMARKS With Foam-Extend
source <path/to/foam/extend/5.0/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel -d yes # Runs Extend tests in serial
```

### Use your local machine

Making this work on your machine is a little bit involved as you have to patch OpenFOAM (v2112) and RheoTool.
The git patch files are tracked in the [patches](patches)
folder.

For an example on how to install/compile the needed libraries, you can look at the
[definition file](build/container.def) of the apptainer container.

A few differences from the container environment may arise:

```bash
export FOAM_INSTALL_PATH=~/OpenFOAM/openfoam-v2112
# ----- Git clone, patch and compile OpenFOAM as in the definition file
# ----- source $FOAM_INSTALL_PATH/etc/bashrc
# ----- Git clone (optionally patch) and compile relevant libs from ICSFoam
# ----- Git clone and patch rheotool
# ----- Install Petsc and Eigen, compile relevant libs of rheotool
# ----- If interested in conducting performance measurements, install HPCtoolkit
# ----- Set your environment as show in %environment section of the definition file

# Setup foamUT and run the benchmarks
export FOAM_FOAMUT=/tmp/ut
export CATCH_TIMEOUT=300
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
ln -s $REPO_ROOT/tests $FOAM_FOAMUT/tests/blockMatrices
ln -s $REPO_ROOT/cases $FOAM_FOAMUT/cases

#---- THE BENCHMARKS With OpenCFD's OpenFOAM
cd $FOAM_FOAMUT
wclean tests/blockMatrices
./Alltest --no-parallel -d yes  # Runs ICSFoam tests + RheoTool

#---- THE BENCHMARKS With Foam-Extend (in a separate shell)
source <path/to/foam/extend/5.0/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel -d yes  # Runs Extend tests
```

## Detailed description

Two simple transport equations are coupled through their source term (for simpler implementations).
The benchmark runs `.solve()` method on the coupled system of equations a number of times and records
the total wall time taken to come out of the solve function.

The setup of the coupled system is identical throughout all tests, and all tests are ran on the same OpenFOAM
case, creatively called [theCase](cases/theCase). All implementations use GMRES with identical settings since
it's the only one available on all benchmarked frameworks, but preconditioners and smoother may differ.

The `T` and `Ts` fields (to be transported) are initialized with a sine wave.

Also, to avoid hyper verbosity of linear solvers, the `Info` streams are set in failing state before each test.
