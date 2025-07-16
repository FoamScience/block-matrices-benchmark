Time &runTime = *timePtr;
argList &args = *argsPtr;
FatalError.dontThrowExceptions();
#include "createMesh.H"
volScalarField T(IOobject("T", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::NO_WRITE), mesh);
volScalarField Ts(IOobject("Ts", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::NO_WRITE), mesh);
forAll(T.internalField(), ci) {
  T[ci] = 300 * Foam::sin(2 * mesh.C()[ci].x()) * Foam::sin(mesh.C()[ci].y());
  T[ci] = 200 * Foam::sin(mesh.C()[ci].x()) * Foam::sin(2 * mesh.C()[ci].y());
}
volVectorField U(IOobject("U", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::NO_WRITE), mesh);
dictionary config;
dimensionedScalar DT("DT", dimLength * dimVelocity, 1.0);
config.set("DT", DT);
dimensionedScalar DTs("DTs", dimLength * dimVelocity, 2.0);
config.set("DTs", DTs);
dimensionedScalar alpha("alpha", dimless / dimTime, 5.0);
config.set("alpha", alpha);
#include "createPhi.H"
