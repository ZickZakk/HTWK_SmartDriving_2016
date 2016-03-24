//
// Created by pbachmann on 10/31/15.
//

#include "common/HTWKTest.h"

using namespace CppUnit;
using namespace std;

int main(int argc, char **argv)
{
    // Create the event manager and test controller
    TestResult controller;

    // Add a listener that collects test result
    TestResultCollector result;
    controller.addListener(&result);

    // Add the top suite to the test runner
    TestRunner runner;
    runner.addTest(TestFactoryRegistry::getRegistry().makeTest());

    // Listen to progress
    TestListener *listener;

    if (JetBrains::underTeamcity())
    {
        listener = new JetBrains::TeamcityProgressListener();
    }
    else
    {
        listener = new BriefTestProgressListener();
    }

    controller.addListener(listener);

    // Run test
    runner.run(controller);

    delete listener;

    return result.wasSuccessful() ? 0 : 1;
}
