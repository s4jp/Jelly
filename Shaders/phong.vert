#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;

uniform mat4 view;
uniform mat4 proj;
uniform vec3 controlPoints[64];
uniform bool interpolateNormals;
uniform float minMaxValues[6];

vec3 rescale(vec3 pos) {
    float xDiff = minMaxValues[1] - minMaxValues[0];
    float yDiff = minMaxValues[3] - minMaxValues[2];
    float zDiff = minMaxValues[5] - minMaxValues[4];

    float diff = xDiff;
    if (yDiff > diff) diff = yDiff;
    if (zDiff > diff) diff = zDiff;

    vec3 dir = normalize(aPos - vec3(0.5f));
    float len = length(aPos - vec3(0.5f));
    vec3 newPos = vec3(0.5f) + dir * len / diff;

    return newPos;
}

float bernstein3(int i, float t) {
    if (i == 0) return (1.0 - t) * (1.0 - t) * (1.0 - t);
    if (i == 1) return 3.0 * t * (1.0 - t) * (1.0 - t);
    if (i == 2) return 3.0 * t * t * (1.0 - t);
    if (i == 3) return t * t * t;
    return 0.0;
}

vec3 bezierTransform(vec3 pos) {
    vec3 result = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        float Bu = bernstein3(i, pos.z);
        for (int j = 0; j < 4; j++) {
            float Bv = bernstein3(j, pos.y);
            for (int k = 0; k < 4; k++) {
                float Bw = bernstein3(k, pos.x);
                int index = i * 16 + j * 4 + k;
                result += controlPoints[index] * Bu * Bv * Bw;
            }
        }
    }
    return result;
}

//float bernstein3_derivative(int i, float t) {
//    if (i == 0) return -3.0 * (1.0 - t) * (1.0 - t);
//    if (i == 1) return 3.0 * (1.0 - 3.0 * t) * (1.0 - t);
//    if (i == 2) return 3.0 * t * (2.0 - 3.0 * t);
//    if (i == 3) return 3.0 * t * t;
//    return 0.0;
//}

//vec3 calculateNormal(float u, float v, float w) {
//    vec3 dP_du = vec3(0.0);
//    vec3 dP_dv = vec3(0.0);
//    vec3 dP_dw = vec3(0.0);
//
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            for (int k = 0; k < 4; k++) {
//                int index = i * 16 + j * 4 + k;
//
//                float B_i = bernstein3(i, u);
//                float B_j = bernstein3(j, v);
//                float B_k = bernstein3(k, w);
//
//                float dB_i = bernstein3_derivative(i, u);
//                float dB_j = bernstein3_derivative(j, v);
//                float dB_k = bernstein3_derivative(k, w);
//
//                dP_du += controlPoints[index] * dB_i * B_j * B_k;
//                dP_dv += controlPoints[index] * B_i * dB_j * B_k;
//                dP_dw += controlPoints[index] * B_i * B_j * dB_k;
//            }
//        }
//    }
//
//    vec3 T1 = cross(dP_du, dP_dv);
//    vec3 N = normalize(cross(T1, dP_dw));
//
//    return N;
//}

void main()
{
    vec3 transformedOGpos = bezierTransform(aPos);
    vec3 newPos = rescale(aPos);
	vec3 transformedPos = bezierTransform(newPos);
	vec4 position = vec4(transformedPos, 1.0);
    vec4 worldPos = position;
    fragPos = worldPos.xyz / worldPos.w;
    gl_Position = proj * view * worldPos;
    //normal = interpolateNormals ? calculateNormal(transformedPos.z, transformedPos.y, transformedPos.x) : aNormal;
    normal = interpolateNormals ? normalize(transformedPos - transformedOGpos) : aNormal;
}