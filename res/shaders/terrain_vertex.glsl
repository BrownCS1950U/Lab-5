// terrain_vertex.glsl
#version 410 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

out vec3 VS_FragPos;
out vec3 VS_Normal;
out vec2 VS_UV;
out float VS_Height;
out float VS_Slope;

uniform mat4  uModel;
uniform mat4  uViewProj;

// height sampling
uniform sampler2D uHeightTex;
uniform float     uHeightScale;
uniform float     uTexel;      // = 1.0 / heightmapResolution

// NEW: uv-scaling
uniform vec2      uUVScale;

void main()
{
    // apply the UV scale here:
    VS_UV = aUV * uUVScale;

    // --- displacement from heightmap ---
    float h = texture(uHeightTex, VS_UV).r * uHeightScale;
    vec3  p = aPos + vec3(0.0, h, 0.0);
    VS_FragPos = vec3(uModel * vec4(p, 1.0));
    VS_Height  = h;

    // --- approximate normal from central difference ---
    float hl = texture(uHeightTex, VS_UV + vec2( uTexel, 0.0)).r * uHeightScale;
    float hr = texture(uHeightTex, VS_UV + vec2(-uTexel, 0.0)).r * uHeightScale;
    float hd = texture(uHeightTex, VS_UV + vec2(0.0,  uTexel)).r * uHeightScale;
    float hu = texture(uHeightTex, VS_UV + vec2(0.0, -uTexel)).r * uHeightScale;
    vec3 rawNormal = normalize(vec3(hl - hr, 2.0 * uTexel * uHeightScale, hd - hu));

    VS_Normal = normalize(mat3(transpose(inverse(uModel))) * rawNormal);

    // slope: 0 = flat, 1 = vertical
    VS_Slope = 1.0 - clamp(VS_Normal.y, 0.0, 1.0);

    gl_Position = uViewProj * vec4(VS_FragPos, 1.0);
}
