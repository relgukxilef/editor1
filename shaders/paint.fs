#version 410 core

in vec2 gs_parameter;
in float gs_u;

out vec4 color;

float rand(float x) {
    uint seed = floatBitsToUint(x);
    seed = seed ^ (seed >> 3) ^ (seed >> 7) ^ (seed >> 11);
    return float(seed % 289) / 289.0;
}

vec2 rand(vec2 x) {
    uvec2 seed = floatBitsToUint(x);
    seed = seed ^ (seed >> 3) ^ (seed >> 7) ^ (seed >> 11);
    return vec2(seed % 289) / 289.0;
}

float perlin(float x) {
    float a = floor(x);
    float b = a + 1;
    float t = x - a;

    float t2 = t*t;
    float t3 = t2*t;

    return (
        (rand(a) * 2 - 1) * (t3 - 2*t2 + t) +
        (rand(b) * 2 - 1) * (t3 - t2)
    ) * 4;
}

float perlin(vec2 p) {
    float a = floor(p.x);
    float b = a + 1;
    float t = p.x - a;

    float t2 = t*t;
    float t3 = t2*t;

    float p_a = perlin(p.y + a * 10);
    float p_b = perlin(p.y + b * 10);
    float m_a = perlin(p.y + a * 10 + 10);
    float m_b = perlin(p.y + b * 10 + 10);

    return (
        p_a * (2*t3 - 3*t2 + 1) +
        p_b * (-2*t3 + 3*t2) +
        m_a * (t3 - 2*t2 + t) +
        m_b * (t3 - t2)
    );
}

void main(void) {
    float p = perlin(vec2(
        (gs_parameter.x + gs_parameter.y * 10) * 6,
        gs_u * 30
    ));

    float a = p - (gs_u - 0.5) * (gs_u - 0.5) * 8;

    if (a < -1)
        discard;

    float r = gs_parameter.y * 3 - 1 + rand(gs_parameter.y + 1) * 0.15;
    color =
        vec4(mix(vec3(0.39, 0.71, 0.98), vec3(0.21, 0.50, 1.0), r), 1);

    //float g = dFdx(p) - dFdy(p);
    //color *= g * 2 + 1;

    //color *= (clamp(a, -1.0, -0.8) + 0.8) * 1 + 1;
}
