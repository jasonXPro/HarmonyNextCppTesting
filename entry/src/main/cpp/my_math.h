//
// Created on 2024/7/20.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once
class MyMath {
public:
    int Add(int a, int b) {
        return a + b;
    }
};