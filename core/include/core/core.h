#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <utility>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using idx   = std::ptrdiff_t;
using usize = std::size_t;
using isize = std::ptrdiff_t;
using byte  = std::byte;

template<typename T>
using rc = std::shared_ptr<T>;

template<typename T>
using weak = std::weak_ptr<T>;

template<typename T, typename D = std::default_delete<T>>
using up = std::unique_ptr<T, D>;

template<typename T, typename... Args>
auto make_up(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename... Args>
auto make_rc(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

struct NoCopy {
protected:
    NoCopy()  = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy&)            = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

namespace core
{

template<typename Wrapper>
static inline typename Wrapper::pointer GetPtrHelper(const Wrapper& p) {
    return p.get();
}

template<typename, template<typename...> class>
constexpr bool is_specialization_of = false;
template<template<typename...> class T, typename... Args>
constexpr bool is_specialization_of<T<Args...>, T> = true;
} // namespace core

template<typename TType>
struct To {
    using out_type = TType;
    template<typename T>
    struct From;

    template<typename T>
    static TType from(const T& t) {
        return From<T>::from(t);
    }
};

template<typename F, typename T>
concept to_able = requires(F f) {
    { To<T>::from(f) } -> std::same_as<T>;
};

template<typename IMPL>
struct CRTP {
protected:
    IMPL&       crtp_impl() { return *static_cast<IMPL*>(this); }
    const IMPL& crtp_impl() const { return *static_cast<const IMPL*>(this); }
};

#define C_DECLARE_PRIVATE(Class, DName)                                            \
    inline Class::Private* d_func() {                                              \
        return reinterpret_cast<Class::Private*>(core::GetPtrHelper(DName));       \
    }                                                                              \
    inline const Class::Private* d_func() const {                                  \
        return reinterpret_cast<const Class::Private*>(core::GetPtrHelper(DName)); \
    }

#define C_D(Class)       Class::Private* const d = d_func()
#define C_DP(Class, Ptr) Class::Private* const d = Ptr->d_func()

#define C_DECLARE_PUBLIC(Class, QName)                                              \
    inline Class*       q_func() { return static_cast<Class*>(QName); }             \
    inline const Class* q_func() const { return static_cast<const Class*>(QName); } \
    friend class Class;

#define C_Q(Class) Class* const q = q_func()
