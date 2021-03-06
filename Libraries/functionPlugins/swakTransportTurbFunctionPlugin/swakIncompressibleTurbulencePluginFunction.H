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

Class
    Foam::swakIncompressibleTurbulencePluginFunction

Description

SourceFiles
    swakIncompressibleTurbulencePluginFunction.C

Contributors/Copyright:
    2012-2013, 2015-2016, 2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>

 SWAK Revision: $Id$
\*---------------------------------------------------------------------------*/

#ifndef swakIncompressibleTurbulencePluginFunction_H
#define swakIncompressibleTurbulencePluginFunction_H

#include "swakTransportModelsPluginFunction.H"

#include "swakTurbulence.H"

#ifdef FOAM_NEW_TURBULENCE_STRUCTURE
#include "singlePhaseTransportModel.H"
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
#include "incompressibleMomentumTransportModel.H"
#include "singlePhaseTransportModel.H"
#include "kinematicMomentumTransportModel.H"
#else
#include "turbulentTransportModel.H"
#endif
#else
#include "turbulenceModel.H"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class swakIncompressibleTurbulencePluginFunction Declaration
\*---------------------------------------------------------------------------*/
class swakIncompressibleTurbulencePluginFunction
:
    public swakTransportModelsPluginFunction
{
    //- Disallow default bitwise assignment
    void operator=(const swakIncompressibleTurbulencePluginFunction &);

    swakIncompressibleTurbulencePluginFunction(const swakIncompressibleTurbulencePluginFunction &);

protected:
    static const
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
    incompressible::momentumTransportModel
#else
    incompressible::turbulenceModel
#endif
    &turbInternal(const fvMesh &reg);

    const
#ifdef FOAM_HAS_MOMENTUM_TRANSPORT_MODELS
    incompressible::momentumTransportModel
#else
    incompressible::turbulenceModel
#endif
    &turb();

  public:
    swakIncompressibleTurbulencePluginFunction(
        const FieldValueExpressionDriver &parentDriver,
        const word &name,
        const word &returnValueType
    );

    virtual ~swakIncompressibleTurbulencePluginFunction() {}

    TypeName("swakIncompressibleTurbulencePluginFunction");

};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
