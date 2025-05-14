#ifndef VEC_HPP_
#define VEC_HPP_

template <typename T>
struct Vec2 {
  T x, y;
};

// * constructor
template <typename T>
Vec2<T> vec2(T x, T y) {
  return {x, y};
}

// * integer alias
using Vec2i = Vec2<int>;

// * /////////////////////////////////////////
// * Scalar Multiplication (Vec2 x Vec2)
// * /////////////////////////////////////////

template <typename T>
Vec2<T> operator+(Vec2<T> a, Vec2<T> b) {
  return {a.x + b.x, a.y + b.y};
}

template <typename T>
Vec2<T> operator-(Vec2<T> a, Vec2<T> b) {
  return {a.x - b.x, a.y - b.y};
}

template <typename T>
Vec2<T> operator*(Vec2<T> a, Vec2<T> b) {
  return {a.x * b.x, a.y * b.y};
}

template <typename T>
Vec2<T> operator/(Vec2<T> a, Vec2<T> b) {
  return {a.x / b.x, a.y / b.y};
}

static inline
int get_sqr_dist(Vec2i p0, Vec2i p1) {
  auto d = p0 - p1;
  return d.x * d.x + d.y * d.y;
}

template <typename T>
Vec2<T> &operator+=(Vec2<T> &a, Vec2<T> b) { a = a + b; return a; }

// * /////////////////////////////////////////
// * Scalar Multiplication (Vec2 x Scalar)
// * /////////////////////////////////////////

template <typename T>
Vec2<T> operator+(Vec2<T> a, T b) {
  return {a.x + b, a.y + b};
}

template <typename T>
Vec2<T> operator-(Vec2<T> a, T b) {
  return {a.x - b, a.y - b};
}

template <typename T>
Vec2<T> operator*(Vec2<T> a, T b) {
  return {a.x * b, a.y * b};
}

template <typename T>
Vec2<T> operator/(Vec2<T> a, T b) {
  return {a.x / b, a.y / b};
}

// * /////////////////////////////////////////
// * Scalar Multiplication (Scalar x Vec2)
// * /////////////////////////////////////////

template <typename T>
Vec2<T> operator+(T a, Vec2<T> b) {
  return {a + b.x, a + b.y};
}

template <typename T>
Vec2<T> operator-(T a, Vec2<T> b) {
  return {a - b.x, a - b.y};
}

template <typename T>
Vec2<T> operator*(T a, Vec2<T> b) {
  return {a * b.x, a * b.y};
}

template <typename T>
Vec2<T> operator/(T a, Vec2<T> b) {
  return {a / b.x, a / b.y};
}

#endif // * VEC_HPP_
