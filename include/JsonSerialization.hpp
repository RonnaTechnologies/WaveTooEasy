#pragma once
#define NTEST
#define ARDUINOJSON_ENABLE_PROGMEM 0

#include "reflect.hpp"
#include <ArduinoJson.h>

#include <type_traits>

template <typename T>
void toJson(const T& obj, JsonObject dst)
{
    reflect::for_each(
    [&](auto I)
    {
        const auto& field = reflect::get<I>(obj);
        using FieldT = std::remove_cvref_t<decltype(field)>;
        if constexpr (std::is_enum_v<FieldT>)
        {
            dst[reflect::member_name<I>(obj)] = static_cast<std::underlying_type_t<FieldT>>(field);
        }
        else
        {
            dst[reflect::member_name<I>(obj)] = field;
        }
    },
    obj);
}

template <typename T>
T fromJson(JsonObjectConst src)
{
    T obj{};
    reflect::for_each(
    [&](auto I)
    {
        auto& field = reflect::get<I>(obj);
        using FieldT = std::remove_cvref_t<decltype(field)>;
        if constexpr (std::is_enum_v<FieldT>)
        {
            using Underlying = std::underlying_type_t<FieldT>;
            field = static_cast<FieldT>(src[reflect::member_name<I>(obj)].template as<Underlying>());
        }
        else
        {
            field = src[reflect::member_name<I>(obj)].template as<FieldT>();
        }
    },
    obj);
    return obj;
}