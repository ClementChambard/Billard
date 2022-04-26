#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>

class Camera {

    public:
        // Basic constructor
        Camera();

        /**
         *  returns the view matrix of this camera
         */
        glm::mat4 getMat() { return viewMat; }

        /**
         *  changes the camera position and rotation
         *   - pitch (float) : the variation of angle between the up plane and the up axis
         *   - yaw   (float) : the variation of angle around the up axis
         *   - forward (float) : the variation of position along the look direction axis
         *   - right (float) : the variation of position along the right axis
         *   - up    (float) : the variation of position along the up axis
         */
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
