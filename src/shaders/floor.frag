#version 410 core

in vec3 vWorldPosition;

uniform vec3 uCameraPosition;
uniform vec3 uBackground;

out vec4 fragColor;

// Anti-aliased grid line intensity for a given cell size.
float grid(vec2 position, float cell)
{
	vec2 coord = position / cell;
	vec2 width = fwidth(coord);
	vec2 line = abs(fract(coord - 0.5) - 0.5) / width;
	return 1.0 - min(min(line.x, line.y), 1.0);
}

void main()
{
	vec2 p = vWorldPosition.xz;
	vec3 color = vec3(0.17, 0.18, 0.21);
	color = mix(color, vec3(0.26, 0.27, 0.31), grid(p, 0.25) * 0.5);
	color = mix(color, vec3(0.36, 0.37, 0.42), grid(p, 1.0));

	// Fade into the background with distance from the centre of the stage.
	float fade = smoothstep(4.0, 14.0, length(p));
	fragColor = vec4(mix(color, uBackground, fade), 1.0);
}
