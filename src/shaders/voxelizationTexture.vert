///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoords;

out vec3 exNormal;
out vec2 exTexCoords;

void main(void)
{
	exNormal = inNormal;
	exTexCoords = inTexCoords;
	gl_Position = vec4(inPosition, 1.0f);
}
