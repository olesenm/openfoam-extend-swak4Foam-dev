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

    swak4Foam is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    swak4Foam is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with swak4Foam.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::CloudMoveStatistics

Description
    Record the number of faces etc that were hit during one evolution. This is
    mainly used to find out whether there is a problem during the tracking phase

    Based on the CloudTracks-function object

SourceFiles
    CloudMoveStatistics.C

Contributors/Copyright:
    2016, 2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>
    2018 Mark Olesen <Mark.Olesen@esi-group.com>

 SWAK Revision: $Id:  $
\*---------------------------------------------------------------------------*/

#ifndef CloudMoveStatistics_H
#define CloudMoveStatistics_H

#include "CloudFunctionObject.H"

#include "swakCloudTypes.H"

#include "TimelineCollection.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class CloudMoveStatistics Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class CloudMoveStatistics
:
    public CloudFunctionObject<CloudType>
{
    // Private data

    // Typedefs

    //- Convenience typedef for parcel type
    typedef typename CloudType::parcelType parcelType;

    //- Convenience typedef for hash hit-table
    typedef HashTable<label, labelPair, typename labelPair::Hash<> > hitTableType;

    typedef HashTable<label> patchHitTableType;

    //- Table of number of times a particle has hit a face
    hitTableType faceHitCounter_;

    //- Table of number of times a particle has been moved
    hitTableType movesCounter_;

    //- table of the number of times a patch has been hit
    patchHitTableType patchHitCounter_;

    //- number of hits to report a particle
    label reportHitNr_;

    //- number of moves to report a particle
    label reportMoveNr_;

    //- here we get our files from
    TimelineCollection out_;

protected:

    // Protected member functions

    //- Write post-processing info
    void write();


public:

    //- Runtime type information
    TypeName("cloudMoveStatistics");


    // Constructors

    //- Construct from dictionary
    CloudMoveStatistics
    (
        const dictionary& dict,
        CloudType& owner,
        const word& modelName
    );

    //- Construct copy
    CloudMoveStatistics(const CloudMoveStatistics<CloudType>& ppm);

    //- Construct and return a clone
    virtual autoPtr<CloudFunctionObject<CloudType> > clone() const
        {
            return autoPtr<CloudFunctionObject<CloudType> >
                (
                    new CloudMoveStatistics<CloudType>(*this)
                );
        }


    //- Destructor
    virtual ~CloudMoveStatistics();


    // Member Functions

    // Evaluation

    //- Pre-evolve hook
    virtual void preEvolve(
#ifdef FOAM_CLOUD_FUNCTION_OBJECT_PRE_POST_EVOLVE_WITH_TD
        const typename CloudType::parcelType::trackingData &td
#endif
    );

    //- Post-evolve hook
    virtual void postEvolve(
#ifdef FOAM_CLOUD_FUNCTION_OBJECT_PRE_POST_EVOLVE_WITH_TD
        const typename CloudType::parcelType::trackingData &td
#endif
    );

    //- Post-move hook
    virtual void postMove
    (
        typename CloudType::parcelType& p,
        const label cellI,
        const scalar dt,
        const point& position0,
        bool& keepParticle
    );

    //- Post-patch hook
    virtual void postPatch
    (
        const typename CloudType::parcelType& p,
        const polyPatch& pp,
        const scalar trackFraction,
        const tetIndices& testIs,
        bool& keepParticle
    );

    //- Post-face hook
    virtual void postFace
    (
        const parcelType& p,
        const label faceI,
        bool& keepParticle
    );


};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "CloudMoveStatistics.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
