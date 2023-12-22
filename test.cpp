#include <iostream>
#include <string>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

int main() {
    vector<vector<vector<string>>> solutions = {
        { {"1/2","1/2"}, {"4/7","3/7","0"} },
        { {"1"}, {"1"} },
        { {"0","1"}, {"0","1"} },
        { {"1","0","0","0"}, {"1","0","0","0"} },
        { {"1","0","0","0","0"}, {"0","0","0","1","0"} },
        { {"1/2","0","0","0","0", "1/2"}, {"1/2","0","0","0","0", "1/2"} },
        { {"0","1","0","0","0","0","0","0","0","0","0","0","0","0"}, {"0","1","0","0","0","0","0","0","0","0","0","0","0","0"} },
        { {"1","0","0","0","0","0","0","0","0","0","0","0","0","0","0"}, {"0","1","0","0","0","0","0","0","0","0","0","0","0","0","0"} },
        { {"1","0","0","0","0"}, {"0","0","0","1","0","0","0","0","0","0","0","0","0","0","0"} },
        { {"1/2","0","0","0","0", "1/2","0","0","0","0","0","0","0","0","0"}, {"1/2","0","0","0","0", "1/2"} },
    };

    // run all test files
    vector<double> runningTimes;
    for (int i = 1; i <= 10; ++i) {
        string filename = "test" + to_string(i) + ".txt";
        string command = "./lemkeHowson < tests/" + filename;
        string command2 = "./lemkeHowson < tests/" + filename + " > dump";

        // run first time with output
        cout << "\nRunning test file: " << filename << endl;
        cout << "----------------------------------------" << endl;
        auto startTime = high_resolution_clock::now();
        int returnCode = system(command.c_str());
        auto stopTime = high_resolution_clock::now();
        if (returnCode != 0) {
            cerr << "Error occurred while running the test: " << filename << endl;
        }
        auto durationTotal = duration_cast<microseconds>(stopTime - startTime);

        // output expected equilibria (used online calculator)
        cout << "----------------------------------------" << endl;
        cout << "Expected Output\nEquilibrium for Player 1" << endl;
        for (auto val : solutions[i-1][0]) {
            cout << val << " ";
        }
        cout << "\n\nEquilibrium for Player 2" << endl;
        for (auto val : solutions[i-1][1]) {
            cout << val << " ";
        }
        cout << endl;
        cout << "----------------------------------------" << endl;

        // run 9 more times to get average execution time
        for (int i = 0; i < 9; i++) {
            startTime = high_resolution_clock::now();
            system(command2.c_str());
            stopTime = high_resolution_clock::now();
            durationTotal += duration_cast<microseconds>(stopTime - startTime);
        }
        
        // report time
        auto t = (durationTotal.count()/10.0)/1000.0;
        runningTimes.push_back(t);
        cout << "Time Taken: " << t << " ms" << endl;
        cout << "----------------------------------------" << endl;
    }

    // summary of size vs. execution time
    cout << "\n\nFinal Summary (Matrix Size vs. Running Time)" << endl;
    for (long unsigned int i = 0; i < solutions.size(); i++) {
        auto sol = solutions[i];
        cout << sol[0].size() << "x" << sol[1].size() << ": " << runningTimes[i] << "ms" << endl;
    }

    return 0;
}
