#include "catch2/catch_all.hpp"
#include "catch2/catch_test_macros.hpp"
#include "implicitSourceCoupling.H"
#include <mpi.h>

using namespace Foam;
extern Time *timePtr;
extern argList *argsPtr;

TEST_CASE("Implicitly coupled source term gets resolved",
          "[theCase][serial][parallel]") {
  FatalError.dontThrowExceptions();
  Time &runTime = *timePtr;
  argList &args = *argsPtr;
#include "createMesh.H"
  volScalarField T(IOobject("T", runTime.timeName(), mesh, IOobject::MUST_READ,
                            IOobject::NO_WRITE),
                   mesh);
  volScalarField Ts(IOobject("Ts", runTime.timeName(), mesh,
                             IOobject::MUST_READ, IOobject::NO_WRITE),
                    mesh);
  forAll(T.internalField(), ci) {
    T[ci] = 300 * Foam::sin(2 * mesh.C()[ci].x()) * Foam::sin(mesh.C()[ci].y());
    T[ci] = 200 * Foam::sin(mesh.C()[ci].x()) * Foam::sin(2 * mesh.C()[ci].y());
  }
  volVectorField U(IOobject("U", runTime.timeName(), mesh, IOobject::MUST_READ,
                            IOobject::NO_WRITE),
                   mesh);
  dictionary config;
  auto dtVal = GENERATE(0.0, 1.0, 50.0);
  dimensionedScalar DT("DT", dimLength * dimVelocity, dtVal);
  config.set("DT", DT);
  auto dtsVal = GENERATE(2.0, 30.0);
  dimensionedScalar DTs("DTs", dimLength * dimVelocity, dtsVal);
  config.set("DTs", DTs);
  auto alphaVal = GENERATE(2.0, 10.0);
  dimensionedScalar alpha("alpha", dimless / dimTime, alphaVal);
  config.set("alpha", alpha);
  auto uVal = GENERATE(0.0, 2.0, 5.0);
  forAll(U.internalField(), ci) { U[ci] = vector(uVal, 0, 0); }
  CAPTURE(dtVal, dtsVal, alphaVal, uVal);
#include "createPhi.H"

  BENCHMARK("Native implementations of coupled equations") {
    return implicitSourceCoupling::solveCoupledEqns(mesh, config);
  };


#if defined(OPENFOAM)
  if (!Pstream::parRun()) {
    MPI_Init(NULL, NULL);
  }
  BENCHMARK("PetSc implementation of coupled equations") {
    return implicitSourceCoupling::solvePetScEqns(mesh, config);
  };

  if (!Pstream::parRun()) {
    MPI_Finalize();
  }
#endif

  runTime.setTime(0.0, 0);
}
