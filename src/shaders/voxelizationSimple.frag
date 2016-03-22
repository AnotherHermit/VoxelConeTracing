///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

out vec4 outColor;

flat in uint domDir;

//uniform vec3 diffColor;

uniform layout(rgba8ui) uimage2D frontView;
uniform layout(rgba8ui) uimage2D sideView;
uniform layout(rgba8ui) uimage2D topView;

void main()
{	
	// Set constant color for textureless models
	//vec4 color = vec4(diffColor,1.0f);
	ivec2 coords = ivec2(floor(gl_FragCoord.xy));

	if(domDir == 0) {
		imageStore(frontView, coords, uvec4(255, 0, 0, 255));
	} else if(domDir == 1) {
		imageStore(frontView, coords, uvec4(0, 255, 0, 255));
	} else {
		imageStore(frontView, coords, uvec4(0, 0, 255, 255));
	}
}
