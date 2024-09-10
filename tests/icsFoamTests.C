#include "implicitSourceCoupling.H"
#include "catch2/catch_all.hpp"
#include "catch2/catch_test_macros.hpp"
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
  dimensionedScalar DT("DT", dimLength * dimVelocity, 1.0);
  config.set("DT", DT);
  dimensionedScalar DTs("DTs", dimLength * dimVelocity, 2.0);
  config.set("DTs", DTs);
  dimensionedScalar alpha("alpha", dimless / dimTime, 10.0);
  config.set("alpha", alpha);

#include "createPhi.H"

  BENCHMARK("Native implementations of coupled equations") {
    return implicitSourceCoupling::solveCoupledEqns(mesh, config);
  };


//#if defined(OPENFOAM)
//  if (!Pstream::parRun()) {
//    MPI_Init(NULL, NULL);
//  }
//
//  BENCHMARK("PetSc implementation of coupled equations") {
//    return implicitSourceCoupling::solvePetScEqns(mesh, config);
//  };
//
//  if (!Pstream::parRun()) {
//    MPI_Finalize();
//  }
//#endif

  runTime.setTime(0.0, 0);
}
