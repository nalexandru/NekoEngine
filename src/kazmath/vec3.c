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

/**
 * @file vec3.c
 */

#include <assert.h>
#include <memory.h>

#include <kazmath/utility.h>
#include <kazmath/vec4.h>
#include <kazmath/mat4.h>
#include <kazmath/mat3.h>
#include <kazmath/vec3.h>
#include <kazmath/plane.h>
#include <kazmath/ray3.h>

const kmVec3 KM_VEC3_POS_Z = { 0, 0, 1 };
const kmVec3 KM_VEC3_NEG_Z = { 0, 0, -1 };
const kmVec3 KM_VEC3_POS_Y = { 0, 1, 0 };
const kmVec3 KM_VEC3_NEG_Y = { 0, -1, 0 };
const kmVec3 KM_VEC3_NEG_X = { -1, 0, 0 };
const kmVec3 KM_VEC3_POS_X = { 1, 0, 0 };
const kmVec3 KM_VEC3_ZERO = { 0, 0, 0 };

kmVec3* kmVec3Fill(kmVec3* pOut, kmScalar x, kmScalar y, kmScalar z)
{
    pOut->x = x;
    pOut->y = y;
    pOut->z = z;
    return pOut;
}


kmScalar kmVec3Length(const kmVec3* pIn)
{
	return sqrtf(kmSQR(pIn->x) + kmSQR(pIn->y) + kmSQR(pIn->z));
}

kmScalar kmVec3LengthSq(const kmVec3* pIn)
{
	return kmSQR(pIn->x) + kmSQR(pIn->y) + kmSQR(pIn->z);
}

kmVec3* kmVec3Lerp(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2, kmScalar t) {
    pOut->x = pV1->x + t * ( pV2->x - pV1->x ); 
    pOut->y = pV1->y + t * ( pV2->y - pV1->y ); 
    pOut->z = pV1->z + t * ( pV2->z - pV1->z ); 
    return pOut;
}

kmVec3* kmVec3Normalize(kmVec3* pOut, const kmVec3* pIn)
{
	kmVec3 v;
        kmScalar l;
        if (!pIn->x && !pIn->y && !pIn->z)
                return kmVec3Assign(pOut, pIn);

        l = 1.0f / kmVec3Length(pIn);

	v.x = pIn->x * l;
	v.y = pIn->y * l;
	v.z = pIn->z * l;

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

	return pOut;
}

kmVec3* kmVec3Cross(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2)
{

	kmVec3 v;

	v.x = (pV1->y * pV2->z) - (pV1->z * pV2->y);
	v.y = (pV1->z * pV2->x) - (pV1->x * pV2->z);
	v.z = (pV1->x * pV2->y) - (pV1->y * pV2->x);

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

	return pOut;
}

kmScalar kmVec3Dot(const kmVec3* pV1, const kmVec3* pV2)
{
	return (  pV1->x * pV2->x
			+ pV1->y * pV2->y
			+ pV1->z * pV2->z );
}

kmVec3* kmVec3Add(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2)
{
	kmVec3 v;

	v.x = pV1->x + pV2->x;
	v.y = pV1->y + pV2->y;
	v.z = pV1->z + pV2->z;

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

	return pOut;
}

kmVec3* kmVec3Subtract(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2)
{
	kmVec3 v;

	v.x = pV1->x - pV2->x;
	v.y = pV1->y - pV2->y;
	v.z = pV1->z - pV2->z;

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

	return pOut;
}

kmVec3* kmVec3Mul( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 ) {
    pOut->x = pV1->x * pV2->x;
    pOut->y = pV1->y * pV2->y;
    pOut->z = pV1->z * pV2->z;
    return pOut;
}

kmVec3* kmVec3Div( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 ) {
    if ( pV2->x && pV2->y && pV2->z ){
        pOut->x = pV1->x / pV2->x;
        pOut->y = pV1->y / pV2->y;
        pOut->z = pV1->z / pV2->z;
    }
    return pOut;
}

kmVec3* kmVec3MultiplyMat3(kmVec3* pOut, const kmVec3* pV, const kmMat3* pM) {
    kmVec3 v;

    v.x = pV->x * pM->mat[0] + pV->y * pM->mat[3] + pV->z * pM->mat[6];
    v.y = pV->x * pM->mat[1] + pV->y * pM->mat[4] + pV->z * pM->mat[7];
    v.z = pV->x * pM->mat[2] + pV->y * pM->mat[5] + pV->z * pM->mat[8];

    pOut->x = v.x;
    pOut->y = v.y;
    pOut->z = v.z;

    return pOut;
}

kmVec3* kmVec3MultiplyMat4(kmVec3* pOut, const kmVec3* pV, const kmMat4* pM) {
    kmVec3 v;

    v.x = pV->x * pM->mat[0] + pV->y * pM->mat[4] + pV->z * pM->mat[8] + pM->mat[12];
    v.y = pV->x * pM->mat[1] + pV->y * pM->mat[5] + pV->z * pM->mat[9] + pM->mat[13];
    v.z = pV->x * pM->mat[2] + pV->y * pM->mat[6] + pV->z * pM->mat[10] + pM->mat[14];

    pOut->x = v.x;
    pOut->y = v.y;
    pOut->z = v.z;

    return pOut;
}


kmVec3* kmVec3Transform(kmVec3* pOut, const kmVec3* pV, const kmMat4* pM)
{
	/*
        @deprecated Should intead use kmVec3MultiplyMat4
	*/
    return kmVec3MultiplyMat4(pOut, pV, pM);
}

kmVec3* kmVec3InverseTransform(kmVec3* pOut, const kmVec3* pVect, const kmMat4* pM)
{
	kmVec3 v1, v2;

	v1.x = pVect->x - pM->mat[12];
	v1.y = pVect->y - pM->mat[13];
	v1.z = pVect->z - pM->mat[14];

	v2.x = v1.x * pM->mat[0] + v1.y * pM->mat[1] + v1.z * pM->mat[2];
	v2.y = v1.x * pM->mat[4] + v1.y * pM->mat[5] + v1.z * pM->mat[6];
	v2.z = v1.x * pM->mat[8] + v1.y * pM->mat[9] + v1.z * pM->mat[10];

	pOut->x = v2.x;
	pOut->y = v2.y;
	pOut->z = v2.z;

	return pOut;
}

kmVec3* kmVec3InverseTransformNormal(kmVec3* pOut, const kmVec3* pVect, const kmMat4* pM)
{
	kmVec3 v;

	v.x = pVect->x * pM->mat[0] + pVect->y * pM->mat[1] + pVect->z * pM->mat[2];
	v.y = pVect->x * pM->mat[4] + pVect->y * pM->mat[5] + pVect->z * pM->mat[6];
	v.z = pVect->x * pM->mat[8] + pVect->y * pM->mat[9] + pVect->z * pM->mat[10];

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

	return pOut;
}


kmVec3* kmVec3TransformCoord(kmVec3* pOut, const kmVec3* pV, const kmMat4* pM)
{
	/*
        a = (Vx, Vy, Vz, 1)
        b = (a×M)T
        Out = 1⁄bw(bx, by, bz)
	*/

    kmVec4 v;
    kmVec4 inV;
    kmVec4Fill(&inV, pV->x, pV->y, pV->z, 1.0);

    kmVec4Transform(&v, &inV,pM);

	pOut->x = v.x / v.w;
	pOut->y = v.y / v.w;
	pOut->z = v.z / v.w;

	return pOut;
}

kmVec3* kmVec3TransformNormal(kmVec3* pOut, const kmVec3* pV, const kmMat4* pM)
{
/*
    a = (Vx, Vy, Vz, 0)
    b = (a×M)T
    Out = (bx, by, bz)
*/
    /*Omits the translation, only scaling + rotating*/
	kmVec3 v;

	v.x = pV->x * pM->mat[0] + pV->y * pM->mat[4] + pV->z * pM->mat[8];
	v.y = pV->x * pM->mat[1] + pV->y * pM->mat[5] + pV->z * pM->mat[9];
	v.z = pV->x * pM->mat[2] + pV->y * pM->mat[6] + pV->z * pM->mat[10];

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;

    return pOut;

}

kmVec3* kmVec3Scale(kmVec3* pOut, const kmVec3* pIn, const kmScalar s)
{
	pOut->x = pIn->x * s;
	pOut->y = pIn->y * s;
	pOut->z = pIn->z * s;

	return pOut;
}

kmBool kmVec3AreEqual(const kmVec3* p1, const kmVec3* p2)
{
    if((!kmAlmostEqual(p1->x, p2->x)) || (!kmAlmostEqual(p1->y, p2->y)) || (!kmAlmostEqual(p1->z, p2->z))) {
        return KM_FALSE;
    }

    return KM_TRUE;
}

kmVec3* kmVec3Assign(kmVec3* pOut, const kmVec3* pIn) {
	if (pOut == pIn) {
		return pOut;
	}

	pOut->x = pIn->x;
	pOut->y = pIn->y;
	pOut->z = pIn->z;

	return pOut;
}

kmVec3* kmVec3Zero(kmVec3* pOut) {
	pOut->x = 0.0f;
	pOut->y = 0.0f;
	pOut->z = 0.0f;

	return pOut;
}

/*
 * Code ported from Irrlicht: http://irrlicht.sourceforge.net/
 */
kmVec3* kmVec3GetHorizontalAngle(kmVec3* pOut, const kmVec3 *pIn) {
   const kmScalar z1 = sqrt(pIn->x * pIn->x + pIn->z * pIn->z);

   pOut->y = kmRadiansToDegrees(atan2(pIn->x, pIn->z));
   if (pOut->y < 0)
      pOut->y += 360;
   if (pOut->y >= 360)
      pOut->y -= 360;

   pOut->x = kmRadiansToDegrees(atan2(z1, pIn->y)) - 90.0;
   if (pOut->x < 0)
      pOut->x += 360;
   if (pOut->x >= 360)
      pOut->x -= 360;

   return pOut;
}

/*
 * Code ported from Irrlicht: http://irrlicht.sourceforge.net/
 */
kmVec3* kmVec3RotationToDirection(kmVec3* pOut, const kmVec3* pIn, const kmVec3* forwards)
{
   const kmScalar xr = kmDegreesToRadians(pIn->x);
   const kmScalar yr = kmDegreesToRadians(pIn->y);
   const kmScalar zr = kmDegreesToRadians(pIn->z);

   const kmScalar cr = cos(xr);
   const kmScalar sr = sin(xr);

   const kmScalar cp = cos(yr);
   const kmScalar sp = sin(yr);

   const kmScalar cy = cos(zr);
   const kmScalar sy = sin(zr);

   const kmScalar srsp = sr*sp;
   const kmScalar crsp = cr*sp;

   const kmScalar pseudoMatrix[] = {
      (cp*cy), (cp*sy), (-sp),
      (srsp*cy-cr*sy), (srsp*sy+cr*cy), (sr*cp),
      (crsp*cy+sr*sy), (crsp*sy-sr*cy), (cr*cp)
   };

   pOut->x = forwards->x * pseudoMatrix[0] +
             forwards->y * pseudoMatrix[3] +
             forwards->z * pseudoMatrix[6];

   pOut->y = forwards->x * pseudoMatrix[1] +
             forwards->y * pseudoMatrix[4] +
             forwards->z * pseudoMatrix[7];

   pOut->z = forwards->x * pseudoMatrix[2] +
             forwards->y * pseudoMatrix[5] +
             forwards->z * pseudoMatrix[8];

   return pOut;
}

kmVec3* kmVec3ProjectOnToPlane(kmVec3* pOut, const kmVec3* point, const struct kmPlane* plane) {
    kmVec3 N;
    kmVec3Fill(&N, plane->a, plane->b, plane->c);
    kmVec3Normalize(&N, &N);
    kmScalar distance = -kmVec3Dot(&N, point);
    kmVec3Scale(&N, &N, distance);
    kmVec3Add(pOut, point, &N);
    return pOut;
}

kmVec3* kmVec3Reflect(kmVec3* pOut, const kmVec3* pIn, const kmVec3* normal) {
  kmVec3 tmp;
  kmVec3Scale(&tmp, normal, 2.0f * kmVec3Dot(pIn, normal));
  kmVec3Subtract(pOut, pIn, &tmp);

  return pOut;
}

void kmVec3Swap(kmVec3* a, kmVec3* b) {
	kmScalar x, y,z;
	x = a->x;
	a->x = b->x;
	b->x = x;

	y = a->y;
	a->y = b->y;
	b->y = y;

	z = a->z;
	a->z = b->z;
	b->z = z;
}


void kmVec3OrthoNormalize(kmVec3* normal, kmVec3* tangent) {
    kmVec3 proj;

    kmVec3Normalize(normal, normal);

    kmVec3Scale(&proj, normal, kmVec3Dot(tangent, normal));
    kmVec3Subtract(tangent, tangent, &proj);
    kmVec3Normalize(tangent, tangent);
}

kmVec3* kmVec3ProjectOnToVec3(const kmVec3* w, const kmVec3* v,
                              kmVec3* projection) {
    kmVec3 unitW, unitV;
    kmVec3Normalize(&unitW, w);
    kmVec3Normalize(&unitV, v);

    kmScalar cos0 = kmVec3Dot(&unitW, &unitV);

    kmVec3Scale(projection, &unitV, kmVec3Length(w) * cos0);

    return projection;
}
