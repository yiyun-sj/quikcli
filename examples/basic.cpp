/*
 * A minimal quikcli demonstration: word count tool.
 *
 * Try:
 *   wc <filename>
 *   wc <filename> --lines
 *   wc <filename> -m 4
 *   wc --help
 */

#include <fstream>
#include <iostream>
#include <quikcli/command.hpp>
#include <sstream>
#include <string>

using namespace quikcli;

int main(int argc, char **argv) {
    auto cmd = Command::basic(
        "Count words or lines in a text file.",

        Flag<std::string>::anon("filename").doc("the file to read") &
            Flag<bool>::no_arg("lines").alias('l').doc("count lines instead of words") &
            Flag<int>::optional_with_default("min-length", 1)
                .alias('m')
                .doc("only count words at least this many characters long"),

        [](const std::string &filename, bool count_lines, int min_length) {
            std::ifstream in(filename);
            if (!in) {
                std::cerr << "error: cannot open '" << filename << "'\n";
                return;
            }
            long long n = 0;
            std::string line;
            while (std::getline(in, line)) {
                if (count_lines) {
                    ++n;
                } else {
                    std::istringstream iss(line);
                    std::string word;
                    while (iss >> word)
                        if (static_cast<int>(word.size()) >= min_length)
                            ++n;
                }
            }
            std::cout << n << "\t" << filename << "\n";
        });

    cmd.run(argc, argv, "1.0.0");
}
