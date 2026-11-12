#ifndef CORE_TIME_HPP
#define CORE_TIME_HPP

namespace Core {

class Time {
public:
    static inline float deltaTime = 0.016f;
    static inline float time = 0.0f;
    static inline float timeScale = 1.0f;
    static inline int frameCount = 0;
    static inline float fps = 60.0f;

    static void update(float dt) {
        deltaTime = dt * timeScale;
        time += deltaTime;
        frameCount++;
        if (dt > 1e-5f) {
            fps = 1.0f / dt;
        }
    }
};

} // namespace Core

#endif // CORE_TIME_HPP
