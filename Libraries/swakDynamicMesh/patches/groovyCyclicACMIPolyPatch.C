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

Contributors/Copyright:
    2016-2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>

 SWAK Revision: $Id$
\*---------------------------------------------------------------------------*/

#include "swak.H"

#if defined(FOAM_HAS_ACMI_INTERFACE) && !defined(FOAM_OLD_AMI_ACMI)

#include "groovyCyclicACMIPolyPatch.H"
#include "SubField.H"
#include "Time.H"
#include "addToRunTimeSelectionTable.H"

#include "cyclicACMIFvPatch.H"
#include "cyclicACMIPointPatch.H"

#include "DebugOStream.H"

#include "volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(groovyCyclicACMIPolyPatch, 0);

    addToRunTimeSelectionTable(polyPatch, groovyCyclicACMIPolyPatch, word);
    addToRunTimeSelectionTable(polyPatch, groovyCyclicACMIPolyPatch, dictionary);

    addNamedToRunTimeSelectionTable(fvPatch,cyclicACMIFvPatch,polyPatch,groovyCyclicACMI);
    addNamedToRunTimeSelectionTable(facePointPatch,cyclicACMIPointPatch,polyPatch,groovyCyclicACMI);
}

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::groovyCyclicACMIPolyPatch::resetAMI
(
#ifndef FOAM_ACMI_RESETAMI_NO_PARAMETERS
    const AMIPatchToPatchInterpolation::interpolationMethod& meth
#endif
) const
{
    // copy/pasted and then adapted version of the original method.
    // Code duplication necessary because that makes sure that the
    // masks are set consistent

    Dbug << "resetAMI - entering" << endl;

    if (owner())
    {
        const polyPatch& nonOverlapPatch = this->nonOverlapPatch();

        if (debug)
        {
            Pout<< "cyclicACMIPolyPatch::resetAMI : recalculating weights"
                << " for " << name() << " and " << nonOverlapPatch.name()
                << endl;
        }

        if (boundaryMesh().mesh().hasCellCentres())
        {
            if (debug)
            {
                Pout<< "cyclicACMIPolyPatch::resetAMI : clearing cellCentres"
                    << " for " << name() << " and " << nonOverlapPatch.name()
                    << endl;
            }

            const_cast<polyMesh&>
            (
                boundaryMesh().mesh()
            ).primitiveMesh::clearGeom();
        }


        // Trigger re-building of faceAreas
        (void)boundaryMesh().mesh().faceAreas();


        // Calculate the AMI using partial face-area-weighted. This leaves
        // the weights as fractions of local areas (sum(weights) = 1 means
        // face is fully covered)
        cyclicAMIPolyPatch::resetAMI
        (
#ifndef FOAM_ACMI_RESETAMI_NO_PARAMETERS
            AMIPatchToPatchInterpolation::imPartialFaceAreaWeight
#endif
        );

        AMIPatchToPatchInterpolation& AMI =
#ifdef FOAM_ACMI_HAS_NO_AMI_METHOD
            this->AMIs_[0];
#else
            const_cast<AMIPatchToPatchInterpolation&>(this->AMI());
#endif

        scalarField &srcMask=const_cast<scalarField&>(
            this->srcMask()
        );
        scalarField &tgtMask=const_cast<scalarField&>(
            this->tgtMask()
        );

        scalarField openValueSrc(
            AMI.srcWeightsSum().size(),
            1
        );
        if(meshReady_) {
            Dbug << "Getting open faces" << endl;

            const volScalarField &openField=
                boundaryMesh().mesh().lookupObject<volScalarField>(openField_);
            openValueSrc=min(
                max(
                    0.,
                    openField.boundaryField()[nonOverlapPatchID()]
                ),
                1.
            );
        } else {
            Info << "Patch " << name() << " will be controlled by "
                << nonOverlapPatch.name() << endl;
        }
        Dbug << "openValueSrc: " << min(openValueSrc) << ", " << max(openValueSrc) << ", "
            << average(openValueSrc) << " - " << openValueSrc.size() << endl;
        scalarField openValueTgt(
            AMI.interpolateToTarget(openValueSrc)
        );
        Dbug << "openValueTgt: " << min(openValueTgt) << ", " << max(openValueTgt) << ", "
            << average(openValueTgt) << " - " << openValueTgt.size()  << endl;

        srcMask =
            min(scalar(1) - tolerance(), max(tolerance(), AMI.srcWeightsSum()*openValueSrc));

        tgtMask =
            min(scalar(1) - tolerance(), max(tolerance(), AMI.tgtWeightsSum()*openValueTgt));

        Dbug << "srcMask: " << min(srcMask) << ", " << max(srcMask) << ", "
            << average(srcMask) << " - " << srcMask.size() << endl;
        Dbug << "tgtMask: " << min(tgtMask) << ", " << max(tgtMask) << ", "
            << average(tgtMask) << " - " << tgtMask.size()  << endl;

        // Adapt owner side areas. Note that in uncoupled situations (e.g.
        // decomposePar) srcMask, tgtMask can be zero size.
        if (srcMask.size())
        {
            vectorField::subField Sf = faceAreas();
            vectorField::subField noSf = nonOverlapPatch.faceAreas();

            forAll(Sf, facei)
            {
                Sf[facei] *= srcMask[facei];
                noSf[facei] *= 1.0 - srcMask[facei];
            }
        }
        // Adapt slave side areas
        if (tgtMask.size())
        {
            const cyclicACMIPolyPatch& cp =
                refCast<const cyclicACMIPolyPatch>(
#ifdef FOAM_NEIGHBOUR_IS_NOW_NBR_PATCHFIELD
                    this->nbrPatch()
#else
                    this->neighbPatch()
#endif
                );
            const polyPatch& pp = cp.nonOverlapPatch();

            vectorField::subField Sf = cp.faceAreas();
            vectorField::subField noSf = pp.faceAreas();

            forAll(Sf, facei)
            {
                Sf[facei] *= tgtMask[facei];
                noSf[facei] *= 1.0 - tgtMask[facei];
            }
        }

        // Re-normalise the weights since the effect of overlap is already
        // accounted for in the area.
        {
            scalarListList& srcWeights = AMI.srcWeights();
            scalarField& srcWeightsSum = AMI.srcWeightsSum();
            forAll(srcWeights, i)
            {
                scalarList& wghts = srcWeights[i];
                if (wghts.size())
                {
                    scalar& sum = srcWeightsSum[i];

                    forAll(wghts, j)
                    {
                        wghts[j] /= sum;
                    }
                    sum = 1.0;
                }
            }
        }
        {
            scalarListList& tgtWeights = AMI.tgtWeights();
            scalarField& tgtWeightsSum = AMI.tgtWeightsSum();
            forAll(tgtWeights, i)
            {
                scalarList& wghts = tgtWeights[i];
                if (wghts.size())
                {
                    scalar& sum = tgtWeightsSum[i];
                    forAll(wghts, j)
                    {
                        wghts[j] /= sum;
                    }
                    sum = 1.0;
                }
            }
        }

        // Set the updated flag
#ifndef FOAM_ACMI_POLYPATCH_NO_SETUPDATED_METHOD
        setUpdated(true);
#endif
    }
    const_cast<groovyCyclicACMIPolyPatch&>(*this).meshReady_=true;
    Dbug << "resetAMI - leaving" << endl;
}




// * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * //

Foam::groovyCyclicACMIPolyPatch::groovyCyclicACMIPolyPatch
(
    const word& name,
    const label size,
    const label start,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
#ifndef FOAM_CYCLIC_FV_PATCH_NEW_TRANSFORM_IMPLEMENTATION
    ,const transformType transform
#endif
)
:
    cyclicACMIPolyPatch(
        name, size, start, index, bm, patchType
#ifndef FOAM_CYCLIC_FV_PATCH_NEW_TRANSFORM_IMPLEMENTATION
        , transform
#endif
    ),
    meshReady_(false),
    openField_(word::null)
{
    Dbug << "Constructor 1" << endl;
}


Foam::groovyCyclicACMIPolyPatch::groovyCyclicACMIPolyPatch
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    cyclicACMIPolyPatch(name, dict, index, bm, patchType),
    meshReady_(false),
    openField_(dict.lookup("openField"))
{
    Dbug << "Constructor 2" << endl;
}


Foam::groovyCyclicACMIPolyPatch::groovyCyclicACMIPolyPatch
(
    const groovyCyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm
)
:
    cyclicACMIPolyPatch(pp, bm),
    meshReady_(pp.meshReady_),
    openField_(pp.openField_)
{
    Dbug << "Constructor 3" << endl;
}


Foam::groovyCyclicACMIPolyPatch::groovyCyclicACMIPolyPatch
(
    const groovyCyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const label newSize,
    const label newStart,
    const word& nbrPatchName,
    const word& nonOverlapPatchName,
    const word& openField
)
:
    cyclicACMIPolyPatch(
        pp,
        bm,
        index,
        newSize,
        newStart,
        nbrPatchName,
        nonOverlapPatchName
    ),
    meshReady_(false),
    openField_(openField)
{
    Dbug << "Constructor 4" << endl;
}


Foam::groovyCyclicACMIPolyPatch::groovyCyclicACMIPolyPatch
(
    const groovyCyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const labelUList& mapAddressing,
    const label newStart
)
:
    cyclicACMIPolyPatch(pp, bm, index, mapAddressing, newStart),
    meshReady_(pp.meshReady_),
    openField_(pp.openField_)
{
    Dbug << "Constructor 5" << endl;
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::groovyCyclicACMIPolyPatch::~groovyCyclicACMIPolyPatch()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //



void Foam::groovyCyclicACMIPolyPatch::write(Ostream& os) const
{
    cyclicACMIPolyPatch::write(os);

    os.writeKeyword("openField") << openField_
        << token::END_STATEMENT << nl;
}

#endif

// ************************************************************************* //
