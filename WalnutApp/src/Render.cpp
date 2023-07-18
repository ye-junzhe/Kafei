#include "Render.h"
#include <cstddef>

namespace Utils {

    static uint32_t ConvertToRGBA(const glm::vec4& color) {
        uint8_t r = uint8_t(color.r * 255.0f);
        uint8_t g = uint8_t(color.g * 255.0f);
        uint8_t b = uint8_t(color.b * 255.0f);
        uint8_t a = uint8_t(color.a * 255.0f);
        
        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }
}

void Renderer::Render(const Scene& scene, const Camera& camera) {

    m_ActiveScene = &scene;
    m_ActiveCamera = &camera;

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            glm::vec4 color = PerPixel(x, y);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->SetData(m_ImageData);
}

// void Renderer::RenderSSAA(float SSAA_level, const Camera& camera, const Scene& scene) {
//     Ray ray;
//     ray.Origin = camera.GetPosition();
//
//     for (uint32_t y = 0; y < m_FinalImage->GetWidth() * SSAA_level; y++) {
//         for (uint32_t x = 0; x < m_FinalImage->GetHeight() * SSAA_level; x++) {
//             uint32_t sampleCount = 0;
//             glm::vec4 color(0.0f);
//
//             // Generate multiple rays per pixel
//             for (uint32_t subY = -SSAA_level; subY <= SSAA_level; subY += SSAA_level) {
//                 for (uint32_t subX = -SSAA_level; subX <= SSAA_level; subX += SSAA_level) {
//                     uint32_t sampleX = (x + subX);
//                     uint32_t sampleY = (y + subY);
//
//                     if (sampleX >= 0 && sampleX < (m_FinalImage->GetWidth()) &&
//                         sampleY >= 0 && sampleY < (m_FinalImage->GetHeight())) {
//                         ray.Direction = camera.GetRayDirections()[sampleX + sampleY * m_FinalImage->GetWidth()];
//                         color += TraceRay(scene, ray);
//                         sampleCount++;
//                     }
//                 }
//             }
//
//             color /= sampleCount;
//             color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
//             m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
//         }
//     }
//     m_FinalImage->Resize(m_FinalImage->GetWidth(), m_FinalImage->GetHeight());
//     m_FinalImage->SetData(m_ImageData);
// }

void Renderer::OnResize(uint32_t width, uint32_t height) {
    if (m_FinalImage) {
        if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) return;
        m_FinalImage->Resize(width, height);
    }
    else {
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[width * height];
}


// WARNING: Negative is farther
Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

    int closestSphere = -1;
    float hitDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++) {

        const Sphere& sphere = m_ActiveScene->Spheres[i];
        glm::vec3 origin = ray.Origin - sphere.Position;

        float a = glm::dot(ray.Direction, ray.Direction);
        float b = 2.0f * glm::dot(origin, ray.Direction);
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f) {
            continue; // move to next sphere
        }
        float closerT = ((-b - glm::sqrt(discriminant)) / (2.0f * a));
        /*
            The farther point which is not using currently
            float fartherT = ((-b + glm::sqrt(discriminant)) / (2.0f * a));
            glm::vec3 fartherPoint = origin + ray.Direction * fartherT;
        */
        if (closerT > 0.0f && closerT < hitDistance) {
            hitDistance = closerT;
            closestSphere = (int)i;
        }
    }

    if (closestSphere < 0)
        return Miss(ray);

    return ClosestHit(ray, hitDistance, closestSphere);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

    glm::vec3 color(0.0f);
    float multiplier = 1.0f;

    int bounces = 5;
    for (int i = 0; i < bounces; i++) {
        Renderer::HitPayload payload = TraceRay(ray);
        if (payload.HitDistance < 0.0f){
            glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
            color += skyColor * multiplier;
            break;
        }
        // The direction of the light
        glm::vec3 lightDir = m_ActiveScene->lightDir;
        // how strong the light will be
        // if cos value < 0, which means the angle between the normal and the -lightdir is too big
        // therefor causing the lightness to be very low
        float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

        const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
        const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

        glm::vec3 sphereColor = material.Albedo;
        sphereColor *= lightIntensity;
        color += sphereColor * multiplier;
        multiplier *= 0.5f;

        // NOTE: Move the next Origin a little bit farther along the normal directionl
        // Prevent the next Origin from being inside the sphere due to float precision problem
        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
        ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
    }

    return glm::vec4(color, 1);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) {
    Renderer::HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;

    const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

    glm::vec3 origin = ray.Origin - closestSphere.Position;
    payload.WorldPosition= origin + ray.Direction * hitDistance;
    // The normal on the sphere of the hit point
    payload.WorldNormal = glm::normalize(payload.WorldPosition);
    payload.WorldPosition += closestSphere.Position;

    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray) {
    Renderer::HitPayload payload;
    payload.HitDistance = -1.0f;
    return payload;
}
