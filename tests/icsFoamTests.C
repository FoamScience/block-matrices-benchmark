#include "implicitSourceCoupling.H"
#include "catch2/catch_all.hpp"
#include "catch2/catch_test_macros.hpp"
#include <mpi.h>

using namespace Foam;
extern Time *timePtr;
extern argList *argsPtr;

const int N_PARALLEL_RUNS = 10;

TEST_CASE("Native implementation of coupled equations",
          "[theCase][serial][parallel]") {
  #include "testSetup.H"
  for(int i=0; i<N_PARALLEL_RUNS; i++) {
    volatile bool out = implicitSourceCoupling::solveCoupledEqns(mesh, config);
    CHECK(out);
  }
  Info().stdStream().clear();
  runTime.setTime(0.0, 0);
}

#if defined(OPENFOAM)

TEST_CASE("PetSc implementation of coupled equations",
          "[theCase][serial][parallel]") {
  #include "testSetup.H"
  if (!Pstream::parRun()) MPI_Init(NULL, NULL);
  for(int i=0; i<N_PARALLEL_RUNS; i++) {
    volatile bool out = implicitSourceCoupling::solvePetScEqns(mesh, config);
    CHECK(out);
  }
  if (!Pstream::parRun()) MPI_Finalize();
  Info().stdStream().clear();
  runTime.setTime(0.0, 0);
}
#endif
