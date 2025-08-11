#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

#include "stdint.hpp"

namespace lib
{
    template <typename T>
    struct remove_reference
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T&>
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T&&>
    {
        typedef T type;
    };

    template <typename T>
    struct remove_extent
    {
        typedef T type;
    };

    template <typename T>
    struct remove_extent<T[]>
    {
        typedef T type;
    };

    template <typename T, size_t N>
    struct remove_extent<T[N]>
    {
        typedef T type;
    };

    template <typename T>
    using remove_extent_t = typename remove_extent<T>::type;


    template <typename T>
    constexpr T &&forward(remove_reference<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <typename T>
    constexpr T &&forward(remove_reference<T>&& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template<bool B, class T = void>
    struct enable_if {};

    template<class T>
    struct enable_if<true, T>
    {
        typedef T type;
    };

    template<bool B, class T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    template <class T, T v>
    struct integral_constant
    {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant;
        constexpr operator value_type() const noexcept
        {
            return value;
        }

        constexpr value_type operator()() const noexcept
        {
            return value;
        }
    };

    using false_type = integral_constant<bool, false>;
    using true_type = integral_constant<bool, true>;

    template <class T, class U>
    struct is_same : false_type {};

    template <class T>
    struct is_same<T, T> : true_type {};

    template <class T, class U>
    inline constexpr bool is_same_v = is_same<T, U>::value;

    template <class>
    struct is_integral : false_type {};
    template <> struct is_integral<int8_t>    : true_type {};
    template <> struct is_integral<int16_t>   : true_type {};
    template <> struct is_integral<int32_t>   : true_type {};
    template <> struct is_integral<int64_t>   : true_type {};
    template <> struct is_integral<uint8_t>   : true_type {};
    template <> struct is_integral<uint16_t>  : true_type {};
    template <> struct is_integral<uint32_t>  : true_type {};
    template <> struct is_integral<uint64_t>  : true_type {};

    template <class T>
    inline constexpr bool is_integral_v = is_integral<T>::value;

    template<class T>
    struct is_floating_point : false_type {};
    template <> struct is_floating_point<float>    : true_type {};
    template <> struct is_floating_point<double>   : true_type {};
    template <> struct is_floating_point<long double> : true_type {};

    template <class T>
    inline constexpr bool is_floating_point_v = is_floating_point<T>::value;
}

// explanation: https://en.wikibooks.org/wiki/More_C++_Idioms/Member_Detector
#define reflection_has_member(member)                                                                    \
    template <class K>                                                                                   \
    class __HAS_MEMBER__##member                                                                         \
    {                                                                                                    \
    private:                                                                                             \
        using yes = char[2];                                                                             \
        using no  = char[1];                                                                             \
                                                                                                         \
        struct fallback { int member; };                                                                 \
        struct derived : K, fallback {};                                                                 \
                                                                                                         \
        template <class U>                                                                               \
        static no &check(decltype(U::member)*);                                                          \
                                                                                                         \
        template <class U>                                                                               \
        static yes &check(U*);                                                                           \
                                                                                                         \
    public:                                                                                              \
        static constexpr bool RESULT = (sizeof(check<derived>(nullptr)) == sizeof(yes));                 \
    };                                                                                                   \
                                                                                                         \
    template <class K>                                                                                   \
    struct has_member_##member : public klib::integral_constant<bool, __HAS_MEMBER__##member<K>::RESULT> \
    {                                                                                                    \
    };

#endif // TYPE_TRAITS_HPP