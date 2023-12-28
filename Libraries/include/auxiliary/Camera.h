#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


// Default camera values
namespace cam {
    extern float YAW;
    extern float PITCH;
    extern float SPEED;
    extern float SENSITIVITY;
    extern float DEF_FOV;
    extern float MIN_FOV;
    extern float MAX_FOV;
}


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
private:
    // camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Fov;
    bool isActive;

public:
    // camera attributes
    inline glm::vec3 getPosition() { return Position; } const
    inline glm::vec3 getFront() { return Front; } const
    inline glm::vec3 getUp() { return Up; } const
    inline glm::vec3 getRight() { return Right; } const
    inline glm::vec3 getWorldUp() { return WorldUp; } const
    // euler angles
    inline float getYaw() { return Yaw; } const
    inline float getPitch() { return Pitch; } const
    // camera options
    inline float getMovementSpeed() { return MovementSpeed; } const
    inline float getMouseSensitivity() { return MouseSensitivity; } const
    inline float getFov() { return Fov; } const
    inline bool getIsActive() { return isActive; } const

    // camera attributes
    inline void setPosition(const glm::vec3& _position) { Position = _position; }
    inline void setFront(const glm::vec3& _front) { Front = _front; }
    inline void setUp(const glm::vec3& _up) { Up = _up; }
    inline void setRight(const glm::vec3& _right) { Right = _right; }
    inline void setWorldUp(const glm::vec3& _worldUp) { WorldUp = _worldUp; }
    // Euler angles
    inline void setYaw(const float _yaw) { Yaw = _yaw; }
    inline void setPitch(const float _pitch) { Pitch = _pitch; }
    // camera options
    inline void setMovementSpeed(const float _movementSpeed) { MovementSpeed = _movementSpeed; }
    inline void setMouseSensitivity(const float _mouseSensitivity) { MouseSensitivity = _mouseSensitivity; }
    inline void setFov(const float _fov) { Fov = _fov; }
    inline void setIsActive(const bool _isActive) { isActive = _isActive; }

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();

    // turn camera to look at given point in the world
    void CameralootAt(const glm::vec3& _lookAtPoint);


    // constructor with vectors
    Camera(
        const glm::vec3& _position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& _worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
        float _yaw = cam::YAW,
        float _pitch = cam::PITCH
    );

    // constructor with scalar values
    Camera(
        float _posX, float _posY, float _posZ,
        float _worldUpX, float _worldUpY, float _worldUpZ,
        float _yaw,
        float _pitch
    );

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    inline glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum class Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement _direction, float _deltaTime);

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float _xOffset, float _yOffset, GLboolean _constrainPitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float _yOffset);
};