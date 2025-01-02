#version 460 core

layout (quads, fractional_odd_spacing) in;

out vec3 normal;
out vec3 fragPos;

uniform mat4 view;
uniform mat4 proj;

vec4 b3(float t)
{
	return vec4
	(
		(1.f - t) * (1.f - t) * (1.f - t), 
		3.f * (1.f - t) * (1.f - t) * t, 
		3.f * (1.f - t) * t * t, 
		t * t * t
	);
};

vec4 db3(float t)
{
	return vec4
	(
		-3.f * (1.f - t) * (1.f - t), 
		3.f * (1.f - t) * (1.f - t) - 6.f * t * (1.f - t), 
		6.f * t * (1.f - t) - 3.f * t * t, 
		3.f * t * t
	);
};

vec4 calc_bezier(vec4 b3u, vec4 b3v)
{
	return vec4(b3u.x * (b3v.x * gl_in[0].gl_Position + b3v.y * gl_in[4] .gl_Position + b3v.z * gl_in[8].gl_Position + b3v.w * gl_in[12].gl_Position) +
			   b3u.y * (b3v.x * gl_in[1].gl_Position + b3v.y * gl_in[5].gl_Position + b3v.z * gl_in[9].gl_Position + b3v.w * gl_in[13].gl_Position) +
			   b3u.z * (b3v.x * gl_in[2].gl_Position + b3v.y * gl_in[6].gl_Position + b3v.z * gl_in[10].gl_Position + b3v.w * gl_in[14].gl_Position) +
			   b3u.w * (b3v.x * gl_in[3].gl_Position + b3v.y * gl_in[7].gl_Position + b3v.z * gl_in[11].gl_Position + b3v.w * gl_in[15].gl_Position));
};

void main()
{
    vec2 sub_uv = gl_TessCoord.xy;

    vec4 pos = calc_bezier(b3(sub_uv.x), b3(sub_uv.y));
	fragPos = (pos / pos.w).xyz;

	vec3 tangent = normalize(calc_bezier(db3(sub_uv.x), b3(sub_uv.y))).xyz;
	vec3 bitangent = normalize(calc_bezier(b3(sub_uv.x), db3(sub_uv.y))).xyz;
	normal = normalize(cross(tangent, bitangent));

    gl_Position = proj * view * pos;
}