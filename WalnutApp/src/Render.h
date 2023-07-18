#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>
#include <limits>

class Renderer {
public:
    Renderer() = default;

    void Render(const Scene& scene, const Camera& camera);

    //  params: SSAA_level: determine the SSAA level, Gets blended in the background if the level is set too high
    void RenderSSAA(float SSAA_level, const Camera& camera, const Scene& scene);

    void OnResize(uint32_t, uint32_t);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }


private:
    struct HitPayload {
        float HitDistance;
        glm::vec3 WorldPosition;
        glm::vec3 WorldNormal;
        int ObjectIndex;
    };
    glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen

    HitPayload TraceRay( const Ray& ray);
    HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
    HitPayload Miss(const Ray& ray);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    const Scene* m_ActiveScene = nullptr;
    const Camera* m_ActiveCamera = nullptr;
    uint32_t* m_ImageData = nullptr;
};
