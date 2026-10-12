#version 410 core

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPosition;

const float HALF_SIZE = 30.0;
const vec2 CORNERS[6] = vec2[](
	vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(1.0, -1.0),
	vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

void main()
{
	vec2 corner = CORNERS[gl_VertexID] * HALF_SIZE;
	vWorldPosition = vec3(corner.x, 0.0, corner.y);
	gl_Position = uProjection * uView * vec4(vWorldPosition, 1.0);
}
