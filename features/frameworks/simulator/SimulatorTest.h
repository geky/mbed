// The 'features' section in 'target.json' is now used to create the device's hardware preprocessor switches.
// Check the 'features' section of the target description in 'targets.json' for more details.
/* mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_SIMULATOR_TEST_H
#define MBED_SIMULATOR_TEST_H

#include "unity.h"

/**
 * Macro that can be used to declare a test anywhere
 *
 * Psuedo Code:
 * @code
 * MBED_TEST(AdditionTest, TestOnePlusOne)
 * {
 *     int value = 1 + 1;
 *     TEST_ASSERT_EQUAL(2, value);
 * }
 * @endcode
 */
#define MBED_TEST(test_case_name, test_name)                                \
    static void simulator_test##test_case_name##test_name(void);            \
    SimulatorTest obj_simulator_test##test_case_name##test_name             \
        (simulator_test##test_case_name##test_name,                         \
         #test_case_name "-" #test_name);                                   \
    void simulator_test##test_case_name##test_name(void)

// Disable registration code for non-simulator builds
#if !defined(TARGET_SIM)
#undef MBED_TEST
#define MBED_TEST(name, _) void simulator_test##name(void)
#endif

typedef void (*test_case_func_t)(void);

/**
 * Wrapper class for a single greentea test case
 */
class SimulatorTest {
public:
    /**
     * Register the test
     *
     * @param[in] test_case the test case function to exectute
     * @param[in] test_name the name of this test
     */
    SimulatorTest(test_case_func_t test_case, const char *test_name);

    /**
     * Run all registered tests using greentea
     */
    static int run_tests(void);
private:
    SimulatorTest *next;
    test_case_func_t function;
    const char * const name;

    static SimulatorTest *test_list_head;
    static SimulatorTest *test_list_tail;
};

/**
 * Entry point for global simulator test runner
 */
extern "C" int simulator_test_main(void);

#endif
