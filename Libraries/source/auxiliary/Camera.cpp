#include "auxiliary/Camera.h"


// Default camera values
namespace cam {
    extern float YAW = -90.0f;
    extern float PITCH = 0.0f;
    extern float SPEED = 5.0f;
    extern float SENSITIVITY = 0.05f;
    extern float DEF_FOV = 45.0f;
    extern float MIN_FOV = 0.5f;
    extern float MAX_FOV = 90.0f;
}


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL

// constructor with vectors
Camera::Camera(
    const glm::vec3 &_position,
    const glm::vec3 &_worldUp,
    float _yaw,
    float _pitch
)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(cam::SPEED), MouseSensitivity(cam::SENSITIVITY), Fov(cam::DEF_FOV)
{
    Position = _position;
    WorldUp = glm::normalize(_worldUp);
    Yaw = _yaw;
    Pitch = _pitch;
    isActive = true;
    updateCameraVectors();
}

// constructor with scalar values
Camera::Camera(
    float _posX, float _posY, float _posZ,
    float _worldUpX, float _worldUpY, float _worldUpZ,
    float _yaw,
    float _pitch
)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(cam::SPEED), MouseSensitivity(cam::SENSITIVITY), Fov(cam::DEF_FOV)
{
    Position = glm::vec3(_posX, _posY, _posZ);
    WorldUp = glm::normalize(glm::vec3(_worldUpX, _worldUpY, _worldUpZ));
    Yaw = _yaw;
    Pitch = _pitch;
    isActive = true;
    updateCameraVectors();
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors() {
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

// turn camera to look at given point in the world
void Camera::CameralootAt(const glm::vec3 &_lookAtPoint) {
    glm::vec3 vecToPoint = glm::normalize(_lookAtPoint - Position);
    Yaw = glm::degrees(atan2(vecToPoint.z, vecToPoint.x));
    Pitch = glm::degrees(asin(vecToPoint.y));

    updateCameraVectors();
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement _direction, float _deltaTime) {
    float velocity = MovementSpeed * _deltaTime;

    //glm::vec3 horizVec = Front;
    //horizVec.y = 0.0f;
    //horizVec = glm::normalize(horizVec);
    //horizVec = glm::cross(WorldUp, Right);

    if (_direction == Camera_Movement::FORWARD)
        Position += Front * velocity;
    if (_direction == Camera_Movement::BACKWARD)
        Position -= Front * velocity;
    if (_direction == Camera_Movement::LEFT)
        Position -= Right * velocity;
    if (_direction == Camera_Movement::RIGHT)
        Position += Right * velocity;

    // When movement is UP or DOWN and we use member:
    // WorldUp - camera will always move vertically along Y axis of the world;
    // Up - camera will move along Up vector of the camera, not the world.
    if (_direction == Camera_Movement::UP)
        Position += WorldUp * velocity;
    if (_direction == Camera_Movement::DOWN)
        Position -= WorldUp * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float _xOffset, float _yOffset, GLboolean _constrainPitch) {
    if (isActive) {
        _xOffset *= MouseSensitivity;
        _yOffset *= MouseSensitivity;

        Yaw += _xOffset;
        Pitch += _yOffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (_constrainPitch) {
            if (Pitch > 89.999f)
                Pitch = 89.999f;
            if (Pitch < -89.999f)
                Pitch = -89.999f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float _yOffset) {
    if (isActive) {
        Fov -= _yOffset;

        if (Fov < cam::MIN_FOV)
            Fov = cam::MIN_FOV;
        if (Fov > cam::MAX_FOV)
            Fov = cam::MAX_FOV;
    }
}