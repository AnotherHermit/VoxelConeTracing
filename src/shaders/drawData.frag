///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
in vec4 exPosition;
in vec2 exTexCoords;
in vec3 exTangent;
in vec3 exBiTangent;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outTangent;
layout(location = 4) out vec4 outBiTangent;

layout(location = 0) uniform vec3 diffColor;
layout(location = 1) uniform sampler2D diffuseUnit;
layout(location = 2) uniform sampler2D maskUnit;

subroutine vec4 SampleColor();

layout(index = 0) subroutine(SampleColor) 
vec4 DiffuseColor() {
	return vec4(diffColor, 1.0f);
}

layout(index = 1) subroutine(SampleColor)
vec4 TextureColor() {
	return vec4(texture(diffuseUnit, exTexCoords).rgb, 1.0f);
}

layout(index = 2) subroutine(SampleColor)
vec4 MaskColor() {
	return vec4(texture(diffuseUnit, exTexCoords).rgb, texture(maskUnit, exTexCoords).r);
}

layout(location = 0) subroutine uniform SampleColor GetColor;

void main()
{	
	outNormal = vec4(normalize(exNormal), 1.0f);
	outTangent = vec4(normalize(exTangent), 1.0f);
	outBiTangent = vec4(normalize(exBiTangent), 1.0f);
	outPosition = exPosition / exPosition.w;
	outColor = GetColor();
}