///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 exTexCoords;

out vec4 outColor;

uniform int drawVoxels;

uniform sampler2D usedView;

uniform sampler3D voxelData;
uniform int layer;
uniform int voxelRes;

void main()
{	

	if(drawVoxels == 1) {
		outColor = texture(usedView, exTexCoords);
	} else {
		outColor = texture(voxelData, vec3(exTexCoords, float(layer) / float(voxelRes-1)));
	}
}
