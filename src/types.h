#ifndef TYPES_H
#define TYPES_H

namespace var {

struct vec2 {
    float x;
    float y;
};

struct vec4 {
    float x;
    float y;
    float z;
    float w;
};

namespace colors {
    constexpr auto black = vec4{   0.0f,   0.0f,   0.0f, 255.0f };
    constexpr auto white = vec4{ 255.0f, 255.0f, 255.0f, 255.0f };
}

} // namespace var

#endif // TYPES_H
