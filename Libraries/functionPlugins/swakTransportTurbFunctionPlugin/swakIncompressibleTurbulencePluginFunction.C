/*---------------------------------------------------------------------------*\
|                       _    _  _     ___                       | The         |
|     _____      ____ _| | _| || |   / __\__   __ _ _ __ ___    | Swiss       |
|    / __\ \ /\ / / _` | |/ / || |_ / _\/ _ \ / _` | '_ ` _ \   | Army        |
|    \__ \\ V  V / (_| |   <|__   _/ / | (_) | (_| | | | | | |  | Knife       |
|    |___/ \_/\_/ \__,_|_|\_\  |_| \/   \___/ \__,_|_| |_| |_|  | For         |
|                                                               | OpenFOAM    |
-------------------------------------------------------------------------------
License
    This file is part of swak4Foam.

    swak4Foam is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    swak4Foam is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with swak4Foam; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Contributors/Copyright:
    2012-2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>

 SWAK Revision: $Id$
\*---------------------------------------------------------------------------*/

#include "swakIncompressibleTurbulencePluginFunction.H"
#include "FieldValueExpressionDriver.H"

#include "HashPtrTable.H"
#include "LESModel.H"
#include "RASModel.H"

#include "addToRunTimeSelectionTable.H"

namespace Foam {

defineTypeNameAndDebug(swakIncompressibleTurbulencePluginFunction,0);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

swakIncompressibleTurbulencePluginFunction::swakIncompressibleTurbulencePluginFunction(
    const FieldValueExpressionDriver &parentDriver,
    const word &name,
    const word &returnValueType
):
    swakTransportModelsPluginFunction(
        parentDriver,
        name,
        returnValueType
    )
{
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
incompressible::momentumTransportModel
#else
incompressible::turbulenceModel
#endif
&swakIncompressibleTurbulencePluginFunction::turbInternal(
    const fvMesh &reg
)
{
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
    typedef incompressible::momentumTransportModel incompressibleTurbulenceModel;
#else
    typedef incompressible::turbulenceModel incompressibleTurbulenceModel;
#endif
    static HashPtrTable<incompressibleTurbulenceModel> turb_;

    if(reg.foundObject<incompressibleTurbulenceModel>("turbulenceProperties")) {
        if(debug) {
            Info << "swakIncompressibleTurbulencePluginFunction::turbInternal: "
                << "turbulence model already in memory" << endl;
        }
        // Somebody else already registered this
        return reg.lookupObject<incompressibleTurbulenceModel>("turbulenceProperties");
    }
    if(reg.foundObject<incompressible::LESModel>("LESProperties")) {
        if(debug) {
            Info << "swakIncompressibleTurbulencePluginFunction::turbInternal: "
                << "LES already in memory" << endl;
        }
        // Somebody else already registered this
        return reg.lookupObject<incompressible::LESModel>("LESProperties");
    }
    if(reg.foundObject<incompressible::RASModel>("RASProperties")) {
        if(debug) {
            Info << "swakIncompressibleTurbulencePluginFunction::turbInternal: "
                << "RAS already in memory" << endl;
        }
        // Somebody else already registered this
        return reg.lookupObject<incompressible::RASModel>("RASProperties");
    }

    if(!turb_.found(reg.name())) {
        if(debug) {
            Info << "swakIncompressibleTurbulencePluginFunction::turbInternal: "
                << "not yet in memory for " << reg.name() << endl;
        }

        turb_.set(
            reg.name(),
            incompressibleTurbulenceModel::New(
                reg.lookupObject<volVectorField>("U"),
                reg.lookupObject<surfaceScalarField>("phi"),
                const_cast<transportModel &>(transportInternal(reg))
            ).ptr()
        );
    }

    return *(turb_[reg.name()]);
}

const
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
incompressible::momentumTransportModel
#else
incompressible::turbulenceModel
#endif
&swakIncompressibleTurbulencePluginFunction::turb()
{
    return turbInternal(mesh());
}

// * * * * * * * * * * * * * * * Concrete implementations * * * * * * * * * //

#define concreteTurbFunction(funcName,resultType)                  \
class swakIncompressibleTurbulencePluginFunction_ ## funcName      \
: public swakIncompressibleTurbulencePluginFunction                \
{                                                                  \
public:                                                            \
    TypeName("swakIncompressibleTurbulencePluginFunction_" #funcName);       \
    swakIncompressibleTurbulencePluginFunction_ ## funcName (           \
        const FieldValueExpressionDriver &parentDriver,                 \
        const word &name                                                \
    ): swakIncompressibleTurbulencePluginFunction(                      \
        parentDriver,                                                   \
        name,                                                           \
        #resultType                                                     \
    ) {}                                                                \
        void doEvaluation() {                                           \
        autoPtr<resultType> rField(                                     \
            new resultType(                                             \
                turb().funcName()                                       \
            )                                                           \
        );                                                              \
        rField->correctBoundaryConditions();                            \
        result().setObjectResult(                                       \
            rField                                                      \
        );                                                              \
    }                                                                   \
};                                                                      \
defineTypeNameAndDebug(swakIncompressibleTurbulencePluginFunction_ ## funcName,0);  \
addNamedToRunTimeSelectionTable(FieldValuePluginFunction,swakIncompressibleTurbulencePluginFunction_ ## funcName,name,turb_ ## funcName);

#define movedConcreteTurbFunction(funcName, resultType)                        \
  class swakIncompressibleTurbulencePluginFunction_##funcName                  \
      : public swakIncompressibleTurbulencePluginFunction {                    \
  public:                                                                      \
    TypeName("swakIncompressibleTurbulencePluginFunction_" #funcName);         \
    swakIncompressibleTurbulencePluginFunction_##funcName(                     \
        const FieldValueExpressionDriver &parentDriver, const word &name)      \
        : swakIncompressibleTurbulencePluginFunction(parentDriver, name,       \
                                                     #resultType) {}           \
    void doEvaluation() {                                                      \
      FatalErrorInFunction                                                     \
          << "Function " #funcName                                             \
             " no longer part of the turbulence model in this Foam-version"    \
          << endl                                                              \
          << exit(FatalError);                                                 \
    }                                                                          \
  };                                                                           \
  defineTypeNameAndDebug(                                                      \
      swakIncompressibleTurbulencePluginFunction_##funcName, 0);               \
  addNamedToRunTimeSelectionTable(                                             \
      FieldValuePluginFunction,                                                \
      swakIncompressibleTurbulencePluginFunction_##funcName, name,             \
      turb_##funcName);

concreteTurbFunction(nut,volScalarField);
concreteTurbFunction(nuEff,volScalarField);
concreteTurbFunction(k,volScalarField);
concreteTurbFunction(epsilon,volScalarField);

#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
movedConcreteTurbFunction(R, volSymmTensorField);
movedConcreteTurbFunction(devReff, volSymmTensorField);
#else
concreteTurbFunction(R, volSymmTensorField);
concreteTurbFunction(devReff, volSymmTensorField);
#endif

} // namespace

// ************************************************************************* //
