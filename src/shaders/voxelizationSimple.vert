///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 exNormal;

void main(void)
{
	exNormal = inNormal;
	gl_Position = vec4(inPosition, 1.0f);
}

