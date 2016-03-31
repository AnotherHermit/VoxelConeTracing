///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

flat in uint domInd;

uniform vec3 diffColor;

uniform layout(RGBA32F) image2D xView;
uniform layout(RGBA32F) image2D yView;
uniform layout(RGBA32F) image2D zView;
uniform layout(RGBA32F) image3D voxelData;

uniform int voxelRes;

void main()
{	
	// Set constant color for textureless models
	vec3 color = /*domDir; //*/ diffColor; // vec3(gl_FragCoord.z);

	if(domInd == 0) {
		imageStore(xView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
		imageStore(voxelData, ivec3(gl_FragCoord.z * voxelRes,gl_FragCoord.y, gl_FragCoord.x), vec4(color, 1.0f));
	} else if (domInd == 1) {
		imageStore(yView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
		imageStore(voxelData, ivec3(gl_FragCoord.x, voxelRes * gl_FragCoord.z, gl_FragCoord.y), vec4(color, 1.0f));
	} else {
		imageStore(zView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
		imageStore(voxelData, ivec3(gl_FragCoord.x, gl_FragCoord.y, voxelRes * (1.0f-gl_FragCoord.z)), vec4(color, 1.0f));
	}
}
