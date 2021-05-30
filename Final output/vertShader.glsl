#version 430

in vec2 v_uv;
in vec3 v_vertex;
in vec3 v_color;
in vec3 v_normal;

//in vec2 TexCoords;

uniform mat4 u_model, u_view, u_projection;

//uniform sampler2D screenTexture;

out vec2 f_uv;
out vec3 f_vertex;
out vec3 f_color;
out vec3 f_normal;

//out vec4 FragColor;

void main(void) {
	gl_Position = u_projection * u_view* u_model * vec4(v_vertex, 1.0);
	f_vertex = vec3(u_model * vec4(v_vertex, 1.0));
	f_color = v_color;
	f_uv = v_uv;
	f_normal = mat3(u_model) * v_normal;

	//FragColor = texture(screenTexture, TexCoords);
}