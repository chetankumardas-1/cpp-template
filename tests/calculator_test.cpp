#include <gtest/gtest.h>
#include "calculator.h"

TEST(CalculatorTest, AddsPositiveNumbers)
{
    Calculator calculator;
    EXPECT_EQ(calculator.add(2, 3), 5);
}

TEST(CalculatorTest, SubtractsNumbers)
{
    Calculator calculator;
    EXPECT_EQ(calculator.subtract(10, 4), 6);
}

TEST(CalculatorTest, MultipliesNumbers)
{
    Calculator calculator;
    EXPECT_EQ(calculator.multiply(3, 4), 12);
}