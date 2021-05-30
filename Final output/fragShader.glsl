#version 430

//layout (location = 0) in vec2 aPos;
//layout (location = 1) in vec2 aTexCoords;

in vec2 f_uv;
in vec3 f_color;
in vec3 f_vertex;
in vec3 f_normal;

uniform sampler2D u_texture;

uniform vec3 light_pos;
uniform vec3 camera_pos;

out vec4 fragColor;

//out vec2 TexCoords;

void main(void)
{    
    vec3 ambient_light = vec3(0.5f, 0.5f, 0.5f);

	vec3 diffuse_vertex = normalize(f_vertex - light_pos);
	vec3 diffuse_color = vec3( 1.0f, 1.0f, 1.0f);
	float diffuse = clamp(dot(diffuse_vertex, f_normal), 0, 1);
	vec3 diffuse_light = diffuse_color * diffuse;

	vec3 specular_vertex = normalize(light_pos - f_vertex);
	vec3 reflection = normalize(reflect(specular_vertex, normalize(f_normal)));
	vec3 view_vertex = normalize(f_vertex - camera_pos);
	float specular = pow(max(dot(view_vertex, reflection), 0), 30);
	vec3 specular_light = vec3(1.0f, 1.0f, 1.0f) * specular;

	//gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    //TexCoords = aTexCoords;

    vec4 texture_color = texture(u_texture, f_uv);
    fragColor = texture_color * (vec4(ambient_light, 1.0f) + vec4(diffuse_light, 1.0f) + vec4(specular_light, 1.0f));
}