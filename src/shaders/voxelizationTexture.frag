///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 exTexCoords;

out vec4 outColor;

uniform sampler2D diffuseUnit;

void main()
{	
	// Set constant color for textured models
	vec3 color = texture(diffuseUnit, exTexCoords).rgb;

	// Output complete color
	outColor =  vec4(color, 1.0);
}
