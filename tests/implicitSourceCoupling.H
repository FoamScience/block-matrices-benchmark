#ifndef IMPLICIT_SOURCE_COUPLING_H
#define IMPLICIT_SOURCE_COUPLING_H

#if defined(OPENFOAM)
#include "coupledMatrix.H"
#include "coupledSolver.H"
#include "fvCFD.H"
#else
#include "vector2.H"
#include "fvCFD.H"
#include "surfaceInterpolationScheme.H"
#include "fvBlockMatrix.H"
#endif

struct implicitSourceCoupling {
  static bool solveCoupledEqns(const fvMesh &mesh, const dictionary &config) {
    Info().stdStream().setstate(std::ios_base::failbit);
    volScalarField &T =
        const_cast<volScalarField &>(mesh.lookupObject<volScalarField>("T"));
    volScalarField &Ts =
        const_cast<volScalarField &>(mesh.lookupObject<volScalarField>("Ts"));
    surfaceScalarField &phi = const_cast<surfaceScalarField &>(
        mesh.lookupObject<surfaceScalarField>("phi"));
    dimensionedScalar alpha(config.lookup("alpha"));
    dimensionedScalar DT(config.lookup("DT"));
    dimensionedScalar DTs(config.lookup("DTs"));

    fvScalarMatrix Eqn1(fvm::div(phi, T) - fvm::laplacian(DT, T) ==
                        alpha * Ts - fvm::Sp(alpha, T));
    Eqn1.relax();
    fvScalarMatrix Eqn2(-fvm::laplacian(DTs, Ts) ==
                        alpha * T - fvm::Sp(alpha, Ts));
    Eqn2.relax();
#if defined(OPENFOAM)
    fvScalarMatrix coupling(T, T.dimensions());
    coupling.source() = scalarField(mesh.nCells(), -alpha.value());
    coupledMatrix blockM(mesh, 2, 0, false);
    blockM.dSByS(0, 0).insertEquation(Eqn1);
    blockM.dSByS(0, 1).insertEquation(coupling);
    blockM.dSByS(1, 1).insertEquation(Eqn2);
    blockM.dSByS(1, 0).insertEquation(coupling);
    PtrList<volScalarField> flds(2);
    flds.set(0, new volScalarField(
                    IOobject("T", mesh.time().timeName(), mesh,
                             IOobject::NO_READ, IOobject::NO_WRITE, false),
                    mesh, dimensionedScalar("z", dimTemperature, 0.0)));
    flds.set(1, new volScalarField(
                    IOobject("Ts", mesh.time().timeName(), mesh,
                             IOobject::NO_READ, IOobject::NO_WRITE, false),
                    mesh, dimensionedScalar("z", dimTemperature, 0.0)));
    PtrList<volVectorField> vflds;
    blockM.solve(flds, vflds);
#else
    scalarField coupling(mesh.nCells(), -alpha.value());
    volVector2Field blockT(IOobject("blockT", mesh.time().timeName(), mesh,
                                    IOobject::NO_READ, IOobject::NO_WRITE),
                           mesh,
                           dimensionedVector2("zero", dimless, vector2::zero));
    fvBlockMatrix<vector2> blockM(blockT);
    blockM.insertEquation(0, Eqn1);
    blockM.insertEquation(1, Eqn2);
    blockM.insertEquationCoupling(0, 1, coupling);
    blockM.insertEquationCoupling(1, 0, coupling);
    blockM.updateSourceCoupling();
    blockM.solve();
    blockM.retrieveSolution(0, T.internalField());
    blockM.retrieveSolution(1, Ts.internalField());
    T.correctBoundaryConditions();
    Ts.correctBoundaryConditions();
#endif
    Info().stdStream().clear();
    return true;
  }
  static bool solvePetScEqns(const fvMesh &mesh, const dictionary &config) {
#if defined(OPENFOAM)
    Info().stdStream().setstate(std::ios_base::failbit);
    auto csPtr = autoPtr<coupledSolver>(new coupledSolver(
        "T-Ts", mesh.name(), mesh.time(), mesh.solutionDict()));
    csPtr->insertMesh(mesh);
    coupledSolver &cs = csPtr();
    volScalarField &T =
        const_cast<volScalarField &>(mesh.lookupObject<volScalarField>("T"));
    volScalarField &Ts =
        const_cast<volScalarField &>(mesh.lookupObject<volScalarField>("Ts"));
    surfaceScalarField &phi = const_cast<surfaceScalarField &>(
        mesh.lookupObject<surfaceScalarField>("phi"));
    dimensionedScalar alpha(config.lookup("alpha"));
    dimensionedScalar DT(config.lookup("DT"));
    dimensionedScalar DTs(config.lookup("DTs"));

    fvScalarMatrix Eqn1(fvm::div(phi, T) - fvm::laplacian(DT, T) ==
                        alpha * Ts - fvm::Sp(alpha, T));
    Eqn1.relax();
    fvScalarMatrix Eqn2(-fvm::laplacian(DTs, Ts) ==
                        alpha * T - fvm::Sp(alpha, Ts));
    Eqn2.relax();
    cs.insertField(T);
    cs.insertField(Ts);
    fvScalarMatrix cpl(T, T.dimensions());
    cpl.source() = scalarField(mesh.nCells(), -alpha.value());
    cs.insertEquation(T.name(), T.name(), Eqn1);
    cs.insertEquation(T.name(), Ts.name(), cpl);
    cs.insertEquation(Ts.name(), Ts.name(), Eqn2);
    cs.insertEquation(Ts.name(), T.name(), cpl);
    cs.solve();
    Info().stdStream().clear();
    return true;
#else
    return true;
#endif
  }
};

#endif // !IMPLICIT_SOURCE_COUPLING_H
