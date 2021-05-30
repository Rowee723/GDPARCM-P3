#version 430

in vec3 skybox_vertex;

out vec3 TexCoords;

uniform mat4 u_view, u_projection;

void main(void) {
	TexCoords = skybox_vertex;
	vec4 pos = u_projection * u_view * vec4(skybox_vertex, 1.0);
	gl_Position = pos.xyww;
}