## Block matrix benchmarks

Benchmarking different implementations of block matrices for implicitly 
coupled equations:

- OpenCFD OpenFOAM: using trivial ICSFoam and Petsc implementations
- Foam-Extend's block coupled system

How to run the benchmarks:

> [!WARNING]
> Most things are **PARTIALLY** compiled here. This is done for the sole purpose
> of benchmarking block matrix implementations, and hence not suitable for
> production use.


```bash
#---- SET UP
export FOAM_INSTALL_PATH=~/OpenFOAM/openfoam-v2112
export REPO_ROOT=/tmp/bmb
git clone https://github.com/FoamScience/block-matrices-benchmark $REPO_ROOT
export FOAM_FOAMUT=/tmp/ut
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
ln -s $REPO_ROOT/tests $FOAM_FOAMUT/tests/blockMatrices
cp -r $REPO_ROOT/cases $FOAM_FOAMUT/

#---- GET OpenFOAM, patch and compile the libs (until turbulenceModels is fine)
git clone https://develop.openfoam.com/Development/openfoam $FOAM_INSTALL_PATH
cd ~/OpenFOAM/openfoam-v2112/
git checkout OpenFOAM-v2112
git am -3 $REPO_ROOT/patches/openfoam/0001-fix-adapt-patch-classes-for-use-with-ICSFoam.patch
source etc/bashrc
./src/Allwmake -j
#---- Compile ICSFoam
git clone https://github.com/stefanoOliani/ICSFoam $REPO_ROOT/ICSFoam
cd $REPO_ROOT/ICSFoam
wmake libso src
#---- Compile parts of RheoTool
git clone https://github.com/fppimenta/rheoTool $REPO_ROOT/rheotool
cd $REPO_ROOT/rheotool
# checkout commit 81f3e8c if this doesn't work
git am -3 $REPO_ROOT/patches/rheotool/0001-build-update-versions.patch
git am -3 $REPO_ROOT/patches/rheotool/0002-fix-port-sparseMatrixSolvers-to-ESI.patch
./downloadEigen
./installPetsc
export PETSC_DIR=$WM_PROJECT_DIR/ThirdParty/petsc-3.19.6
export PETSC_ARCH=arch-linux-c-opt
export LD_LIBRARY_PATH=${PETSC_DIR}/${PETSC_ARCH}/lib/:$LD_LIBRARY_PATH
wmake libso of90/src/libs/fvmb
wmake libso of90/src/libs/sparseMatrixSolvers

#---- THE BENCHMARKS With OpenCFD's OpenFOAM
cd $FOAM_FOAMUT
wclean tests/blockMatrices
./Alltest --no-parallel --benchmark-samples 20  # Runs ICSFoam tests + RheoTool
#---- THE BENCHMARKS With Foam-Extend
source <path/to/foam/extend/5.0/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel --benchmark-samples 20  # Runs Extend tests
```

Benchmark results are presented as the following:
```
benchmark name                       samples       iterations    est run time
                                     mean          low mean      high mean
                                     std dev       low std dev   high std dev

                                     149.339 ms    147.605 ms    151.148 ms 
                                     4.0121 ms     2.59951 ms    6.26408 ms
```
What we care about is the mean execution time for a single matrix solution run.

## Important notes

- This repository benchmarks the performance of different block matrix
  implementations on a sample problem of coupling source term in a scalar
  transport.
- The initial field state is set using sine waves. 
- More things to test may include:
  1. Parallel scalability: Number of processors needs to be changed in case and Alltest script
  1. Different operator schemes
  1. Different linear solvers
