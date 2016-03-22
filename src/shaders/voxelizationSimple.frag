///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

out vec4 outColor;

//in vec3 domDir;

uniform vec3 diffColor;

uniform layout(RGBA32F) image2D frontView;

void main()
{	
	// Set constant color for textureless models
	vec3 color = /*domDir;*/ diffColor;



	imageStore(frontView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
	
	// Output complete color
	outColor =  vec4(color, 1.0f);
}
