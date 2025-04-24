// Include C++ headers
#include <iostream>
#include <string>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>
#include <common/light.h> 

#include "Sphere.h"
#include "common/FountainEmitter.h"
#include <chrono>


using namespace std::chrono;
using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


#define W_WIDTH 1024
#define W_HEIGHT 768
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024
#define TITLE "Space Golf"
#define gg 9.80665f
#define RAND ((float) rand()) / (float) RAND_MAX
#define n 16

// Global variables
GLFWwindow* window;
Camera* camera;
Light* light;
Drawable* plane;
Drawable* target;
Drawable* quad;

GLuint viewMatrixLocation;
GLuint projectionMatrixLocation;
GLuint modelMatrixLocation;
GLuint shaderProgram, depthProgram;
GLuint useTextureLocation;
GLuint particlesDepthLocation;
GLuint particlesLightLocation;
GLuint useDist_sqLocation;
GLuint lightPositionLocation;
GLuint lightPowerLocation;
GLuint lightVPLocation;
GLuint modelDiffuseTexture, modelSpecularTexture;
GLuint KaLocation, KdLocation, KsLocation, NsLocation;
GLuint LaLocation, LdLocation, LsLocation;
GLuint diffuseColorSampler, specularColorSampler;
GLuint depthMapSampler;
GLuint depthFrameBuffer, depthTexture;

GLuint transformations_buffer, rotations_buffer, scales_buffer;

GLuint shadowViewProjectionLocation;
GLuint shadowModelLocation;

GLuint diffuceColorSampler, projectionAndViewMatrix;

int particles_slider = 7000;
bool game_paused = false;


//mat4 viewMatrix;
std::vector<const char*> DiffuseTextures = { "Rough_Rock_023_COLOR.bmp", "Rock_047_BaseColor.bmp", "Rock_040_basecolor.bmp","Canyon_Rock_002_basecolor.bmp", "Substance_Graph_BaseColor.bmp" };
std::vector<const char*> SpecularTextures = { "Rough_Rock_023_ROUGH.bmp", "Rock_047_Roughness.bmp", "Rock_040_roughness.bmp","Canyon_Rock_002_roughness.bmp", "Substance_Graph_Roughness.bmp" };
std::vector<int> text = { GL_TEXTURE1,GL_TEXTURE2,GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5 ,GL_TEXTURE6,GL_TEXTURE7, GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10};

vec3 spherePosition[n], moonPosition[n];
vec3 traj[n];
float dist[n];
float Rs[n], moonRs[n], R[4];
mat4 translations[n], moontranslations[n], tr[4];
mat4 rotations[n];

int thrownball = 0;
vec3 x = glm::vec3(0, 0, 0);
vec3 y = glm::vec3(0, 2, 0);
vec3 z = glm::vec3(0, 0, 30);
float zt = RAND * 4 - 28;
float yt = -RAND * 4;
vec3 center;
mat4 planeModelMatrix, rot;
bool r = 1;


// Drawables
Sphere* mdl;
Sphere* model;




struct Material {
    vec4 Ka;
    vec4 Kd;
    vec4 Ks;
    float Ns;
};

const Material turquoise{
    vec4{ 0.1, 0.18725, 0.1745, 0.8 },
    vec4{ 0.396, 0.74151, 0.69102, 1.0 },
    vec4{ 0.297254, 0.30829, 0.306678, 0 },
    150.0f
};

void uploadMaterial(const Material& mtl) {
    glUniform4f(KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
    glUniform4f(KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
    glUniform4f(KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
    glUniform1f(NsLocation, mtl.Ns);
}

void uploadLight(const Light& light) {
    glUniform4f(LaLocation, light.La.r, light.La.g, light.La.b, light.La.a);
    glUniform4f(LdLocation, light.Ld.r, light.Ld.g, light.Ld.b, light.Ld.a);
    glUniform4f(LsLocation, light.Ls.r, light.Ls.g, light.Ls.b, light.Ls.a);
    glUniform3f(lightPositionLocation, light.lightPosition_worldspace.x,
        light.lightPosition_worldspace.y, light.lightPosition_worldspace.z);
    glUniform1f(lightPowerLocation, light.power);
}

void createContext() {
    shaderProgram = loadShaders("Shader.vertexshader", "Shader.fragmentshader");
    depthProgram = loadShaders("Depth.vertexshader", "Depth.fragmentshader");

    KaLocation = glGetUniformLocation(shaderProgram, "mtl.Ka");
    KdLocation = glGetUniformLocation(shaderProgram, "mtl.Kd");
    KsLocation = glGetUniformLocation(shaderProgram, "mtl.Ks");
    NsLocation = glGetUniformLocation(shaderProgram, "mtl.Ns");

    LaLocation = glGetUniformLocation(shaderProgram, "light.La");
    LdLocation = glGetUniformLocation(shaderProgram, "light.Ld");
    LsLocation = glGetUniformLocation(shaderProgram, "light.Ls");
    lightPositionLocation = glGetUniformLocation(shaderProgram, "light.lightPosition_worldspace");
    lightPowerLocation = glGetUniformLocation(shaderProgram, "light.power");
    lightVPLocation = glGetUniformLocation(shaderProgram, "lightVP");

    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
    specularColorSampler = glGetUniformLocation(shaderProgram, "specularColorSampler");
    useTextureLocation = glGetUniformLocation(shaderProgram, "useTexture");
    useDist_sqLocation = glGetUniformLocation(shaderProgram, "useDistance_sq");
    particlesLightLocation = glGetUniformLocation(shaderProgram, "particles");

    shadowViewProjectionLocation = glGetUniformLocation(depthProgram, "VP");
    shadowModelLocation = glGetUniformLocation(depthProgram, "M");
    particlesDepthLocation = glGetUniformLocation(depthProgram, "particles");

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");

    srand(time(NULL));

    //Arxikopoihseis thesewn, aktinwn kai troxiwn.
    //Xrhsimopoiw ena ball.obj arxeio pou den exei aktina 1, opote oi floats pou dhmiourgw den einai oi pragmatikes aktines twn planhtwn/feggariwn. 
    //Me tous floats sta Rs kai ta moonRs, omws, kanw scaling, opote tous xrhsimopoiw kai tous antimetwpizw san aktines stis perissoteres periptwseis.
    spherePosition[0] = vec3(RAND * 26 - 13, RAND * 10 - 3.5, RAND * (-30) );
    moonPosition[0] = spherePosition[0] + dist[0] * traj[0];
    Rs[0] = RAND * 0.003 + 0.002;
    moonRs[0] = RAND * 0.0005 + 0.0009;
    dist[0] = RAND * 1 + 100 * Rs[0] + 1;
    traj[0] = normalize(vec3(RAND, RAND, RAND));
    model= new Sphere(vec3(0, 0, 0), vec3(0, 0, 0), Rs[0], 1);
    center = spherePosition[0];
    moontranslations[0] = translate(mat4(), moonPosition[0]);
    translations[0] = translate(mat4(), spherePosition[0]);
    rotations[0] = rotate(mat4(), -3.14f, vec3(1, 0, 0));

    for (int i = 1; i < n; i++) {
        spherePosition[i] = vec3(RAND * 26 - 13, RAND * 10 - 3.5, RAND * (-30) );
        for (int j = 0; j < i; j++) {
            while (distance(spherePosition[j], spherePosition[i]) < 5.0f) {
                j = 0;
                spherePosition[i] = vec3(RAND * 26 - 13, RAND * 10 - 3.5, RAND * (-30) );
            }
        }
        while (distance(vec3(0, yt, zt), spherePosition[i]) < 5.0f) {
            zt = RAND * 4 - 28;
            yt = -RAND * 4;
        }
        Rs[i] = RAND * 0.003 + 0.002;
        dist[i] = RAND * 1 + 100 * Rs[i] + 1;
        traj[i] = normalize(vec3(RAND, RAND, RAND));
        center += spherePosition[i];
        moonPosition[i] = spherePosition[i] + dist[i] * traj[i];
        translations[i] = translate(mat4(), spherePosition[i]);
        moontranslations[i] = translate(mat4(), moonPosition[i]);
        rotations[i] = rotate(mat4(), -3.14f, vec3(1, 0, 0));
        moonRs[i] = RAND * 0.0005 + 0.0009;
        R[i / 4] = Rs[i / 4];
        tr[i / 4] = translations[i / 4];
    }
    R[0] = Rs[0];
    tr[0] = translations[0];

    //Dhmiourgw tous buffers gia ta translations,ta rotations kai ta scalings
    model->sphere->bind();
    
    std::size_t vec4Size = sizeof(glm::vec4);
    glGenBuffers(1, &transformations_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, transformations_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::mat4), &tr[0]);


    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glGenBuffers(1, &rotations_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, rotations_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::mat4), &rotations[0]);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glVertexAttribDivisor(10, 1);

    glGenBuffers(1, &scales_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, scales_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(float), &R[0]);

    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribDivisor(11, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    vector<vec3> planeVertices = {
        vec3(-25.0f, -25.0f, 0),
        vec3(-25.0f, 25.0f, 0),
        vec3(25.0f,  25.0f, 0),
        vec3(25.0f,  25.0f, 0),
        vec3(25.0f,  -25.0f, 0),
        vec3(-25.0f, -25.0f, 0)

    };

    vector<vec3> targetVertices = {
        vec3(-1.0f, -1.0f, 0),
        vec3(-1.0f, 1.0f, 0),
        vec3(1.0f,  1.0f, 0),
        vec3(1.0f,  1.0f, 0),
        vec3(1.0f,  -1.0f, 0),
        vec3(-1.0f, -1.0f, 0)

    };
    // plane normals
    vector<vec3> planeNormals = {
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    // plane uvs
    vector<vec2> planeUVs = {
        vec2(0.0f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 1.0f),
        vec2(1.0f, 1.0f),
        vec2(1.0f, 0.0f),
        vec2(0.0f, 0.0f),
    };

    plane = new Drawable(planeVertices, planeUVs, planeNormals);

    center = center / (n * 1.0f);
    light->targetPosition = center;

    //Arxikopoiw th thesh tou plane
    rot = rotate(mat4(), -acos(dot(normalize(-z), normalize(-(x + y + z) - center))), normalize(cross(-z, -(x + y + z) - center)));
    planeModelMatrix = translate(mat4(), (-x - y - z) + 2.0f * center) * rot;

    target = new Drawable(targetVertices, planeUVs, planeNormals);

    vector<vec3> quadVertices = {
    vec3(0.5, 0.5, 0.0),
    vec3(1.0, 0.5, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.5, 1.0, 0.0),
    vec3(0.5, 0.5, 0.0)
    };

    vector<vec2> quadUVs = {
      vec2(0.0, 0.0),
      vec2(1.0, 0.0),
      vec2(1.0, 1.0),
      vec2(1.0, 1.0),
      vec2(0.0, 1.0),
      vec2(0.0, 0.0)
    };
    quad = new Drawable(quadVertices, quadUVs);

    //Dhmiourgw ton depth buffer
    glGenFramebuffers(1, &depthFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

    //Dhmiourgw to depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);                          // GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER

    //Sundew ston depth buffer to depth texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glfwTerminate();
        throw runtime_error("Frame buffer not initialized correctly");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updatebuffers(mat4* tr, float* R) {

    //Update the data in transformations_buffer and scales_buffer
    glBindBuffer(GL_ARRAY_BUFFER, transformations_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::mat4), &tr[0]);

    glBindBuffer(GL_ARRAY_BUFFER, scales_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(float), &R[0]);
}

void free() {
    delete model;

    delete plane;
    delete target;

    delete mdl;
    glDeleteProgram(shaderProgram);
    glDeleteProgram(depthProgram);

    glfwTerminate();
}

void depth_pass(mat4 viewMatrix, mat4 projectionMatrix, int b, FountainEmitter f_emitter) {
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(depthProgram);

    mat4 view_projection = projectionMatrix * viewMatrix;
    glUniformMatrix4fv(shadowViewProjectionLocation, 1, GL_FALSE, &view_projection[0][0]);

    glUniform1i(particlesDepthLocation, 1);


    for (int i = 0; i < 4; i++) {
        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &model->modelMatrix[0][0]);
        model->sphere->bind();

        //Me kathe zeugari diffuse-specular textures kanw instanced draw 4 planhtes kai 4 feggaria
        updatebuffers(&translations[i * 4], &Rs[i * 4]);
        glDrawElementsInstanced(GL_TRIANGLES, model->sphere->indices.size(), GL_UNSIGNED_INT, NULL, 4);
        updatebuffers(&moontranslations[i * 4], &moonRs[i * 4]);
        glDrawElementsInstanced(GL_TRIANGLES, model->sphere->indices.size(), GL_UNSIGNED_INT, NULL, 4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    if (thrownball == 2) {
        glUniform1i(particlesDepthLocation, 0);
        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &mdl->modelMatrix[0][0]);
        mdl->draw();
        glUniform1i(particlesDepthLocation, 1);
        if (r == 1 ) {
            auto PV = projectionMatrix * viewMatrix;
            glUniformMatrix4fv(projectionAndViewMatrix, 1, GL_FALSE, &PV[0][0]);
            f_emitter.renderParticles();
        }
    }
    glUniform1i(particlesDepthLocation, 0);

    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &planeModelMatrix[0][0]);
    plane->bind();
    plane->draw();


    mat4 targetModelMatrix = translate(mat4(), vec3(0, yt, zt));
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &targetModelMatrix[0][0]);
    target->bind();
    target->draw();


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaderProgram);
}

void lighting_pass(mat4 viewMatrix, mat4 projectionMatrix, float currentTime, float dt, FountainEmitter f_emitter) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, W_WIDTH, W_HEIGHT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    uploadLight(*light);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(depthMapSampler, 0);
    mat4 light_VP = light->lightVP();
    glUniformMatrix4fv(lightVPLocation, 1, GL_FALSE, &light_VP[0][0]);

    glUniform1i(useTextureLocation, 1);
    glUniform1i(particlesLightLocation, 1);

    for (int i = 0; i < 4; i++) {
        glUniform1i(diffuseColorSampler, i + 1);
        glUniform1i(specularColorSampler, i + 6);

        //Me kathe zeugari diffuse-specular textures kanw instanced draw 4 planhtes kai 4 feggaria
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &model->modelMatrix[0][0]);
        model->sphere->bind();
        updatebuffers(&translations[i * 4], &Rs[i * 4]);
        glDrawElementsInstanced(GL_TRIANGLES, model->sphere->indices.size(), GL_UNSIGNED_INT, NULL, 4);
        updatebuffers(&moontranslations[i * 4], &moonRs[i * 4]);
        glDrawElementsInstanced(GL_TRIANGLES, model->sphere->indices.size(), GL_UNSIGNED_INT, NULL, 4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glUniform1i(particlesLightLocation, 0);

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &planeModelMatrix[0][0]);
    uploadMaterial(turquoise);

    glUniform1i(useTextureLocation, 0);
    glUniform1i(useDist_sqLocation, 0);
    plane->bind();
    plane->draw();
    glUniform1i(useTextureLocation, 1);

    
    mat4 targetModelMatrix = translate(mat4(), vec3(0, yt, zt));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &targetModelMatrix[0][0]);
    glUniform1i(diffuseColorSampler, 12);

    target->bind();
    target->draw();
    glUniform1i(useDist_sqLocation, 1);
    if (thrownball == 2) {
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &mdl->modelMatrix[0][0]);

        glUniform1i(diffuseColorSampler, 5);
        glUniform1i(specularColorSampler, 10);

        mdl->draw();
        glUniform1i(particlesLightLocation, 1);

        auto PV = projectionMatrix * viewMatrix;
        glUniformMatrix4fv(projectionAndViewMatrix, 1, GL_FALSE, &PV[0][0]);

        //Χρησιμοποιώ τα ίδια textures για τα paricles
        f_emitter.renderParticles();
        glUniform1i(particlesLightLocation, 0);
    }

    glUseProgram(shaderProgram);

}


void mainLoop() {

    light->update();
    float t = glfwGetTime();
    float currentTime, dt;

    mat4 light_proj = light->projectionMatrix;
    mat4 light_view = light->viewMatrix;
    mat4 projectionMatrix, modelVP, viewMatrix;
    mat4 vm;
    vec3 p, pm;

    auto* quad = new Drawable("sphere.obj");
    FountainEmitter f_emitter = FountainEmitter(quad, particles_slider);
    f_emitter.use_rotations = false;
    f_emitter.use_sorting = false;

    for (int i = 0; i < 5; i++) {
        glActiveTexture(text[i]);
        glBindTexture(GL_TEXTURE_2D, loadBMP(DiffuseTextures[i]));
        glActiveTexture(text[i+5]);
        glBindTexture(GL_TEXTURE_2D, loadBMP(SpecularTextures[i]));
    }
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, loadSOIL("nb.png"));


    do {
        currentTime = glfwGetTime();
        dt = currentTime - t;
        
        light->update();
        light_proj = light->projectionMatrix;
        light_view = light->viewMatrix;
        light->targetPosition = center;
        camera->update();
        projectionMatrix = camera->projectionMatrix;
        viewMatrix = camera->viewMatrix;

        for (int i = 0; i < n; i++) {
            mat3 rott = mat3(rotate(mat4(), 3.14f / 6.0f, traj[i]));
            moonPosition[i] = spherePosition[i] + rott * vec3(dist[i] * sin(pow(2, i % 3) * 0.2 * t), 0, dist[i] * cos(pow(2, i % 3) * 0.2 * t));
            moontranslations[i] = translate(mat4(), moonPosition[i]);
        }

        model->update(t, dt);

        if (thrownball == 1) {
            vm = viewMatrix;
            mdl = new Sphere(camera->position, vec3(cos(camera->verticalAngle) * sin(camera->horizontalAngle),
                sin(camera->verticalAngle),
                cos(camera->verticalAngle) * cos(camera->horizontalAngle)), 0.0013, 5);
            
            thrownball = 2;
        }
        else if (thrownball == 2) {
            mdl->forcing = [&](float t, const vector<float>& y)->vector<float> {
                vector<float> f(6, 0.0f);
                for (int i = 0; i < n; i++) {
                    p = normalize(spherePosition[i] - mdl->x);
                    pm = normalize(moonPosition[i] - mdl->x);
                    for (int j = 0; j < 3; j++) {
                        //Xrhsimopoiw tis aktines x 1000 ws mazes twn planhtwn/feggariwn
                        f[j] += dot(mdl->m * Rs[i]*1000 * gg / (float)pow(distance(spherePosition[i], mdl->x), 2), p[j]) + dot(mdl->m * moonRs[i]*1000 * gg / (float)pow(distance(mdl->x, moonPosition[i]), 2), pm[j]);
                    }
                    //check for collision
                    if (distance(mdl->x, spherePosition[i]) < 80 * (Rs[i] + mdl->r)) {
                        cout << "Collision!" << endl;
                        thrownball = 0;
                    }
                }
                return f;
            };
            mdl->update(t, dt);
            f_emitter.emitter_pos = mdl->x;
            if (!game_paused) {
                f_emitter.updateParticles(currentTime, dt, camera->position);
            }
            if (distance(mdl->x, vec3(0, yt, zt)) <= 100 * mdl->r) {
                cout << "You hit the target!" << endl;
            }
        }

        depth_pass(light_view, light_proj, 1, f_emitter);
        lighting_pass(viewMatrix, projectionMatrix, currentTime, dt, f_emitter);

        t += dt;
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);
}

void plnrot(float fln, vec3 v) {
    //Pollaplasiazw apo aristera ton prohgoumeno rotation matrix me ton rotation matrix pou dinei h kainourgia metatopish tou fwtos.
    rot = rotate(mat4(1.0),
        acos(fln), v) * rot;

    //Thelw h thesh tou plane na einai summetrikh ws pros to kentro twn sfairwn..
    planeModelMatrix = translate(mat4(), (-x - y - z) + 2.0f * center) * rot;
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE) {
        thrownball = 1;
    }
    if (key == GLFW_KEY_N) {
        light->power += 5;
    }
    if (key == GLFW_KEY_M) {
        light->power -= 5;
    }
    if (key == GLFW_KEY_H) {
        x[0] -= 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[0] -= 0.5;
    }
    if (key == GLFW_KEY_K) {
        x[0] += 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[0] += 0.5;
    }
    if (key == GLFW_KEY_Y) {
        y[1] -= 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[1] -= 0.5;
    }
    if (key == GLFW_KEY_I) {
        y[1] += 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[1] += 0.5;
    }
    if (key == GLFW_KEY_J) {
        z[2] -= 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[2] -= 0.5;
    }
    if (key == GLFW_KEY_U) {
        z[2] += 0.5;
        plnrot(dot(normalize(light->lightPosition_worldspace - center), normalize(x + y + z - center)), normalize(cross(light->lightPosition_worldspace - center, (x + y + z - center))));
        light->lightPosition_worldspace[2] += 0.5;
    }
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        if (r == 1) r = 0;
        else r = 1;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        game_paused = !game_paused;
    }

}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
            " If you have an Intel GPU, they are not 3.3 compatible." +
            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    glfwSetKeyCallback(window, pollKeyboard);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
    light = new Light(window,
        vec4{ 1, 1, 1, 1 },
        vec4{ 1, 1, 1, 1 },
        vec4{ 1, 1, 1, 1 },
        x + z + y,
        30.0f
    );
}

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    }
    catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}