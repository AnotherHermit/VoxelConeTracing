///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

flat in uint domInd;

uniform vec3 diffColor;

uniform layout(RGBA32F) image2D frontView;
uniform layout(RGBA32F) image2D sideView;
uniform layout(RGBA32F) image2D topView;

void main()
{	
	// Set constant color for textureless models
	vec3 color = /*domDir; // diffColor; //*/ vec3(gl_FragCoord.z);

	if(domInd == 0) {
		imageStore(frontView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
	} else if (domInd == 1) {
		imageStore(sideView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
	} else {
		imageStore(topView, ivec2(gl_FragCoord.xy), vec4(color, 1.0f));
	}
}
