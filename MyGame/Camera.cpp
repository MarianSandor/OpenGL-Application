#include "Camera.hpp"
#include <iostream>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;

        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));

        this->worldUp = cameraUp;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(this->cameraPosition, this->cameraTarget, this->worldUp);
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    void Camera::setCameraPosition(glm::vec3 pos) {
        this->cameraPosition = pos;
        this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;
    }

    void Camera::setCameraTarget(glm::vec3 cameraTarget) {
        this->cameraTarget = cameraTarget;

        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->worldUp));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction)
        {
        case gps::MOVE_FORWARD:
            this->cameraPosition += this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_BACKWARD:
            this->cameraPosition -= this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_RIGHT:
            this->cameraPosition += this->cameraRightDirection * speed;
            break;
        case gps::MOVE_LEFT:
            this->cameraPosition -= this->cameraRightDirection * speed;
            break;
        case gps::MOVE_UP:
            this->cameraPosition += this->cameraUpDirection * speed;
            break;
        case gps::MOVE_DOWN:
            this->cameraPosition -= this->cameraUpDirection * speed;
            break;
        default:
            break;
        }

        this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 newFrontDirection;

        newFrontDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFrontDirection.y = sin(glm::radians(pitch));
        newFrontDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(newFrontDirection);

        this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->worldUp));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }
}