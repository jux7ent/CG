// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <random>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/text2D.hpp>
#include <memory>

class MetaObject {
public:
    MetaObject(const char* file) {
        loadOBJ(file, Vertices, Uvs, Normals);

        glGenBuffers(1, &VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(glm::vec3), &Vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &UvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, UvBuffer);
        glBufferData(GL_ARRAY_BUFFER, Uvs.size() * sizeof(glm::vec2), &Uvs[0], GL_STATIC_DRAW);//GLuint enemy_uvbuffer;

        glGenBuffers(1, &NormalsID);
        glBindBuffer(GL_ARRAY_BUFFER, NormalsID);
        glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(glm::vec3), &Normals[0], GL_STATIC_DRAW);
    }

    std::vector<glm::vec3> Vertices;
    std::vector<glm::vec2> Uvs;
    std::vector<glm::vec3> Normals;

    GLuint ProgramID;
    std::unordered_map<std::string, GLuint> Values;
    std::unordered_map<std::string, GLuint> Textures;

    GLuint VertexBuffer;
    GLuint UvBuffer;
    GLuint NormalsID;

    mat4 Scale;

    ~MetaObject() {
        // Cleanup VBO and shader
        glDeleteBuffers(1, &VertexBuffer);
        glDeleteBuffers(1, &UvBuffer);
        glDeleteBuffers(1, &NormalsID);
        glDeleteProgram(ProgramID);
        for (auto& texture : Textures)
            glDeleteTextures(1, &texture.second);
    }
};

class Object {
public:
    virtual ~Object() = default;

    MetaObject* meta;
    mat4 ModelMatrix;

    bool IsCollide(Object* o) {
        auto diff = ModelMatrix - o->ModelMatrix;
        float dist = std::sqrt(std::pow(diff[3][0], 2) + std::pow(diff[3][1], 2) + std::pow(diff[3][2], 2));
        return dist < 1.0f;
    }


};

class Enemy : public Object {
public:
    Enemy() = default;
    Enemy(MetaObject* meta, vec3 position) {
        this->meta = meta;
        ModelMatrix = translate(mat4(), position);
    }

    void Draw() {
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        //glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(meta->Values["MatrixID"], 1, GL_FALSE, &MVP[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, meta->Textures["Texture"]);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(meta->Values["TextureID"], 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, meta->VertexBuffer);
        glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, meta->UvBuffer);
        glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, meta->Vertices.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
};

class Fireball : public Object {
public:
    Fireball() = default;
    Fireball(MetaObject* meta, vec3 forward, vec3 position) : Forward(forward), Position(position + forward), SpawnPosition(position) {
        this->meta = meta;
        ModelMatrix = translate(mat4(), forward * 0.1f);
    }

    bool Update(float dt) {
        Position += Forward * kSpeed * dt;
        ModelMatrix = translate(mat4(), Position) * meta->Scale;
        return std::sqrt(std::pow(Position[0] - SpawnPosition[0], 2) + std::pow(Position[1] - SpawnPosition[1], 2) + std::pow(Position[2] - SpawnPosition[2], 2)) <= 7;
    }

    void Draw() {
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        //glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(meta->Values["MatrixID"], 1, GL_FALSE, &MVP[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, meta->Textures["Fire"]);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(meta->Values["TextureID"], 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, meta->Textures["Noise"]);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(meta->Values["NoiseTextureID"], 1);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, meta->VertexBuffer);
        glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, meta->UvBuffer);
        glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, meta->NormalsID);
        glVertexAttribPointer(
            2,                                // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, meta->Vertices.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    static constexpr float kSpeed = 1;

    const vec3 Forward;
    vec3 Position;
    vec3 SpawnPosition;
};

template <class T>
class ObjectGenerator {
public:
    ObjectGenerator(MetaObject& meta, std::random_device& rd) :
        counter_(0),
        last_spawn_time_(glfwGetTime()),
        gen_(rd()),
        len_distr_(2.0f, 6.0f),
        alpha_distr_(0.0f, 2 * pi<float>()),
        meta_(&meta)
    { }
    void Spawn(size_t N, std::vector<std::unique_ptr<T>>& objects) {
        if (counter_ < 13 && glfwGetTime() - last_spawn_time_ > N) {
            last_spawn_time_ = glfwGetTime();
            const double r = len_distr_(gen_);
            const double phi = alpha_distr_(gen_);
            const double psi = alpha_distr_(gen_);
            glm::vec3 center = glm::vec3(
                cos(phi) * sin(psi) * r,
                sin(phi) * r,
                cos(phi) * cos(psi) * r
            );
            objects.emplace_back(std::make_unique<T>(meta_, center));
            ++counter_;
        }
    }
private:
    int counter_;
    double last_spawn_time_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<> len_distr_;
    std::uniform_real_distribution<> alpha_distr_;
    MetaObject* meta_;
};

void save(std::vector<std::unique_ptr<Enemy>>& enemies,
    std::vector<std::unique_ptr<Fireball>>& balls,
    int kills,
    std::string save_name) {
    std::ofstream out(save_name, std::fstream::out | std::fstream::trunc);
    auto pos = getPosition();
    auto angels = getAngels();
    out << kills << ' ' << enemies.size() << ' ' << balls.size() << '\n'
        << pos[0] << ' ' << pos[1] << ' ' << pos[2] << '\n'
        << angels.first << ' ' << angels.second << '\n';
    for (auto& e : enemies) {
        auto& matrix = e->ModelMatrix;
        out << matrix[3][0] << ' ' << matrix[3][1] << ' ' << matrix[3][2] << ' ';
    }
    for (auto& b : balls) {
        out << b->Position[0] << ' ' << b->Position[1] << ' ' << b->Position[2] << ' '
            << b->Forward[0] << ' ' << b->Forward[1] << ' ' << b->Forward[2] << ' ';
    }
}

void load(std::vector<std::unique_ptr<Enemy>>& enemies,
    std::vector<std::unique_ptr<Fireball>>& balls,
    int& kills,
    std::string save_name,
    std::pair<MetaObject*, MetaObject*> metas) {
    std::ifstream in(save_name, std::fstream::in);
    auto esize = 0;
    auto bsize = 0;
    in >> kills >> esize >> bsize;
    vec3 position;
    std::pair<float, float> angels;
    in >> position[0] >> position[1] >> position[2] >> angels.first >> angels.second;
    setAngels(angels.first, angels.second);
    setPosition(position);
    for (auto i = 0; i < esize; ++i) {
        float x, y, z;
        in >> x >> y >> z;
        enemies.emplace_back(std::make_unique<Enemy>(metas.first, vec3{ x, y, z }));
    }
    for (auto i = 0; i < bsize; ++i) {
        vec3 pos;
        vec3 forw;
        in >> pos[0] >> pos[1] >> pos[2] >> forw[0] >> forw[1] >> forw[2];
        balls.emplace_back(std::make_unique<Fireball>(metas.second, forw, pos - forw));
        balls.back()->SpawnPosition = getPosition();
    }
}


int main(void)
{
    // Initialise GLFW

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Tutorial 07 - Model Loading", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    initText2D("Holstein.DDS");

    MetaObject MetaEnemy("haha.obj");
    MetaEnemy.Textures["Texture"] = loadDDS("enemy.dds");
    MetaEnemy.ProgramID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
    // Get a handle for our "MVP" uniform
    MetaEnemy.Values["MatrixID"] = glGetUniformLocation(MetaEnemy.ProgramID, "MVP");

    // Get a handle for our "myTextureSampler" uniform
    MetaEnemy.Values["TextureID"] = glGetUniformLocation(MetaEnemy.ProgramID, "myTextureSampler");

    MetaObject MetaBall("sphere.obj");
    MetaBall.Scale = glm::scale(mat4(), { 0.1f, 0.1f, 0.1f });
    MetaBall.Textures["Fire"] = loadBMP_custom("fire.bmp");
    MetaBall.Textures["Noise"] = loadDDS("texture.dds");
    MetaBall.ProgramID = LoadShaders("FireTransformVertexShader.vertexshader", "FireTextureFragmentShader.fragmentshader");
    MetaBall.Values["MatrixID"] = glGetUniformLocation(MetaBall.ProgramID, "MVP");
    MetaBall.Values["TextureID"] = glGetUniformLocation(MetaBall.ProgramID, "myTextureSampler");
    MetaBall.Values["NoiseTextureID"] = glGetUniformLocation(MetaBall.ProgramID, "noiseTex");
    MetaBall.Values["itime"] = glGetUniformLocation(MetaBall.ProgramID, "itime");

    std::vector<std::unique_ptr<Enemy>> objs;
    std::vector<std::unique_ptr<Fireball>> balls;
    std::random_device rd;
    ObjectGenerator<Enemy> gg(MetaEnemy, rd);

    auto last_time = glfwGetTime();
    int mouseState = GLFW_RELEASE;
    int saveState = GLFW_RELEASE;
    int loadState = GLFW_RELEASE;
    auto kills = 0;

    int ct = 0;

    do {
        auto time = glfwGetTime();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(MetaEnemy.ProgramID);

        // Compute the MVP matrix from keyboard and mouse input


        int curSaveState = glfwGetKey(window, GLFW_KEY_S);
        if (curSaveState == GLFW_RELEASE && saveState == GLFW_PRESS) {
            save(objs, balls, kills, "cool_save");
        }
        saveState = curSaveState;

        int curLoadState = glfwGetKey(window, GLFW_KEY_L);
        if (curLoadState == GLFW_RELEASE && loadState == GLFW_PRESS) {
            std::vector<std::unique_ptr<Enemy>> nobjs;
            std::vector<std::unique_ptr<Fireball>> nballs;
            load(nobjs, nballs, kills, "cool_save", { &MetaEnemy, &MetaBall });
            objs = std::move(nobjs);
            balls = std::move(nballs);
        }
        loadState = curLoadState;

        computeMatricesFromInputs();
        int currMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (mouseState == GLFW_RELEASE && currMouseState == GLFW_PRESS) {
            balls.emplace_back(std::make_unique<Fireball>(&MetaBall, getForward(), getPosition()));
        }
        mouseState = currMouseState;

        std::vector<int> bad_enemys;
        std::vector<int> bad_balls;

        gg.Spawn(1, objs);
        for (auto& obj : objs)
            obj->Draw();

        glUseProgram(MetaBall.ProgramID);

        glUniform1f(MetaBall.Values["itime"], time * 0.3);

        for (auto i = 0; i < balls.size(); ++i) {
            if (!balls[i]->Update(time - last_time))
                bad_balls.push_back(i);

            balls[i]->Draw();
        }

        for (auto i = 0; i < balls.size(); ++i) {
            for (auto j = 0; j < objs.size(); ++j) {
                if (balls[i]->IsCollide(objs[j].get())) {
                    bad_balls.push_back(i);
                    bad_enemys.push_back(j);
                    ++kills;
                }
            }
        }

        for (auto x : bad_balls)
            balls.erase(balls.begin() + x);
        for (auto x : bad_enemys)
            objs.erase(objs.begin() + x);



        printText2D("Wee-Wee Ball", 10, 10, 50);
        char killsT[5];
        sprintf(killsT, "%u", kills);
        printText2D(killsT, 10, 100, 40);

        char fpsT[5];
        auto fps = time - last_time < 1e-6 ? 1337 : static_cast<int>(1 / (time - last_time));
        sprintf(fpsT, "%d", fps);
        printText2D(fpsT, 10, 200, 40);

        last_time = time;
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    glDeleteVertexArrays(1, &VertexArrayID);
    cleanupText2D();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

