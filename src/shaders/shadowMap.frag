///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 6) uniform layout(R32UI) uimage2D shadowMap;
//layout(location = 0) out float shadow;

void main()
{	
	imageAtomicMin(shadowMap, ivec2(gl_FragCoord.xy), uint((1-gl_FragCoord.z) * 65536.0f));
	//shadow = gl_FragCoord.z;
}
