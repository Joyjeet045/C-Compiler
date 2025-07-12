#include "optimizer.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iterator>
#include "../target_keywords.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <set>

void Optimizer::optimize(std::string target_filename)
{
    std::vector<std::string> lines;
    target.open(target_filename);
    std::string line;
    while (std::getline(target, line)) {
        lines.push_back(line);
    }
    target.close();

    std::map<std::string, std::pair<int, int>> last_load_stackw_idx;
    std::set<int> redundant_indices;
    std::set<std::string> used_locations;

    for (int i = 1; i < lines.size(); ++i) {
        std::vector<std::string> words = split_line(lines[i]);
        if (words.empty()) continue;

        if (words[0] == "STACKW" && words.size() > 1) {
            std::string loc = words[1];
            std::vector<std::string> prev_words = split_line(lines[i-1]);
            if (!prev_words.empty() && (prev_words[0] == "LOAD" || prev_words[0] == "STACKR")) {
                if (last_load_stackw_idx.count(loc) && used_locations.find(loc) == used_locations.end()) {
                    redundant_indices.insert(last_load_stackw_idx[loc].first); 
                    redundant_indices.insert(last_load_stackw_idx[loc].second); 
                }
                last_load_stackw_idx[loc] = {i-1, i};
                used_locations.erase(loc);
            }
        } else if (words[0] == "STACKR" && words.size() > 1) {
            std::string loc = words[1];
            used_locations.insert(loc);
        }
    }

    std::ofstream tmp;
    std::string tmp_filename = "tmp" + target_filename;
    tmp.open(tmp_filename);
    for (int i = 0; i < lines.size(); ++i) {
        if (redundant_indices.find(i) == redundant_indices.end()) {
            tmp << lines[i] << std::endl;
        }
    }
    tmp.close();

    if (remove(target_filename.c_str()) != 0) {
        perror("Error deleting file");
    }
    if (rename(tmp_filename.c_str(), target_filename.c_str()) != 0) {
        perror("Error renaming file");
    }
}

bool Optimizer::is_line(std::string line, std::string target_keyword)
{
    return line.find(target_keyword) != std::string::npos;
}

std::vector<std::string> Optimizer::split_line(std::string line)
{
    std::istringstream iss(line);
    std::vector<std::string> words((std::istream_iterator<std::string>(iss)),
                                     std::istream_iterator<std::string>());
    return words;
}

int Optimizer::to_int(std::string str)
{
    int num;
    std::istringstream iss(str);
    iss >> num;
    return num;
}
