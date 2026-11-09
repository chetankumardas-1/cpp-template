#pragma once

class Calculator
{
public:
    int add(int lhs, int rhs) const noexcept
    {
        return lhs + rhs;
    }

    int subtract(int lhs, int rhs) const noexcept
    {
        return lhs - rhs;
    }

    int multiply(int lhs, int rhs) const noexcept
    {
        return lhs * rhs;
    }
};