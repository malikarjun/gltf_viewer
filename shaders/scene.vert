#version 410 core
layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normal_matrix;

out vec3 normal;
out vec3 position;
out vec2 texcoord;

void main(){
    gl_Position = mvp * vec4(in_vertex, 1.0);

    normal = normalize(vec3(normal_matrix * vec4(in_normal, 0.0)));

//    vec4 position_homo = model_view * vec4(in_vertex, 1.0);
//    position = position_homo.xyz/position_homo.w;

    position = vec3(model * vec4(in_vertex, 1.0));
    texcoord = in_texcoord;
}