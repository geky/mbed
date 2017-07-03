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
#include "SimulatorTest.h"
#include "unity/unity.h"
#include "utest/utest.h"

using namespace utest::v1;

SimulatorTest *SimulatorTest::test_list_head;
SimulatorTest *SimulatorTest::test_list_tail;

SimulatorTest::SimulatorTest(test_case_func_t test_case, const char *test_name) : next(NULL), function(test_case), name(test_name)
{
    if (test_list_head) {
        test_list_tail->next = this;
        test_list_tail = this;
    } else {
        test_list_head = this;
        test_list_tail = this;
    }
}

int SimulatorTest::run_tests()
{
    int test_count = 0;
    SimulatorTest *test = test_list_head;
    while (test) {
        test_count++;
        test = test->next;
    }

    Case *cases = (Case*)operator new[](test_count * sizeof(Case));

    test = test_list_head;
    for (int i = 0; i < test_count; i++) {
        new(&cases[i]) Case(test->name, test->function);
        test = test->next;
    }

    Specification specification(verbose_test_setup_handler, cases, test_count, verbose_test_teardown_handler, verbose_test_failure_handler);
    Harness::run(specification);

    for (int i = 0; i < test_count; i++) {
        cases[i].~Case();
    }

    operator delete[]((void*)cases);
    return 0;
}

int simulator_test_main()
{
    return SimulatorTest::run_tests();
}
