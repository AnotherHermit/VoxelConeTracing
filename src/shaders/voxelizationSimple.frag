///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

out vec4 outColor;

uniform vec3 diffColor;

void main()
{	
	// Set constant color for textureless models
	vec3 color = diffColor;
	
	// Output complete color
	outColor =  vec4(color, 1.0f);
}
