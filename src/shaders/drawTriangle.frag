///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430
#define M_PI 3.1415926535897932384626433832795

in vec2 exTexCoords;
out vec4 outColor;

layout(binding = 2) uniform usampler2DArray voxelTextures;
layout(binding = 3) uniform usampler3D voxelData;
layout(binding = 5) uniform sampler2DShadow shadowMap;
layout(binding = 6) uniform sampler2DArray sceneTex;
layout(binding = 7) uniform sampler2D sceneDepth;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 0) uniform CameraBuffer {
	Camera cam;
};

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	mat4 MTShadowMatrix;
	vec3 lightDir;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
	uint numMipLevels;
	uint mipLevel;
};

layout (std140, binding = 1) uniform SceneBuffer {
	SceneParams scene;
};

struct VoxelData {
	vec4 color;
	uint light;
	uint count;
};

VoxelData unpackARGB8(uint bytesIn) {
	VoxelData data;
	uvec3 uiColor;

	// Put a first to improve max operation but it should not be very noticable
	data.light = (bytesIn & 0xF0000000) >> 28;
	data.count = (bytesIn & 0x0F000000) >> 24;
	uiColor.r =  (bytesIn & 0x00FF0000) >> 16;
	uiColor.g =  (bytesIn & 0x0000FF00) >> 8;
	uiColor.b =  (bytesIn & 0x000000FF);


	float div[] = { 0.0f, 1.0f / float(data.count) };
	int index = int(data.count > 0);

	data.color.rgb = vec3(uiColor) / 31.0f * div[index];
	data.color.a = float(sign(data.count));

	return data;
}

//subroutine vec4 DrawTexture();

//layout(index = 0) subroutine(DrawTexture) 
vec4 ProjectionTexture() {
	return unpackARGB8(texture(voxelTextures, vec3(exTexCoords, float(scene.view))).r).color;
}

//layout(index = 1) subroutine(DrawTexture)
vec4 VoxelTexure() {
	float size = float(scene.voxelRes >> scene.mipLevel);
	float depth = float(scene.voxelLayer) / size;
	return unpackARGB8(textureLod(voxelData, vec3(exTexCoords, depth), scene.mipLevel).r).color;
}

//layout(index = 2) subroutine(DrawTexture)
//vec4 ShadowTexture() {
//	return texture(shadowMapTemp, exTexCoords);
//}

float SceneShadow(vec3 texCoords) {
    return texture(shadowMap, texCoords);
}

//layout(index = 3) subroutine(DrawTexture)
vec4 SceneColor() {
	return texture(sceneTex, vec3(exTexCoords, 0.0f));
}

//layout(index = 4) subroutine(DrawTexture)
vec4 ScenePosition() {
	return texture(sceneTex, vec3(exTexCoords, 1.0f));
}

//layout(index = 5) subroutine(DrawTexture)
vec4 SceneNormal() {
	vec4 normal = texture(sceneTex, vec3(exTexCoords, 2.0f));
	//normal.xyz /= 2;
	//normal.xyz += vec3(0.5f);
	return normal;
}

//layout(index = 6) subroutine(DrawTexture)
vec4 SceneTangent() {
	vec4 tangent = texture(sceneTex, vec3(exTexCoords, 3.0f));
	//tangent.xyz /= 2;
	//tangent.xyz += vec3(0.5f);
	return tangent;
}

//layout(index = 7) subroutine(DrawTexture)
vec4 SceneBiTangent() {
	vec4 biTangent = texture(sceneTex, vec3(exTexCoords, 4.0f));
	//biTangent.xyz /= 2;
	//biTangent.xyz += vec3(0.5f);
	return biTangent;
}

//layout(index = 8) subroutine(DrawTexture)
vec4 SceneDepth() {
	return texture(sceneDepth, exTexCoords);
}

//layout(location = 0) subroutine uniform DrawTexture SampleTexture;

vec3 BasicLight(vec3 p, vec3 n){
    vec3 retLight;
    vec3 l = scene.lightDir;
    vec3 r = 2.0f * n * dot(l, n) - l;
    p = p * 2.0f + vec3(-1.0f,-1.0f,-1.0f);
    vec3 c = cam.position;
    vec3 v = normalize(c - p);
    retLight.x = 1.0f;
    retLight.y = max(dot(l,n), 0.0f);
    retLight.z = pow(max(dot(r,v), 0.0f), 5.0f);
    return retLight;
}

vec4 voxelSampleLevel(vec3 position, float level) {
	float mip = round(level);
	float scale = float(scene.voxelRes >> int(mip));
	position *= scale;
	position = position - vec3(0.5f);
	vec3 positionWhole = floor(position);
	vec3 intpol = position - positionWhole;
	vec3 offsets[] = {  vec3(0.0f, 0.0f, 0.0f),
						vec3(0.0f, 0.0f, 1.0f),
						vec3(0.0f, 1.0f, 0.0f),
						vec3(0.0f, 1.0f, 1.0f),
						vec3(1.0f, 0.0f, 0.0f),
						vec3(1.0f, 0.0f, 1.0f),
						vec3(1.0f, 1.0f, 0.0f),
						vec3(1.0f, 1.0f, 1.0f) };
	
	//vec4 total = vec4(0.0f);
	VoxelData total = {vec4(0.0f), 0, 0 };
	float count = 1.0f;
	for(int i = 0; i < 8; i++) {
		vec3 voxelPos = (positionWhole + offsets[i]) / scale;
		vec3 offset = mix(1.0f - offsets[i], offsets[i], intpol);
        float factor = offset.x * offset.y * offset.z;

        VoxelData voxel = unpackARGB8(textureLod(voxelData, voxelPos, mip).r);
        if(any(greaterThan(voxelPos, vec3(1.0f))) || any(lessThan(voxelPos, vec3(0.0f)))) {
//            count -= factor;
            factor = 0.0f;
        }

		total.color.rgb += voxel.color.rgb * factor * float(sign(int(voxel.light)));
		total.color.a += voxel.color.a * factor;
	}

    vec4 result[] = {vec4(0.0f), total.color / count};
    int index = int(count > 0.01f);

	return result[index];
}

vec4 voxelSampleBetween(vec3 position, float level) {
	float levelLow = floor(level);
	float levelHigh = levelLow + 1.0f;
	float intPol = level - levelLow;

	vec4 voxelLow = voxelSampleLevel(position, levelLow);
	vec4 voxelHigh = voxelSampleLevel(position, levelHigh);

	return mix(voxelLow, voxelHigh, intPol);
}

vec4 ConeTrace(vec3 startPos, vec3 dir, float coneRatio, float maxDist,	float voxelSize) {

	vec4 accum = vec4(0.0f);
	vec3 samplePos;
	vec4 sampleValue;
	float sampleRadius;
	float sampleDiameter;
	float sampleWeight;
	float sampleLOD = 0.0f;
	
	for(float dist = voxelSize; dist < maxDist && accum.a < 1.0f;) {
		sampleRadius = coneRatio * dist;
		sampleDiameter = max(2.0f * sampleRadius, voxelSize);
		sampleLOD = log2(sampleDiameter * float(scene.voxelRes));
		samplePos = startPos + dir * (dist + sampleRadius);
		sampleValue = voxelSampleBetween(samplePos, sampleLOD);
		sampleWeight = 1.0f - accum.a;
		accum += sampleValue * sampleWeight;
		dist += sampleRadius;
	}

	return accum;
}

vec4 ConeTrace60(vec3 startPos, vec3 dir, float aoDist, float maxDist, float voxelSize) {
	
	vec4 accum = vec4(0.0f);
	float opacity = 0.0f;
	vec3 samplePos;
	vec4 sampleValue;
	float sampleWeight;
	float sampleLOD = 0.0f;
	
	for(float dist = voxelSize; dist < maxDist && accum.a < 1.0f;) {
		samplePos = startPos + dir * dist;
		sampleValue = voxelSampleLevel(samplePos, sampleLOD);
		sampleWeight = 1.0f - accum.a;
		accum += sampleValue * sampleWeight;
		opacity = (dist < aoDist) ? accum.a : opacity;
		sampleLOD += 1.0f;
		dist *= 2.0f;
	}

	return vec4(accum.rgb, 1.0f - opacity);
}

vec4 DiffuseTrace (vec3 p, vec3 n, vec3 t, vec3 b, float d) {
	vec3 dir[] = {  vec3( 0.000000f, 1.000000f,  0.000000f), 
					vec3( 0.000000f, 0.500000f,  0.866025f), 
					vec3( 0.823639f, 0.500000f,  0.267617f), 
					vec3( 0.509037f, 0.500000f, -0.700629f), 
					vec3(-0.509037f, 0.500000f, -0.700629f), 
					vec3(-0.823639f, 0.500000f,  0.267617f) };

	float weight[2];
	weight[0] = 0.25f;
	weight[1] = (1.0f - weight[0]) / 5.0f;
	
	float voxelSize = 2.0f / float(scene.voxelRes);
	float maxDistance = d;
	float aoDistance = min(0.03f, maxDistance);

	p += n * voxelSize;

	vec4 total = vec4(0.0f);
	for(int i = 0; i < 6; i++) {
		vec3 direction = dir[i].x * t + dir[i].y * n + dir[i].z * b;
		total += weight[int(i != 0)] * ConeTrace60(p, direction, aoDistance, maxDistance, voxelSize);
	}

	return total;
}

vec4 AngleTrace(vec3 dir, float theta) {
	float voxelSize = 1.0f / float(scene.voxelRes);
	float maxDistance = sqrt(3.0f);
	
	vec3 pos = ScenePosition().xyz;
	vec3 norm = SceneNormal().xyz;
	pos += norm * voxelSize * 2.0f;

	
	float halfTheta = sin(radians(theta)/2.0f);
	float coneRatio = halfTheta / (1.0f - halfTheta);
	return ConeTrace(pos, dir, coneRatio, maxDistance, voxelSize);
}

// Shadows

float ShadowMapping(vec3 p, vec3 n) {
    p = p * 2.0f + vec3(-1.0f, -1.0f, -1.0f);
    vec4 s = scene.MTShadowMatrix * vec4(p, 1.0f);
    s = s * 0.5f + 0.5f;

    vec3 l = scene.lightDir;
    float cosTheta = max(dot(l,n), 0.0f);
    float b = 0.005f*tan(acos(cosTheta));
    b = clamp(b, 0.0f,0.02f);
    s.z = s.z - b;

    return SceneShadow(s.xyz);
}

vec4 SoftShadows() {
	vec4 result = AngleTrace(scene.lightDir, 5.0f);
	vec4 color = SceneColor();
	return vec4(color.rgb * (1.0f - result.a), 1.0f);
}

// Scenes

subroutine vec4 DrawScene();

layout(index = 0) subroutine(DrawScene)
vec4 Basic() {
    vec3 p = ScenePosition().xyz;
    vec3 n = SceneNormal().xyz;

    if (length(n) < 0.5f) {
        return vec4(0.0f);
    }

    vec4 c = SceneColor();
    vec3 l = BasicLight(p, n);

    c.xyz *= 0.8f * l.x; // Ambient
    c.xyz += 0.5f * l.y; // Diffuse
    c.xyz += 0.2f * l.z; // Speclar

    return c;

}

layout(index = 1) subroutine(DrawScene)
vec4 BasicShadows() {
    vec3 p = ScenePosition().xyz;
    vec3 n = SceneNormal().xyz;

    if (length(n) < 0.5f) {
        return vec4(0.0f);
    }

    vec4 c = SceneColor();
    vec3 l = BasicLight(p, n);
    float s = 1.0f - 0.5f * ShadowMapping(p, n);

    c.xyz *= 0.8f * l.x; // Ambient
    c.xyz += 0.5f * l.y * s; // Diffuse
    c.xyz += 0.2f * l.z * s; // Speclar

    return c;
}

layout(index = 2) subroutine(DrawScene)
vec4 BasicAOShadows() {
    vec3 p = ScenePosition().xyz;
    vec3 n = SceneNormal().xyz;

    if (length(n) < 0.5f) {
        return vec4(0.0f);
    }

    vec3 t = SceneTangent().xyz;
	vec3 b = SceneBiTangent().xyz;
    float m = 0.03f;
    vec4 c = SceneColor();
    vec3 l = BasicLight(p, n);
    float s = 1.0f - 0.5f * ShadowMapping(p, n);
    vec4 d = DiffuseTrace(p, n, t, b, m);

    c.xyz *= (0.4f * l.x + 0.4f * d.w); // Ambient
    c.xyz += 0.5f * l.y * s; // Diffuse
    c.xyz += 0.2f * l.z * s; // Speclar

    return c;
}

layout(index = 3) subroutine(DrawScene)
vec4 GIAOShadows() {
    vec3 p = ScenePosition().xyz;
    vec3 n = SceneNormal().xyz;

    if (length(n) < 0.5f) {
        return vec4(0.0f);
    }

    vec3 t = SceneTangent().xyz;
	vec3 b = SceneBiTangent().xyz;
    float m = 1.0f;
    vec4 c = SceneColor();
    vec3 l = BasicLight(p, n);
    float s = 1.0f - 0.5f * ShadowMapping(p, n);
    vec4 d = DiffuseTrace(p, n, t, b, m);

    c.xyz += 0.5f * d.xyz;
    c.xyz *= (0.4f * l.x + 0.4f * d.w); // Ambient
    c.xyz += 0.5f * l.y * s; // Diffuse
    c.xyz += 0.2f * l.z * s; // Speclar

    return c;
}

layout(index = 4) subroutine(DrawScene)
vec4 GIAOSoftShadows() {
    vec3 p = ScenePosition().xyz;
    vec3 n = SceneNormal().xyz;

    if (length(n) < 0.5f) {
        return vec4(0.0f);
    }

    vec3 t = SceneTangent().xyz;
    vec3 b = SceneBiTangent().xyz;
    float m = 1.0f;
    vec4 c = SceneColor();
    vec3 l = BasicLight(p, n);
    float s = 1.0f - AngleTrace(scene.lightDir, 5.0f).a;
    vec4 d = DiffuseTrace(p, n, t, b, m);

    c.xyz += 0.5f * d.xyz;
    c.xyz *= (0.4f * l.x + 0.4f * d.w); // Ambient
    c.xyz += 0.5f * l.y * s; // Diffuse
    c.xyz += 0.2f * l.z * s; // Speclar

    return c;
}

layout(location = 0) subroutine uniform DrawScene SampleScene;

void main()
{	
	outColor = SampleScene();
}
