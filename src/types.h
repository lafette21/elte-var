#ifndef TYPES_H
#define TYPES_H

#include <imgui.h>

#include <compare>

namespace var {

struct vec2 {
    constexpr vec2() = default;
    constexpr vec2(float x, float y):
        x(x), y(y)
    {}
    constexpr vec2(ImVec2 obj):
        x(obj.x), y(obj.y)
    {}

    auto operator<=>(const vec2&) const = default;

    operator ImVec2() const {
        return { x, y };
    }

    float x;
    float y;
};

inline vec2 operator+(const vec2& lhs, const vec2& rhs) noexcept {
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

inline vec2 operator+(const ImVec2& lhs, const vec2& rhs) noexcept {
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y
    };
}

inline vec2 operator+(const vec2& lhs, const ImVec2& rhs) noexcept {
    return rhs + lhs;
}

inline vec2 operator-(const vec2& lhs, const vec2& rhs) noexcept {
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y
    };
}

inline vec2 operator-(const ImVec2& lhs, const vec2& rhs) noexcept {
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y
    };
}

inline vec2 operator-(const vec2& lhs, const ImVec2& rhs) noexcept {
    return rhs - lhs;
}

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
