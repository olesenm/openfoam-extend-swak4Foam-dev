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
    SetDeltaTWithInterpreterFunctionObject

Description
    Set the time-step according to a number returned by a interpreter-snipplet

SourceFiles
    SetDeltaTWithInterpreterFunctionObject.C

Contributors/Copyright:
    2008-2011, 2013-2014, 2016-2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>

 SWAK Revision: $Id$
\*---------------------------------------------------------------------------*/

#ifndef SetDeltaTWithInterpreterFunctionObject_H
#define SetDeltaTWithInterpreterFunctionObject_H

#include "TimeManipulationWithInterpreterFunctionObject.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class SetDeltaTWithInterpreterFunctionObject Declaration
\*---------------------------------------------------------------------------*/

template<class Wrapper>
class SetDeltaTWithInterpreterFunctionObject
:
    public TimeManipulationWithInterpreterFunctionObject<Wrapper>
{
    // Private Member Functions

    //- Disallow default bitwise copy construct
    SetDeltaTWithInterpreterFunctionObject(const SetDeltaTWithInterpreterFunctionObject&);

    //- Disallow default bitwise assignment
    void operator=(const SetDeltaTWithInterpreterFunctionObject&);

    //- the code that should be evaluated in the beginning
    string initCode_;

    //- the code that should be evaluated
    string deltaTCode_;

    void readParameters(const dictionary &);

public:

    //- Runtime type information
    TypeName("SetDeltaTWithInterpreter");


    // Constructors

    //- Construct from components
    SetDeltaTWithInterpreterFunctionObject
    (
        const word&,
        const Time&,
        const dictionary&
    );

    virtual scalar deltaT();

    bool read(const dictionary &dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#ifdef NoRepository
#include "SetDeltaTWithInterpreterFunctionObject.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
