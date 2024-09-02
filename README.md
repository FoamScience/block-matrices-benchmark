## Block matrix benchmarks

Benchmarking different implementations of block matrices for implicitly 
coupled equations:

- OpenCFD OpenFOAM: using trivial ICSFoam and Petsc implementations
- Foam-Extend's block coupled system

How to run the benchmarks:
```bash
git clone https://github.com/FoamScience/foamUT /tmp/ut
export FOAM_FOAMUT=/tmp/ut
export REPO_ROOT=$(git rev-parse --show-toplevel)
rm -rf $FOAM_FOAMUT/tests/exampleTests $FOAM_FOAMUT/cases
ln -s $PWD/tests $FOAM_FOAMUT/tests/blockMatrices
cp -r $PWD/cases $FOAM_FOAMUT/
cd $FOAM_FOAMUT
source <your/favorite/etc/bashrc>
wclean tests/blockMatrices # to try another openfoam fork
./Alltest --no-parallel
source <Another/etc/bashrc>
wclean tests/blockMatrices
./Alltest --no-parallel # run benchmarks for new OpenFOAM fork
```

## Important notes

- This repository benchmarks the performance of different block matrix
  implementations on a sample problem of coupling source term in a scalar
  transport.
- The initial field state is set using sine waves. 
- More things to test may include:
  1. Parallel scalability: Number of processors needs to be changed in case and Alltest script
  1. Different operator schemes
  1. Different linear solvers
