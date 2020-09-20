uint
RandSeed(uint a, uint b)
{
	uint s = 0;

	[unroll(16)]
	for (uint i = 0; i < 16; ++i) {
		s += 0x9e3779b9;
		a += ((b << 4) + 0xa341316c) ^ (b + s) ^ ((b >> 5) + 0xc8013ea4);
		b += ((a << 4) + 0xad90777d) ^ (a + s) ^ ((a >> 5) + 0x7e95761e);
	}
	
	return a;
}

uint
RandI(inout uint s)
{
	return (s = 1664525 * s + 1013904223);
}

float
RandF(inout uint s)
{
	const uint one = 0x3f800000;
	const uint msk = 0x007fffff;
	return asfloat(one | (msk & (RandI(s) >> 9))) - 1;
}

float2
RandDisk(inout uint s)
{
	while (1) {
		const float2 v = 2 * float2(RandF(s), RandF(s)) - 1;
		if (dot(v, v) < 1)
			return v;
	}
}
