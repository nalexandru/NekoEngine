
/*
Copyright (c) 2008, Luke Benstead.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VEC3_H_INCLUDED
#define VEC3_H_INCLUDED

#include <assert.h>
#include "utility.h"

struct kmMat4;
struct kmMat3;
struct kmPlane;

#pragma pack(push, 1)

typedef struct kmVec3 {
	kmScalar x;
	kmScalar y;
	kmScalar z;
} kmVec3;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fill a kmVec3 structure using 3 floating point values
 * The result is store in pOut, returns pOut
 */
kmVec3* kmVec3Fill(kmVec3* pOut, kmScalar x, kmScalar y, kmScalar z);

/** Returns the length of the vector */
kmScalar kmVec3Length(const kmVec3* pIn);

/** Returns the square of the length of the vector */
kmScalar kmVec3LengthSq(const kmVec3* pIn);

/** Returns the interpolation of 2 4D vectors based on t.*/
kmVec3* kmVec3Lerp(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2,
                   kmScalar t);

/**
 * Returns the vector passed in set to unit length
 * the result is stored in pOut.
 */
kmVec3* kmVec3Normalize(kmVec3* pOut, const kmVec3* pIn);

/**
 * Returns a vector perpendicular to 2 other vectors.
 * The result is stored in pOut.
 */
kmVec3* kmVec3Cross(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);

/** Returns the cosine of the angle between 2 vectors */
kmScalar kmVec3Dot(const kmVec3* pV1, const kmVec3* pV2);

/**
 * Adds 2 vectors and returns the result. The resulting
 * vector is stored in pOut.
 */
kmVec3* kmVec3Add(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);

/**
 * Subtracts 2 vectors and returns the result. The result is stored in
 * pOut.
 */
kmVec3* kmVec3Subtract(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
kmVec3* kmVec3Mul( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 ); 
kmVec3* kmVec3Div( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 );

kmVec3* kmVec3MultiplyMat3(kmVec3 *pOut, const kmVec3 *pV,
                           const struct kmMat3* pM);

/**
 * Multiplies vector (x, y, z, 1) by a given matrix. The result
 * is stored in pOut. pOut is returned.
 */
kmVec3* kmVec3MultiplyMat4(kmVec3* pOut, const kmVec3* pV,
                           const struct kmMat4* pM);

/** Transforms a vector (assuming w=1) by a given matrix (deprecated) */
kmVec3* kmVec3Transform(kmVec3* pOut, const kmVec3* pV1,
                        const struct kmMat4* pM);

/**Transforms a 3D normal by a given matrix */
kmVec3* kmVec3TransformNormal(kmVec3* pOut, const kmVec3* pV,
                              const struct kmMat4* pM);

/**Transforms a 3D vector by a given matrix, projecting the result
 * back into w = 1. */
kmVec3* kmVec3TransformCoord(kmVec3* pOut, const kmVec3* pV,
                             const struct kmMat4* pM);

/**
 * Scales a vector to length s. Does not normalize first,
 * you should do that!
 */
kmVec3* kmVec3Scale(kmVec3* pOut, const kmVec3* pIn, const kmScalar s);

/**
 * Returns KM_TRUE if the 2 vectors are approximately equal
 */
kmBool kmVec3AreEqual(const kmVec3* p1, const kmVec3* p2);
kmVec3* kmVec3InverseTransform(kmVec3* pOut, const kmVec3* pV,
                               const struct kmMat4* pM);
kmVec3* kmVec3InverseTransformNormal(kmVec3* pOut, const kmVec3* pVect,
                                     const struct kmMat4* pM);

/**
 * Assigns pIn to pOut. Returns pOut. If pIn and pOut are the same
 * then nothing happens but pOut is still returned
 */
kmVec3* kmVec3Assign(kmVec3* pOut, const kmVec3* pIn);

/**
 * Sets all the elements of pOut to zero. Returns pOut.
 */
kmVec3* kmVec3Zero(kmVec3* pOut);

/**
 * Get the rotations that would make a (0,0,1) direction vector point
 * in the same direction as this direction vector.  Useful for
 * orienting vector towards a point.
 *
 * Returns a rotation vector containing the X (pitch) and Y (raw)
 * rotations (in degrees) that when applied to a +Z (e.g. 0, 0, 1)
 * direction vector would make it point in the same direction as this
 * vector. The Z (roll) rotation is always 0, since two Euler
 * rotations are sufficient to point in any given direction.
 */
kmVec3* kmVec3GetHorizontalAngle(kmVec3* pOut, const kmVec3 *pIn);

/**
 * Builds a direction vector from input vector.
 *
 * Input vector is assumed to be rotation vector composed from 3 Euler
 * angle rotations, in degrees.  The forwards vector will be rotated
 * by the input vector
 */
kmVec3* kmVec3RotationToDirection(kmVec3* pOut, const kmVec3* pIn,
                                  const kmVec3* forwards);

kmVec3* kmVec3ProjectOnToPlane(kmVec3* pOut, const kmVec3* point,
                               const struct kmPlane* plane);
kmVec3* kmVec3ProjectOnToVec3(const kmVec3* pIn, const kmVec3* other,
                              kmVec3* projection);

/**< Reflects a vector about a given surface normal. The surface
 * normal is assumed to be of unit length. */
kmVec3* kmVec3Reflect(kmVec3* pOut, const kmVec3* pIn, const kmVec3* normal);

/**
 * swaps the values in one vector with another
 * NB does not return a value unlike normal
 */
void kmVec3Swap(kmVec3* a, kmVec3* b);
void kmVec3OrthoNormalize(kmVec3* normal, kmVec3* tangent);

extern const kmVec3 KM_VEC3_NEG_Z;
extern const kmVec3 KM_VEC3_POS_Z;
extern const kmVec3 KM_VEC3_POS_Y;
extern const kmVec3 KM_VEC3_NEG_Y;
extern const kmVec3 KM_VEC3_NEG_X;
extern const kmVec3 KM_VEC3_POS_X;
extern const kmVec3 KM_VEC3_ZERO;

#ifdef __cplusplus
}
#endif
#endif /* VEC3_H_INCLUDED */
