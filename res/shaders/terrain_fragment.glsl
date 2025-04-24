// terrain_fragment.glsl
#version 410 core

in  vec3  VS_FragPos;
in  vec3  VS_Normal;
in  vec2  VS_UV;
in  float VS_Height;
in  float VS_Slope;

out vec4 FragColor;

// height thresholds
uniform float uWaterLevel;
uniform float uRockLine;
uniform float uSnowLine;

// NEW: blend-width uniform
uniform float uBlendW;

// lighting
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uAmbient;

void main()
{
    // use the dynamic blend width
    float T = uBlendW;

    // palettes
    const vec3 waterCol = vec3(0.0, 0.20, 0.70);
    const vec3 grassCol = vec3(0.05, 0.50, 0.15);
    const vec3 rockCol  = vec3(0.45);
    const vec3 snowCol  = vec3(1.0);

    // height blends
    float waterBlend = 1.0 - smoothstep(uWaterLevel,          uWaterLevel + T, VS_Height);
    float grassBlend = smoothstep(uWaterLevel + T,           uRockLine - T,   VS_Height)
    * (1.0 - smoothstep(uRockLine - T,      uRockLine + T,   VS_Height));
    float rockBlend  = smoothstep(uRockLine - T,             uRockLine + T,   VS_Height)
    * (1.0 - smoothstep(uSnowLine - T,      uSnowLine + T,   VS_Height));
    float snowBlend  = smoothstep(uSnowLine - T,             uSnowLine + T,   VS_Height);

    // slope-based rock override
    float slopeRock = smoothstep(0.35, 0.75, VS_Slope);
    rockBlend  = max(rockBlend, slopeRock);
    grassBlend = grassBlend * (1.0 - slopeRock);

    vec3 base = waterBlend * waterCol
    + grassBlend * grassCol
    + rockBlend  * rockCol
    + snowBlend  * snowCol;

    // lighting: diffuse + simple AO
    vec3 N = normalize(VS_Normal);
    vec3 L = normalize(-uSunDir);
    float diff = max(dot(N, L), 0.0);
    float ao   = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 lighting = uAmbient * ao + diff * uSunColor;

    vec3 colour = pow(base * lighting, vec3(1.0 / 2.2));
    FragColor = vec4(colour, 1.0);
}
