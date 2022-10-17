#include <glad.h>
#include <GLFW/glfw3.h>
#include "tiny_gltf.h"
#include "shaders.h"
#include "window.h"
#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

// default eye pos, lookat pos, up
const glm::vec3 EYE = glm::vec3(3, 5, 13);
const glm::vec3 LOOK_AT = glm::vec3(0, 0, 0);
const glm::vec3 UP = glm::vec3(0, 1, 0);


struct MaterialTex {
    // texture ids
    GLuint emissiveId;
    GLuint normalId;
    GLuint occlusionId;
    GLuint baseColorId;
    GLuint metallicRoughnessId;

    // constant colors
    glm::vec3 basecolor;

    MaterialTex() : emissiveId(0), normalId(0), occlusionId(0), baseColorId(0), metallicRoughnessId(0) {}
};

struct TransformationMat {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

Camera camera;


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    camera.processInput(window);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    camera.processWindowResize(width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.processCursorPos(xpos, ypos);
}


bool loadModel(tinygltf::Model &model, const char *filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}

int createTexture(tinygltf::Model &model, int texIndex) {

    tinygltf::Texture &tex = model.textures[texIndex];

    GLuint texid;
    glGenTextures(1, &texid);

    tinygltf::Image &image = model.images[tex.source];
    tinygltf::Sampler sampler = model.samplers[tex.sampler];

    glBindTexture(GL_TEXTURE_2D, texid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (sampler.minFilter != -1)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
    if (sampler.magFilter != -1)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);

    GLenum format = GL_RGBA;

    if (image.component == 1) {
        format = GL_RED;
    } else if (image.component == 2) {
        format = GL_RG;
    } else if (image.component == 3) {
        format = GL_RGB;
    } else {
        // ???
    }

    GLenum type = GL_UNSIGNED_BYTE;
    if (image.bits == 8) {
        // ok
    } else if (image.bits == 16) {
        type = GL_UNSIGNED_SHORT;
    } else {
        // ???
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                 format, type, &image.image.at(0));
    glGenerateMipmap(GL_TEXTURE_2D);
    return texid;
}



void bindMesh(std::vector<MaterialTex> &matTexs, std::vector<GLuint> &vaos, tinygltf::Model &model,
              tinygltf::Mesh &mesh) {

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
        tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
        tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        vaos.push_back(vao);

        unsigned int ebo;
        glGenBuffers(1, &ebo);

        glBindBuffer(indexBufferView.target, ebo);
        glBufferData(indexBufferView.target, indexBufferView.byteLength,
                     &indexBuffer.data.at(0) + indexBufferView.byteOffset, GL_STATIC_DRAW);


        for (auto &attrib : primitive.attributes) {
            const tinygltf::Accessor accessor = model.accessors[attrib.second];
            const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(bufferView.target, vbo);


            glBufferData(bufferView.target, bufferView.byteLength,
                         &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

            int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int vaa = -1;
            if (attrib.first.compare("POSITION") == 0) vaa = 0;
            if (attrib.first.compare("NORMAL") == 0) vaa = 1;
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
            if (vaa > -1) {
                glVertexAttribPointer(vaa, size, accessor.componentType,
                                      accessor.normalized ? GL_TRUE : GL_FALSE,
                                      byteStride, BUFFER_OFFSET(accessor.byteOffset));
                glEnableVertexAttribArray(vaa);
            } else
                std::cout << "vaa missing: " << attrib.first << std::endl;
        }

        MaterialTex matTex;
        matTex.baseColorId = 0;
        matTexs.push_back(matTex);

        tinygltf::Material material = model.materials[primitive.material];

        if (material.pbrMetallicRoughness.baseColorTexture.index != -1) {
            matTexs.back().baseColorId = createTexture(
                model, material.pbrMetallicRoughness.baseColorTexture.index);
        } else {
            std::vector<double> basecolorFactor = material.pbrMetallicRoughness.baseColorFactor;
            matTexs.back().basecolor = glm::vec3(basecolorFactor[0], basecolorFactor[1], basecolorFactor[2]);
        }

    }
}

// bind models
void bindModelNodes(std::vector<MaterialTex> &matTexs, std::vector<GLuint> &vaos, tinygltf::Model &model,
                    tinygltf::Node &node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        bindMesh(matTexs, vaos, model, model.meshes[node.mesh]);
    }
}

std::pair<std::vector<GLuint>, std::vector<MaterialTex>> bindModel(tinygltf::Model &model) {
    std::vector<GLuint> vaos;
    std::vector<MaterialTex> matTexs;
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {

        // skip light nodes
        if (!model.nodes[i].extensions.empty()) continue;

        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
        bindModelNodes(matTexs, vaos, model, model.nodes[scene.nodes[i]]);
    }


    return {vaos, matTexs};
}

glm::mat4 createTranslationMatrix(std::vector<double> trans) {
    glm::mat4 transMat;
    if (trans.size() > 0) {
        transMat =  glm::translate(glm::mat4(1.0f),  glm::vec3(trans[0], trans[1], trans[2]));
    } else {
        transMat =  glm::mat4(1.0f);
    }
    return transMat;

}

glm::mat4 createRotationMatrix(std::vector<double> rot) {
    glm::mat4 rotMat;
    if (rot.size() > 0) {
        glm::quat quat = glm::quat(rot[3], rot[0], rot[1], rot[2]);
        rotMat =  glm::mat4_cast(quat);
    } else {
        glm::quat quat = glm::quat(1, 0, 0, 0);
        rotMat =  glm::mat4_cast(quat);
//        rotMat = glm::mat4(1.0f);
    }
    return rotMat;

}

glm::mat4 createScaleMatrix(std::vector<double> scale) {
    glm::mat4 scaleMat;
    if (scale.size() > 0) {
        scaleMat = glm::scale(glm::mat4(1.0f),  glm::vec3(scale[0], scale[1], scale[2]));
    } else {
        scaleMat = glm::mat4(1.0f);
    }
    return scaleMat;
}

glm::mat4 createModelMatrix(tinygltf::Node node) {
    glm::mat4 translationMatrix = createTranslationMatrix(node.translation);
    glm::mat4  rotationMatrix = createRotationMatrix(node.rotation);
    glm::mat4 scaleMatrix = createScaleMatrix(node.scale);
    glm::mat4 model = translationMatrix * rotationMatrix * scaleMatrix;
    return model;
}


void drawMesh(Shaders &shader,
              int &vaoId,
              const std::vector<GLuint>& vaos,
              const std::vector<MaterialTex>& matTexs,
              TransformationMat& transMat,
              tinygltf::Model &model,
              tinygltf::Mesh &mesh) {
    glm::mat4 mvp = transMat.proj * transMat.view * transMat.model;
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(transMat.model));

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        glBindVertexArray(vaos[vaoId]);

        if (shader.type == ShaderType::SCENE) {
            // to regular shader
            // ---------------------------------------------------------------
            shader.setMat4("mvp", mvp);
            shader.setMat4("normal_matrix", normalMatrix);

            int textured;
            if (matTexs[vaoId].baseColorId > 0) {

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, matTexs[vaoId].baseColorId);
                textured = 1;
            } else {
                glm::vec3 basecolor = matTexs[vaoId].basecolor;
                shader.setVec3("basecolor", basecolor);
                textured = 0;
            }
            shader.setInt("textured", textured);
            // ---------------------------------------------------------------
        }
        shader.setMat4("model", transMat.model);

        vaoId++;

        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        glDrawElements(primitive.mode, indexAccessor.count,
                       indexAccessor.componentType,
                       BUFFER_OFFSET(indexAccessor.byteOffset));
    }
}

void drawModelNodes(Shaders &shader, int &vaoId,
                    const std::vector<GLuint>& vaos, const std::vector<MaterialTex>& matTexs,
                    TransformationMat& transMat,
                    tinygltf::Model &model,
                    tinygltf::Node &node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {

        glm::mat4 modelMatrix = createModelMatrix(node);
        transMat.model = modelMatrix;
        drawMesh(shader, vaoId, vaos, matTexs, transMat, model, model.meshes[node.mesh]);
    }
}
void drawModel(Shaders &shader,
               const std::vector<GLuint>& vaos,
               const std::vector<MaterialTex>& matTexs,
               TransformationMat& transMat,
               tinygltf::Model &model) {
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    int vaoId = 0;
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        drawModelNodes(shader, vaoId, vaos, matTexs, transMat, model, model.nodes[scene.nodes[i]]);
    }
}



glm::vec3 setUpLighting(TransformationMat &transMat, tinygltf::Model &model, Shaders &shader) {
    glm::vec3 lightWorldPos;
    for(auto node : model.nodes) {
        // skip non-light nodes
        if (node.extensions.empty()) continue;

        int lightId = node.extensions["KHR_lights_punctual"].Get("light").GetNumberAsInt();
        tinygltf::Light light = model.lights[lightId];

        // skip directional light
        if (light.type != "point") continue;

        glm::mat4 modelMatrix = createModelMatrix(node);
        // TODO @mswamy check: do we need perspective divide here?
        lightWorldPos = modelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

//        std::cout << glm::to_string(lightWorldPos) << std::endl;
//        std::cout << glm::to_string(modelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) << std::endl;

        glm::vec3 color = glm::vec3(light.color[0], light.color[1], light.color[2]);

        shader.setVec3("light_position", lightWorldPos);
        shader.setVec3("light_color", color);
    }
    return lightWorldPos;

}

void displayLoop(Window &window, const std::string &filename) {
    Shaders shader = Shaders(
        ShaderType::SCENE,
        "../shaders/scene.vert",
        "../shaders/scene.frag"
        );
    Shaders depthShader = Shaders(
        ShaderType::DEPTH,
        "../shaders/point_shadows_depth.vert",
        "../shaders/point_shadows_depth.frag",
        "../shaders/point_shadows_depth.geom"
        );

    // configure depth map FBO
    // -----------------------
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    camera = Camera(EYE, LOOK_AT, UP, SCR_WIDTH, SCR_HEIGHT);

    tinygltf::Model model;
    if (!loadModel(model, filename.c_str())) return;

    auto bindingData = bindModel(model);
    std::vector<GLuint> vaos = bindingData.first;
    std::vector<MaterialTex> matTexs = bindingData.second;

    while (!window.Close()) {
        window.Resize();
        processInput(window.window);

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // build a model-view-projection
        GLint w, h;
        glfwGetWindowSize(window.window, &w, &h);

        TransformationMat transMat{};
        transMat.proj = camera.projection;
        transMat.view = camera.view;

        shader.use();
        glm::vec3 lightWorldPos = setUpLighting(transMat, model, shader);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane  = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightWorldPos, lightWorldPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightWorldPos);
        drawModel(depthShader, vaos, matTexs, transMat, model);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setVec3("view_pos", camera.position);
        shader.setFloat("far_plane", far_plane);

        glUniform1i(glGetUniformLocation(shader.pid, "basecolor_tex"), 0);
        glUniform1i(glGetUniformLocation(shader.pid, "depth_map"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        drawModel(shader, vaos, matTexs, transMat, model);
        glfwSwapBuffers(window.window);
        glfwPollEvents();
    }

    for (int i = 0; i < vaos.size(); ++i) {
        glDeleteVertexArrays(1, &vaos[i]);
    }
}



int main(int argc, char **argv)
{
    std::string filename = "../scene/separate/assets.gltf";

    if (argc > 1) {
        filename = argv[1];
    }

    tinygltf::Model model;
    if (!loadModel(model, filename.c_str())) return -1;

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    Window window = Window(SCR_WIDTH, SCR_HEIGHT, "gltf viewer");
    glfwMakeContextCurrent(window.window);
    glfwSetFramebufferSizeCallback(window.window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window.window, cursor_position_callback); // doing nothing for now

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << glGetString(GL_RENDERER) << ", " << glGetString(GL_VERSION)
              << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    displayLoop(window, filename);

    glfwTerminate();
    return 0;
}


