///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 intTexCoords;
in vec3 domDir;

out vec4 outColor;

uniform sampler2D diffuseUnit;

void main()
{	
	// Set constant color for textured models
	vec3 color = domDir; //texture(diffuseUnit, intTexCoords).rgb;

	// Output complete color
	outColor =  vec4(color, 1.0);
}
