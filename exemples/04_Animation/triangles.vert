#version 460 core

// Matrices
layout(location = 0) uniform mat4 uMatrix;
layout(location = 1) uniform mat3 uMatrixNormal;

// Vertex attributes
in vec4 vPosition;
in vec3 vNormal;

// Output vertices
out vec3 fNormal;

void main()
{
    // Output (pour le fragment shader)
    vec3 pos = vec3(uMatrix * vPosition); 
    fNormal = uMatrixNormal * vNormal;

    // Sortie dans l'espace NDC (normalized device coordinate)
    gl_Position = vec4(pos.x, pos.y, -pos.z, 1.0);
}

