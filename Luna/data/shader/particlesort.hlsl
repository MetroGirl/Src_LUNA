#include "particle.hlsli"

#define BLOCK_SIZE 512
#define TRANSPOSE_SIZE 16

cbuffer CB : register(b1)
{
	uint cLevel;
	uint cLevelMask;
	uint cWidth;
	uint cHeight;
}

StructuredBuffer<ParticleIndex> gInput : register(t0);
RWStructuredBuffer<ParticleIndex> gOutput : register(u0);
Buffer<uint> gIndirectDrawArgs : register(t1);

groupshared ParticleIndex gShared[BLOCK_SIZE];

[numthreads(BLOCK_SIZE, 1, 1)]
void BitonicSortCS( uint3 Gid : SV_GroupID, 
                  uint DTid : SV_DispatchThreadID, 
                  uint3 GTid : SV_GroupThreadID, 
                  uint GI : SV_GroupIndex )
{
	gShared[GI] = gOutput[DTid];
	GroupMemoryBarrierWithGroupSync();
	for(uint j = cLevel >> 1; j > 0; j >>= 1)
	{
		bool cmp = (gShared[GI & ~j].viewDistanceSq > gShared[GI | j].viewDistanceSq);
		ParticleIndex result;

//		uint swapIndex = DTid + (GI ^ j);

		if (cmp == (bool)(cLevelMask & DTid)) {
			result = gShared[GI ^ j];
		} else {
			result = gShared[GI];
		}
		GroupMemoryBarrierWithGroupSync();
		gShared[GI] = result;
		GroupMemoryBarrierWithGroupSync();
	}
	gOutput[DTid] = gShared[GI];
}

groupshared ParticleIndex gTransposeShared[TRANSPOSE_SIZE * TRANSPOSE_SIZE];

[numthreads(TRANSPOSE_SIZE, TRANSPOSE_SIZE, 1)]
void MatrixTransposeCS(uint3 Gid : SV_GroupID, 
                      uint2 DTid : SV_DispatchThreadID, 
                      uint2 GTid : SV_GroupThreadID, 
                      uint GI : SV_GroupIndex )
{
	gTransposeShared[GI] = gInput[DTid.y * cWidth + DTid.x];
	GroupMemoryBarrierWithGroupSync();
	uint2 XY = DTid.yx - GTid.yx + GTid.xy;
	gOutput[XY.y * cHeight + XY.x] = gTransposeShared[GTid.x * TRANSPOSE_SIZE + GTid.y];
}
