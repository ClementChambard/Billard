#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>

class Camera {

    public:
        Camera();

        glm::mat4 getMat() { return viewMat; }

        void move(float pitch, float yaw, float forward, float right, float up);

    private:

        float pitch = 0.f;
        float yaw = 0.f;
        glm::vec3 pos;
        glm::vec3 lookat;

        glm::mat4 viewMat;
        glm::vec3 up = glm::vec3(0,1,0);

};

#endif // CAMERA_H_
