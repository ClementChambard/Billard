//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <SDL2/SDL_image.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <vector>
#include <stack>

#include <iostream>
#include <string>
#include <sstream>

#include "Shader.h"
#include "logger.h"

#include "Sphere.h"
#include "Cube.h"
#include "Cone.h"

#include "Texture.h"
#include "LensFlare.h"
#include "Camera.h"
#include "Material.h"

#define WIDTH     800
#define HEIGHT    600
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))

#define NB_TEXTURE_BOULE 15

struct GameObjectGraph {
    GLuint vboID = 0;
    Material mtl = { {1,1,1},0.2,0.2,0.2,1 };
    unsigned int nb_vertices = 0;
    glm::mat4 matrix_local = glm::mat4(1.0f);           // pour repercuter la modif sur cet objet uniquement et pas ses enfants
    glm::mat4 matrix_propagated = glm::mat4(1.0f);      // pour repercuter la modif sur lui et aussi ses enfants
    Shader* shader = nullptr;

    std::vector<GameObjectGraph*> children;
};

void draw(GameObjectGraph& go, std::stack<glm::mat4>& matrices, const Light& light, const glm::vec3& cameraPosition, const glm::mat4& view_projection) {

    matrices.push(matrices.top() * go.matrix_propagated);

    glm::mat4 model = matrices.top() * go.matrix_local;
    glm::mat3 invModel3x3 = glm::inverse(glm::mat3(model));
    glm::mat4 mvp = view_projection * model;

    glUseProgram(go.shader->getProgramID());
    {
        glBindBuffer(GL_ARRAY_BUFFER, go.vboID);


        //VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(go.nb_vertices * (3 + 3) * sizeof(float)));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(go.nb_vertices * 3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);


        //Uniform for vertex shader
        GLint uMVP         = glGetUniformLocation(go.shader->getProgramID(), "uMVP");
        GLint uModel       = glGetUniformLocation(go.shader->getProgramID(), "uModel");
        GLint uInvModel3x3 = glGetUniformLocation(go.shader->getProgramID(), "uInvModel3x3");
        glUniformMatrix4fv(uMVP,         1, GL_FALSE, glm::value_ptr(mvp        ));
        glUniformMatrix4fv(uModel,       1, GL_FALSE, glm::value_ptr(model      ));
        glUniformMatrix3fv(uInvModel3x3, 1, GL_FALSE, glm::value_ptr(invModel3x3));

        GLint uMtlColor       = glGetUniformLocation(go.shader->getProgramID(), "uMtlColor"      );
        GLint uMtlCts         = glGetUniformLocation(go.shader->getProgramID(), "uMtlCts"        );
        GLint uLightPos       = glGetUniformLocation(go.shader->getProgramID(), "uLightPos"      );
        GLint uLightColor     = glGetUniformLocation(go.shader->getProgramID(), "uLightColor"    );
        GLint uCameraPosition = glGetUniformLocation(go.shader->getProgramID(), "uCameraPosition");
        glUniform3fv(uMtlColor,       1, glm::value_ptr(go.mtl.getColor  ()));
        glUniform4fv(uMtlCts,         1, glm::value_ptr(go.mtl.getCoefs  ()));
        glUniform3fv(uLightPos,       1, glm::value_ptr(light.getPosition()));
        glUniform3fv(uLightColor,     1, glm::value_ptr(light.getColor   ()));
        glUniform3fv(uCameraPosition, 1, glm::value_ptr(cameraPosition     ));

        if (go.mtl.getTexture() != nullptr) go.mtl.getTexture()->bind();
        glDrawArrays(GL_TRIANGLES, 0, go.nb_vertices);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);

    for (size_t i = 0; i < go.children.size(); i++) {
        draw(*(go.children[i]), matrices, light, cameraPosition, view_projection);
    }

    matrices.pop();
};

int main(int argc, char* argv[])
{
    ////////////////////////////////////////
    //SDL2 / OpenGL Context initialization : 
    ////////////////////////////////////////

    //Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }
    //Init the IMG component
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        ERROR("Could not load SDL2_image with PNG files\n");
        return EXIT_FAILURE;
    }

    //Create a Window
    SDL_Window* window = SDL_CreateWindow("VR Camera",                           //Titre
        SDL_WINDOWPOS_UNDEFINED,               //X Position
        SDL_WINDOWPOS_UNDEFINED,               //Y Position
        WIDTH, HEIGHT,                         //Resolution
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

    //Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    //Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();


    //Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

    //The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
    glClearColor(0.0, 0.0, 0.0, 1.0); //Full Black

    glEnable(GL_DEPTH_TEST); //Active the depth test


    //Formes
    Sphere sphere(32, 32);
    Cube cube;
    Cone cone(32, 0.1f);

    //material
    Material verreMtl{ {1.0f, 0.9f, 0.7f}, 1.0f, 0.0f, 0.0f, 50 };
    Material metalMtl{ {0.3f, 0.3f, 0.4f}, 0.2f, 0.2f, 0.8f, 50 };
    Material murMtl{ {1.0f, 1.0f, 1.0f}, 0.2f, 0.5f, 0.2f, 50 };
    Material solMtl{ {0.5f, 0.38f, 0.21f}, 0.2f, 0.5f, 0.2f, 50 };
    Material boisMtl{ {0.5f, 0.38f, 0.21f}, 0.2f, 0.5f, 0.2f, 50 };
    Material tableMtl{ {0.4f, 1.0f, 0.5f}, 0.2f, 0.5f, 0.0f, 50 };
    Material bouleMtl{ {1.0f, 1.0f, 1.0f}, 0.2f, 0.5f, 1.0f, 40 };
    Light light({0.0f, 8.2f, -10.0f}, {1.0f, 0.9f, 0.7f});


    GLuint vboCube; //id
    glGenBuffers(1, &vboCube);//buffer

    glBindBuffer(GL_ARRAY_BUFFER, vboCube);

    glBufferData(GL_ARRAY_BUFFER, cube.getNbVertices() * (3 + 3) * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cube.getNbVertices() * 3 * sizeof(float), cube.getVertices());
    glBufferSubData(GL_ARRAY_BUFFER, cube.getNbVertices() * 3 * sizeof(float), cube.getNbVertices() * 3 * sizeof(float), cube.getNormals());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vboSphere;
    glGenBuffers(1, &vboSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, sphere.getNbVertices() * (3 + 3 + 2) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sphere.getNbVertices() * 3 * sizeof(float), sphere.getVertices());
    glBufferSubData(GL_ARRAY_BUFFER, sphere.getNbVertices() * 3 * sizeof(float), sphere.getNbVertices() * 3 * sizeof(float), sphere.getNormals());
    glBufferSubData(GL_ARRAY_BUFFER, sphere.getNbVertices() * (3 + 3) * sizeof(float), sphere.getNbVertices() * 2 * sizeof(float), sphere.getUVs());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vboCone; //id
    glGenBuffers(1, &vboCone);//buffer

    glBindBuffer(GL_ARRAY_BUFFER, vboCone);

    glBufferData(GL_ARRAY_BUFFER, cone.getNbVertices() * (3 + 3) * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cone.getNbVertices() * 3 * sizeof(float), cone.getVertices());
    glBufferSubData(GL_ARRAY_BUFFER, cone.getNbVertices() * 3 * sizeof(float), cone.getNbVertices() * 3 * sizeof(float), cone.getNormals());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const char* vertPath = "Shaders/shading.vert";
    const char* fragPath = "Shaders/shading.frag";

    FILE* vertFile = fopen(vertPath, "r");
    FILE* fragFile = fopen(fragPath, "r");

    Shader* shader = Shader::loadFromFiles(vertFile, fragFile);

    fclose(vertFile);
    fclose(fragFile);

    if (shader == nullptr) {
        std::cerr << "The shader 'color' did not compile correctly. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    glUseProgram(shader->getProgramID()); {
        GLint uTexture = glGetUniformLocation(shader->getProgramID(), "uTexture");
        glUniform1i(uTexture, 0);
    } glUseProgram(0);

    float coteSolPlafondMur = 20.0f;
    float epaisseurSolplafondMur = 0.5f;

    float hauteurMur = 12.0f;

    float hauteurTige = 2.2f;
    float coteTige = 0.3f;

    //Table (+ bords + pieds)
    float longueurTable = 8.0f;
    float hauteurTable = 0.5f;
    float largeurTable = 4.0f;

    float longueurPieds = 0.3f;
    float hauteurPieds = 1.5f;
    float largeurPieds = 0.3f;

    float longueurBordC = 0.4f;
    float largeurBordC = largeurTable + 2 * longueurBordC;
    float hauteurBord = hauteurTable + 0.1f;
    float longueurBordL = longueurTable + 2 * longueurBordC;
    float largeurBordL = longueurBordC;


    GameObjectGraph Mur1;
    GameObjectGraph Mur2;
    GameObjectGraph Mur3;
    GameObjectGraph Sol;
    GameObjectGraph Plafond;
    GameObjectGraph TigeLampe;
    GameObjectGraph BaseLampe;
    GameObjectGraph Ampoule;

    GameObjectGraph Table;
    GameObjectGraph Pied1;
    GameObjectGraph Pied2;
    GameObjectGraph Pied3;
    GameObjectGraph Pied4;
    GameObjectGraph BordC1;
    GameObjectGraph BordC2;
    GameObjectGraph BordL1;
    GameObjectGraph BordL2;

    Sol.children.push_back(&Table);
    Sol.children.push_back(&Mur1);
    Sol.children.push_back(&Mur2);
    Sol.children.push_back(&Mur3);
    Sol.children.push_back(&Plafond);

    Plafond.children.push_back(&TigeLampe);

    TigeLampe.children.push_back(&BaseLampe);
    BaseLampe.children.push_back(&Ampoule);

    Table.children.push_back(&Pied1);
    Table.children.push_back(&Pied2);
    Table.children.push_back(&Pied3);
    Table.children.push_back(&Pied4);
    Table.children.push_back(&BordC1);
    Table.children.push_back(&BordC2);
    Table.children.push_back(&BordL1);
    Table.children.push_back(&BordL2);

    Sol.nb_vertices = cube.getNbVertices();
    Mur1.nb_vertices = cube.getNbVertices();
    Mur2.nb_vertices = cube.getNbVertices();
    Mur3.nb_vertices = cube.getNbVertices();
    Plafond.nb_vertices = cube.getNbVertices();

    TigeLampe.nb_vertices = cube.getNbVertices();
    BaseLampe.nb_vertices = cone.getNbVertices();
    Ampoule.nb_vertices = sphere.getNbVertices();


    Table.nb_vertices = cube.getNbVertices();

    Pied1.nb_vertices = cube.getNbVertices();
    Pied2.nb_vertices = cube.getNbVertices();
    Pied3.nb_vertices = cube.getNbVertices();
    Pied4.nb_vertices = cube.getNbVertices();

    BordC1.nb_vertices = cube.getNbVertices();
    BordC2.nb_vertices = cube.getNbVertices();
    BordL1.nb_vertices = cube.getNbVertices();
    BordL2.nb_vertices = cube.getNbVertices();

    Sol.vboID = vboCube;
    Mur1.vboID = vboCube;
    Mur2.vboID = vboCube;
    Mur3.vboID = vboCube;
    Plafond.vboID = vboCube;

    TigeLampe.vboID = vboCube;
    BaseLampe.vboID = vboCone;
    Ampoule.vboID = vboSphere;


    Table.vboID = vboCube;

    Pied1.vboID = vboCube;
    Pied2.vboID = vboCube;
    Pied3.vboID = vboCube;
    Pied4.vboID = vboCube;

    BordC1.vboID = vboCube;
    BordC2.vboID = vboCube;
    BordL1.vboID = vboCube;
    BordL2.vboID = vboCube;

    Sol.mtl = solMtl;
    Mur1.mtl = murMtl;
    Mur2.mtl = murMtl;
    Mur3.mtl = murMtl;
    Plafond.mtl = murMtl;

    TigeLampe.mtl = metalMtl;
    BaseLampe.mtl = metalMtl;
    Ampoule.mtl = verreMtl;

    Table.mtl = tableMtl;

    Pied1.mtl = boisMtl;
    Pied2.mtl = boisMtl;
    Pied3.mtl = boisMtl;
    Pied4.mtl = boisMtl;

    BordC1.mtl = boisMtl;
    BordC2.mtl = boisMtl;
    BordL1.mtl = boisMtl;
    BordL2.mtl = boisMtl;

    Sol.shader = shader;
    Mur1.shader = shader;
    Mur2.shader = shader;
    Mur3.shader = shader;
    Plafond.shader = shader;

    TigeLampe.shader = shader;
    BaseLampe.shader = shader;
    Ampoule.shader = shader;

    Table.shader = shader;

    Pied1.shader = shader;
    Pied2.shader = shader;
    Pied3.shader = shader;
    Pied4.shader = shader;

    BordC1.shader = shader;
    BordC2.shader = shader;
    BordL1.shader = shader;
    BordL2.shader = shader;

    Sol.matrix_local = glm::mat4(1.0f);
    Sol.matrix_local = glm::scale(Sol.matrix_local, glm::vec3(coteSolPlafondMur, epaisseurSolplafondMur, coteSolPlafondMur));
    Sol.matrix_propagated = glm::mat4(1.0f);
    Sol.matrix_propagated = glm::translate(Sol.matrix_propagated, glm::vec3(0.0f, 0.0f, -10.0f));

    Plafond.matrix_local = glm::mat4(1.0f);
    Plafond.matrix_local = glm::scale(Plafond.matrix_local, glm::vec3(coteSolPlafondMur, epaisseurSolplafondMur, coteSolPlafondMur));
    Plafond.matrix_propagated = glm::mat4(1.0f);
    Plafond.matrix_propagated = glm::translate(Plafond.matrix_propagated, glm::vec3(0.0f , hauteurMur, 0.0f));

    Mur1.matrix_local = glm::mat4(1.0f);
    Mur1.matrix_local = glm::scale(Mur1.matrix_local, glm::vec3(epaisseurSolplafondMur, hauteurMur, coteSolPlafondMur));
    Mur1.matrix_propagated = glm::mat4(1.0f);
    Mur1.matrix_propagated = glm::translate(Mur1.matrix_propagated, glm::vec3(-0.5f * coteSolPlafondMur + 0.5f * epaisseurSolplafondMur, - 0.5f * epaisseurSolplafondMur + 0.5f*hauteurMur, 0.0f));

    Mur2.matrix_local = glm::mat4(1.0f);
    Mur2.matrix_local = glm::scale(Mur2.matrix_local, glm::vec3(coteSolPlafondMur, hauteurMur, epaisseurSolplafondMur));
    Mur2.matrix_propagated = glm::mat4(1.0f);
    Mur2.matrix_propagated = glm::translate(Mur2.matrix_propagated, glm::vec3(0.0f, -0.5f * epaisseurSolplafondMur + 0.5f * hauteurMur, -0.5f * coteSolPlafondMur + 0.5f * epaisseurSolplafondMur));

    Mur3.matrix_local = glm::mat4(1.0f);
    Mur3.matrix_local = glm::scale(Mur3.matrix_local, glm::vec3(epaisseurSolplafondMur, hauteurMur, coteSolPlafondMur));
    Mur3.matrix_propagated = glm::mat4(1.0f);
    Mur3.matrix_propagated = glm::translate(Mur3.matrix_propagated, glm::vec3(0.5f * coteSolPlafondMur - 0.5f * epaisseurSolplafondMur, - 0.5f * epaisseurSolplafondMur + 0.5f*hauteurMur, 0.0f));

    TigeLampe.matrix_local = glm::mat4(1.0f);
    TigeLampe.matrix_local = glm::scale(TigeLampe.matrix_local, glm::vec3(coteTige, hauteurTige, coteTige));
    TigeLampe.matrix_propagated = glm::mat4(1.0f);
    TigeLampe.matrix_propagated = glm::translate(TigeLampe.matrix_propagated, glm::vec3(0.0f, -0.5f * epaisseurSolplafondMur - 0.5f * hauteurTige, 0.0f));

    BaseLampe.matrix_local = glm::mat4(1.0f);
    BaseLampe.matrix_local = glm::rotate(BaseLampe.matrix_local, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    BaseLampe.matrix_local = glm::scale(BaseLampe.matrix_local, glm::vec3(1.3f, 1.3f, 1.3f));
    BaseLampe.matrix_propagated = glm::mat4(1.0f);
    BaseLampe.matrix_propagated = glm::translate(BaseLampe.matrix_propagated, glm::vec3(0.0f,- 0.5f * hauteurTige - 0.5f, 0.0f));

    Ampoule.matrix_local = glm::mat4(1.0f);
    //Ampoule.matrix_local = glm::scale(Ampoule.matrix_local, glm::vec3(0.8f, 0.8f, 0.8f));
    Ampoule.matrix_propagated = glm::mat4(1.0f);
    Ampoule.matrix_propagated = glm::translate(Ampoule.matrix_propagated, glm::vec3(0.0f, -0.5f, 0.0f));

    Pied1.matrix_local = glm::mat4(1.0f);
    Pied1.matrix_propagated = glm::mat4(1.0f);
    Pied1.matrix_propagated = glm::translate(Pied1.matrix_propagated, glm::vec3(-0.5f * longueurTable + 0.5f * longueurPieds, -0.5f * hauteurPieds - 0.5f * hauteurTable, -0.5f * largeurTable + 0.5f * largeurPieds));
    Pied1.matrix_propagated = glm::scale(Pied1.matrix_propagated, glm::vec3(longueurPieds, hauteurPieds, largeurPieds));

    Pied2.matrix_local = glm::mat4(1.0f);
    Pied2.matrix_propagated = glm::mat4(1.0f);
    Pied2.matrix_propagated = glm::translate(Pied2.matrix_propagated, glm::vec3(-0.5f * longueurTable + 0.5f * longueurPieds, -0.5f * hauteurPieds - 0.5f * hauteurTable, 0.5f * largeurTable - 0.5f * largeurPieds));
    Pied2.matrix_propagated = glm::scale(Pied2.matrix_propagated, glm::vec3(longueurPieds, hauteurPieds, largeurPieds));//local

    Pied3.matrix_local = glm::mat4(1.0f);
    Pied3.matrix_propagated = glm::mat4(1.0f);
    Pied3.matrix_propagated = glm::translate(Pied3.matrix_propagated, glm::vec3(0.5f * longueurTable - 0.5f * longueurPieds, -0.5f * hauteurPieds - 0.5f * hauteurTable, -0.5f * largeurTable + 0.5f * largeurPieds));
    Pied3.matrix_propagated = glm::scale(Pied3.matrix_propagated, glm::vec3(longueurPieds, hauteurPieds, largeurPieds));

    Pied4.matrix_local = glm::mat4(1.0f);
    Pied4.matrix_propagated = glm::mat4(1.0f);
    Pied4.matrix_propagated = glm::translate(Pied4.matrix_propagated, glm::vec3(0.5f * longueurTable - 0.5f * longueurPieds, -0.5f * hauteurPieds - 0.5f * hauteurTable, 0.5f * largeurTable - 0.5f * largeurPieds));
    Pied4.matrix_propagated = glm::scale(Pied4.matrix_propagated, glm::vec3(longueurPieds, hauteurPieds, largeurPieds));

    BordC1.matrix_local = glm::mat4(1.0f);
    BordC1.matrix_propagated = glm::mat4(1.0f);
    BordC1.matrix_propagated = glm::translate(BordC1.matrix_propagated, glm::vec3(-0.5f * longueurTable - 0.5f * longueurBordC, 0.0f, 0.0f));
    BordC1.matrix_propagated = glm::scale(BordC1.matrix_propagated, glm::vec3(longueurBordC, hauteurBord, largeurBordC));

    BordC2.matrix_local = glm::mat4(1.0f);
    BordC2.matrix_propagated = glm::mat4(1.0f);
    BordC2.matrix_propagated = glm::translate(BordC2.matrix_propagated, glm::vec3(0.5f * longueurTable + 0.5f * longueurBordC, 0.0f, 0.0f));
    BordC2.matrix_propagated = glm::scale(BordC2.matrix_propagated, glm::vec3(longueurBordC, hauteurBord, largeurBordC));

    BordL1.matrix_local = glm::mat4(1.0f);
    BordL1.matrix_propagated = glm::mat4(1.0f);
    BordL1.matrix_propagated = glm::translate(BordL1.matrix_propagated, glm::vec3(0.0f, 0.0f, -0.5f * largeurTable - 0.5f * largeurBordL));
    BordL1.matrix_propagated = glm::scale(BordL1.matrix_propagated, glm::vec3(longueurBordL, hauteurBord, largeurBordL));

    BordL2.matrix_local = glm::mat4(1.0f);
    BordL2.matrix_propagated = glm::mat4(1.0f);
    BordL2.matrix_propagated = glm::translate(BordL2.matrix_propagated, glm::vec3(0.0f, 0.0f, 0.5f * largeurTable + 0.5f * largeurBordL));
    BordL2.matrix_propagated = glm::scale(BordL2.matrix_propagated, glm::vec3(longueurBordL, hauteurBord, largeurBordL));

    //Boules
    float scaleBoules = 0.25f;
    float rayonBoules = scaleBoules * 0.5f;
    float distBoules = 2 * rayonBoules;


    GameObjectGraph Boules[16];

    Table.children.push_back(&Boules[8]);
    Table.children.push_back(&Boules[0]);

    int ordre_boules[NB_TEXTURE_BOULE] = { 9, 12, 7, 1, 8, 15, 14, 3, 10, 6, 5, 4, 13, 2, 11 };

    //La boule blanche n'a pas de texture et n'est pas dans le triangle
    Boules[0].nb_vertices = sphere.getNbVertices();
    Boules[0].mtl = bouleMtl;
    Boules[0].vboID = vboSphere;
    Boules[0].shader = shader;

    Boules[0].matrix_local = glm::mat4(1.0f);
    Boules[0].matrix_propagated = glm::mat4(1.0f);
    Boules[0].matrix_propagated = glm::translate(Boules[0].matrix_propagated, glm::vec3(-0.5f * longueurTable + 1.0f, 0.5f * hauteurTable + rayonBoules, 0.0f));
    Boules[0].matrix_propagated = glm::scale(Boules[0].matrix_propagated, glm::vec3(scaleBoules, scaleBoules, scaleBoules));

    //Variables pour le positionnement des boules du triangle
    float h = sqrt(3) / 2 * distBoules; //par le théorème de Pythagore : dist² = h² + (dist/2)² avec dist/2 = demi_dist
    float x = -2 * h; //Position x de départ pour que la boule du milieu (boule n°8) est un x de 0
    float y_min = 0;

    int boule_n = 0;
    int num_boule_n;

    srand(time(0));

    for (int i = 1; boule_n < NB_TEXTURE_BOULE; i++)
    {
        for (int j = 0; j < i; j++)
        {
            num_boule_n = ordre_boules[boule_n];

            float y = y_min + distBoules * j;

            //Propriétés de la boule
            Boules[num_boule_n].nb_vertices = sphere.getNbVertices();
            Boules[num_boule_n].mtl = bouleMtl;
            Boules[num_boule_n].vboID = vboSphere;

            //Placement de la boule
            Boules[num_boule_n].matrix_local = glm::mat4(1.0f);
            Boules[num_boule_n].matrix_propagated = glm::mat4(1.0f);

            int rand_rotation = rand();
            int bool_axeX = rand() % 2;
            int bool_axeY = rand() % 2;
            int bool_axeZ = rand() % 2;
            if (bool_axeX == 0 && bool_axeY == 0) {
                bool_axeZ = 1; //il ne faut pas que les 3 axes soient sur 0
            }

            Boules[num_boule_n].matrix_local = glm::rotate(Boules[num_boule_n].matrix_local, (float)rand_rotation, glm::vec3(bool_axeX % 2, bool_axeY % 2, bool_axeZ % 2)); //Rotation aléatoire des boules

            if (boule_n != 4) {//Pas la boule du milieu
                Boules[ordre_boules[4]].children.push_back(&Boules[num_boule_n]);
                Boules[num_boule_n].matrix_propagated = glm::translate(Boules[num_boule_n].matrix_propagated, glm::vec3(x, 0.0f, y));
            }
            else { //Boule du milieu
                Boules[num_boule_n].matrix_propagated = glm::translate(Boules[num_boule_n].matrix_propagated, glm::vec3(0.5f * longueurTable - 2.0f, 0.5f * hauteurTable + rayonBoules, 0.0f));
            }

            Boules[num_boule_n].matrix_local = glm::scale(Boules[num_boule_n].matrix_local, glm::vec3(scaleBoules, scaleBoules, scaleBoules));


            Boules[num_boule_n].mtl.setColor({0,0,0});
            Boules[num_boule_n].mtl.setTexture(new Texture("Assets/Boule_" + std::to_string(num_boule_n) + ".png")); //Donner la texture à la Boule
            Boules[num_boule_n].shader = shader;

            boule_n++;
        }
        y_min = y_min - distBoules / 2;
        x = x + h;
    }



    // lens flare initialization
    Texture* texLight1 = new Texture("Assets/shape_0.png");
    Texture* texLight2 = new Texture("Assets/shape_1.png");
    Texture* texLight3 = new Texture("Assets/shape_2.png");
    Texture* texLight4 = new Texture("Assets/shape_3.png");
    Texture* texLight5 = new Texture("Assets/shape_4.png");
    Texture* texLight6 = new Texture("Assets/shape_5.png");
    Texture* texLight7 = new Texture("Assets/shape_6.png");
    Texture* texLight8 = new Texture("Assets/shape_7.png");
    Texture* texLight9 = new Texture("Assets/shape_8.png");
    LensFlare::Init();
    LensFlare::setLight(&light);
    LensFlare::addTexture(texLight6, 1.f);
    LensFlare::addTexture(texLight4, 0.46f);
    LensFlare::addTexture(texLight2, 0.2f);
    LensFlare::addTexture(texLight7, 0.1f);
    LensFlare::addTexture(texLight1, 0.04f);
    LensFlare::addTexture(texLight3, 0.12f);
    LensFlare::addTexture(texLight9, 0.24f);
    LensFlare::addTexture(texLight5, 0.14f);
    LensFlare::addTexture(texLight1, 0.024f);
    LensFlare::addTexture(texLight7, 0.4f);
    LensFlare::addTexture(texLight9, 0.2f);
    LensFlare::addTexture(texLight3, 0.14f);
    LensFlare::addTexture(texLight5, 0.6f);
    LensFlare::addTexture(texLight4, 0.8f);
    LensFlare::addTexture(texLight8, 1.2f);


    Camera cam;
    bool mouseLock = false;
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keyShift = false;
    bool keySpace = false;
    float mouseX = 0;
    float mouseY = 0;
    //Main application loop
    float t = 0.5f;
    bool isOpened = true;
    while (isOpened)
    {
        //Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

        //Fetch the SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    isOpened = false;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_z: keyW = true; break;
                    case SDLK_q: keyA = true; break;
                    case SDLK_s: keyS = true; break;
                    case SDLK_d: keyD = true; break;
                    case SDLK_SPACE: keySpace = true; break;
                    case SDLK_LSHIFT: keyShift = true; break;
                    case SDLK_LCTRL: SDL_ShowCursor(mouseLock); mouseLock = !mouseLock; break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_z: keyW = false; break;
                    case SDLK_q: keyA = false; break;
                    case SDLK_s: keyS = false; break;
                    case SDLK_d: keyD = false; break;
                    case SDLK_SPACE: keySpace = false; break;
                    case SDLK_LSHIFT: keyShift = false; break;
                    default: break;
                }
                break;
            case SDL_MOUSEMOTION:
                mouseX = event.motion.x - WIDTH/2.f;
                mouseY = -event.motion.y + HEIGHT/2.f;
                break;
                //We can add more event, like listening for the keyboard or the mouse. See SDL_Event documentation for more details
            }
        }

        //Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


        //Update the camera
        float pitch = 0.f ,yaw = 0.f;
        if (mouseLock)
        {
            yaw = -mouseX / 100;
            pitch = mouseY / 100;
            SDL_WarpMouseInWindow(window, WIDTH/2.f, HEIGHT/2.f);
        }
        cam.move(pitch, yaw, (keyW - keyS) * .08f, (keyD - keyA) * .08f, (keySpace - keyShift) * .08f);
        light.setView(cam.getMat());


        glm::mat4 projection = glm::perspective(45.0f, WIDTH / (float)HEIGHT, 0.01f, 1000.0f);

        Table.matrix_local = glm::mat4(1.0f);
        Table.matrix_local = glm::scale(Table.matrix_local, glm::vec3(longueurTable, hauteurTable, largeurTable));
        Table.matrix_propagated = glm::mat4(1.0f);
        Table.matrix_propagated = glm::translate(Table.matrix_propagated, glm::vec3(0.0f, 0.5f*epaisseurSolplafondMur+hauteurPieds, 0.0f));
        Table.matrix_propagated = glm::rotate(Table.matrix_propagated, t, glm::vec3(0.0f, 1.0f, 0.0f));

        t += 0.01f;


        std::stack<glm::mat4> matrices;
        matrices.push(glm::mat4(1.0f));

        //APPEL A DRAW
        draw(Sol, matrices, light, cam.getPos(), projection * cam.getMat());
        LensFlare::render(projection);


        //Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        //Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        //We want FRAMERATE FPS
        if (timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay((uint32_t)(TIME_PER_FRAME_MS)-(timeEnd - timeBegin));
    }

    glDeleteBuffers(1, &vboCube);
    glDeleteBuffers(1, &vboCone);
    glDeleteBuffers(1, &vboSphere);
    for (int i = 0; i < 16; i++)
    {
        if (Boules[i].mtl.getTexture() != nullptr) delete Boules[i].mtl.getTexture();
    }
    LensFlare::Cleanup();
    delete shader;
    delete texLight1;
    delete texLight2;
    delete texLight3;
    delete texLight4;
    delete texLight5;
    delete texLight6;
    delete texLight7;
    delete texLight8;
    delete texLight9;

    //Free everything
    if (context != NULL)
        SDL_GL_DeleteContext(context);
    if (window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}
