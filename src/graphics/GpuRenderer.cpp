#include "Graphics/GpuRenderer.hpp"

#include <SFML/OpenGL.hpp>

#include <cmath>

namespace {
constexpr float NEAR_PLANE = 0.12f;
constexpr float FAR_PLANE = 1000.0f;
constexpr float FIELD_OF_VIEW_FACTOR = 0.82f;

void applyTranslation(const Vector3& translation) {
    glTranslatef(translation.x, translation.y, translation.z);
}

void applyViewRotation(const Quaternion& orientation) {
    const Quaternion view = orientation.inverse();
    const float xx = view.x * view.x;
    const float yy = view.y * view.y;
    const float zz = view.z * view.z;
    const float xy = view.x * view.y;
    const float xz = view.x * view.z;
    const float yz = view.y * view.z;
    const float wx = view.w * view.x;
    const float wy = view.w * view.y;
    const float wz = view.w * view.z;
    const float rotation[16] = {
        1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
        2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
        2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(rotation);
}
}

void GpuRenderer::beginFrame(
    const sf::Vector2u& targetSize,
    float cameraDistance
) const {
    const float width = static_cast<float>(targetSize.x);
    const float height = static_cast<float>(targetSize.y);
    if (width <= 0.0f || height <= 0.0f) {
        return;
    }

    const float focalLength = height * FIELD_OF_VIEW_FACTOR * 0.5f;
    const float halfWidth = width * NEAR_PLANE / (2.0f * focalLength);
    const float halfHeight = height * NEAR_PLANE / (2.0f * focalLength);

    glViewport(0, 0, targetSize.x, targetSize.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(
        -halfWidth,
        halfWidth,
        -halfHeight,
        halfHeight,
        NEAR_PLANE,
        FAR_PLANE
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -cameraDistance);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glClearColor(8.0f / 255.0f, 12.0f / 255.0f, 20.0f / 255.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GpuRenderer::beginWorldView(
    float cameraDistance,
    const Quaternion& shipOrientation
) const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -cameraDistance);
    applyViewRotation(shipOrientation);
}

void GpuRenderer::beginCockpitView(float cameraDistance) const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -cameraDistance);
}

void GpuRenderer::beginDepthPass() const {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
}

void GpuRenderer::drawMeshDepth(
    const Mesh& mesh,
    const Vector3& translation
) const {
    glPushMatrix();
    applyTranslation(translation);

    const std::vector<Vector3>& vertices = mesh.getVertices();
    for (const Face& face : mesh.getFaces()) {
        if (face.vertexIndices.size() < 3) {
            continue;
        }

        glBegin(GL_TRIANGLE_FAN);
        bool validFace = true;
        for (unsigned int index : face.vertexIndices) {
            if (index >= vertices.size()) {
                validFace = false;
                break;
            }
            const Vector3& vertex = vertices[index];
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        glEnd();

        if (!validFace) {
            continue;
        }
    }

    glPopMatrix();
}

void GpuRenderer::beginColorPass() const {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
}

void GpuRenderer::drawStars(const std::vector<Vector3>& stars) const {
    glColor3ub(220, 230, 245);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const Vector3& star : stars) {
        glVertex3f(star.x, star.y, star.z);
    }
    glEnd();
}

void GpuRenderer::drawMeshEdges(
    const Mesh& mesh,
    const Vector3& translation,
    sf::Color color
) const {
    glPushMatrix();
    applyTranslation(translation);
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_LINES);

    const std::vector<Vector3>& vertices = mesh.getVertices();
    for (const Edge& edge : mesh.getEdges()) {
        if (edge.start >= vertices.size() || edge.end >= vertices.size()) {
            continue;
        }

        const Vector3& start = vertices[edge.start];
        const Vector3& end = vertices[edge.end];
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
    }

    glEnd();
    glPopMatrix();
}