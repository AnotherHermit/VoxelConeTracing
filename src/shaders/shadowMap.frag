///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
//in vec4 exPosition;

//layout(location = 0) out vec4 position;
layout(location = 0) out vec4 normal;
//layout(location = 2) out vec4 color;


void main()
{	
	normal = vec4(exNormal, 1.0f);
}
