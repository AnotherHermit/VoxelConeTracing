///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 4) uniform layout(R16F) image2D voxelData;

void main()
{	
	imageStore(voxelData, ivec2(gl_FragCoord.xy), vec4(gl_FragCoord.z));
}
