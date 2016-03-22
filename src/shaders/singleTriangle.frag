///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 exTexCoords;

out vec4 outColor;

uniform sampler2D usedView;

void main()
{	
	outColor = texture(usedView, exTexCoords);
}
