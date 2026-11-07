#include "Graphics/Mesh.hpp"

#include <stdexcept>

void Mesh::addVertex(const Vector3& vertex) {
    vertices.push_back(vertex);
}

void Mesh::addEdge(unsigned int start, unsigned int end) {
    if (start >= vertices.size() || end >= vertices.size()) {
        throw std::out_of_range(
            "Mesh edge references a vertex that does not exist."
        );
    }

    edges.push_back({start, end});
}

void Mesh::addFace(const std::vector<unsigned int>& vertexIndices) {
    if (vertexIndices.size() < 3) {
        throw std::invalid_argument(
            "A mesh face must contain at least three vertices."
        );
    }

    for (unsigned int index : vertexIndices) {
        if (index >= vertices.size()) {
            throw std::out_of_range(
                "Mesh face references a vertex that does not exist."
            );
        }
    }

    faces.push_back({vertexIndices});
}

const std::vector<Vector3>& Mesh::getVertices() const {
    return vertices;
}

const std::vector<Edge>& Mesh::getEdges() const {
    return edges;
}

const std::vector<Face>& Mesh::getFaces() const {
    return faces;
}