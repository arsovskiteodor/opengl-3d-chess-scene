#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <GL/glew.h>
#include "tiny_obj_loader.h" // Include tinyobj header for tinyobj::shape_t

struct Object3D {
    std::string filename;                        // Path to the .obj file
    std::string path;                            // Path to the object
    std::vector<tinyobj::shape_t> shapes;        // Shapes loaded from the .obj file
    glm::mat4 model;                             // Model transformation matrix
    glm::mat3 normalMatrix;                      // Normal matrix
    GLuint u_texture;                            // Texture uniform ID
    std::vector<GLuint> vao;                     // VAO IDs
    GLuint textureID;                            // Texture ID
    std::vector<GLuint> numTriangles;            // Number of triangles per shape
    GLuint modelLoc;                             // Model matrix uniform location
    GLuint normalMatrixLoc;                      // Normal matrix uniform location
    unsigned char* pixels;                       // Pixel data for the texture
};

#endif // OBJECT3D_H
