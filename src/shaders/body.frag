#version 410 core

in vec3 vWorldPosition;
in vec3 vNormal;

uniform vec3 uColor;		// sRGB
uniform vec3 uLightDirection;	// from the light toward the scene
uniform vec3 uCameraPosition;

out vec4 fragColor;

void main()
{
	vec3 albedo = pow(uColor, vec3(2.2));	// to linear space
	vec3 n = normalize(vNormal);
	vec3 l = normalize(-uLightDirection);
	vec3 v = normalize(uCameraPosition - vWorldPosition);

	// Sky / ground hemispheric ambient, wrapped key light, soft specular, rim.
	vec3 ambient = mix(vec3(0.10, 0.09, 0.08), vec3(0.32, 0.36, 0.45), n.y * 0.5 + 0.5);
	float diffuse = max(dot(n, l) * 0.8 + 0.2, 0.0);
	float specular = pow(max(dot(n, normalize(l + v)), 0.0), 32.0) * 0.15;
	float rim = pow(1.0 - max(dot(n, v), 0.0), 3.0) * 0.25;

	vec3 color = albedo * (ambient + vec3(1.0, 0.96, 0.9) * diffuse) + vec3(specular + rim * 0.6);
	fragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);	// back to sRGB
}
