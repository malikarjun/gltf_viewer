#version 410 core
in vec3 normal;
in vec3 position;
in vec2 texcoord;

uniform int textured;

uniform vec3 basecolor;
uniform sampler2D basecolor_tex;
uniform samplerCube depth_map;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform float far_plane;

out vec4 out_color;


// array of offset direction for sampling
vec3 grid_sampling_disk[20] = vec3[]
(
vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 frag_pos)
{
    // get vector between fragment position and light position
    vec3 frag_to_light = frag_pos - light_position;

    float current_depth = length(frag_to_light);
    // test for shadows
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float view_distance = length(view_pos - frag_pos);
    float disk_radius = (1.0 + (view_distance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closest_depth = texture(depth_map, frag_to_light + grid_sampling_disk[i] * disk_radius).r;
        closest_depth *= far_plane;   // undo mapping [0;1]
        if(current_depth - bias > closest_depth)
        shadow += 1.0;
    }
    shadow /= float(samples);

    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    

    return shadow;
}

void main() {

    vec3 color;
    if (textured == 1) {
        color = texture(basecolor_tex, texcoord).xyz;
    } else {
        color = basecolor;
    }

    vec3 normal = normalize(normal);
    // ambient
    vec3 ambient = 0.3 * light_color;
    // diffuse
    vec3 light_dir = normalize(light_position - position);
    float diff = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff * light_color;
    // specular
    vec3 view_dir = normalize(view_pos - position);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = 0.0;
    vec3 halfway_dir = normalize(light_dir + view_dir);
    spec = pow(max(dot(normal, halfway_dir), 0.0), 64.0);
    vec3 specular = spec * light_color;

    // calculate shadow
    float shadow = ShadowCalculation(position);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular) ) * color;

    out_color = vec4(lighting, 1.0);

}