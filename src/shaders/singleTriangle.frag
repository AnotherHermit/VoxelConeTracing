///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#version 430

in vec2 outTexCoords;

out vec4 outColor;

uniform sampler2D diffuseUnit;

void main()
{	
	// Set constant color for textured models
	vec3 color = texture(diffuseUnit, outTexCoords).rgb;
	
	// Output complete color
	outColor =  vec4(color, 1.0);
}
