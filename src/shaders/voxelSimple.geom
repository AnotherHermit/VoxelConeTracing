///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(triangle) in;
layout(triangle_strip, max_vertices = 36) out;

uniform sampler3D voxelData;

in vec3 voxelPos;
in vec3 exNormal;

out vec4 outColor;
out vec3 outNormal;

void main()
{
	vec4 color = texture(voxelData, voxelPos);
	
	if(color == vec4(0.0f)) {
		EndPrimitive();
	}

	outColor = color;
	outNormal = exNormal;

	for(int i = 0; i < 3; i++) {
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	
	EndPrimitive();
}


