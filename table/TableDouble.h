//
// Created by ifkbhit on 21.05.19.
//

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>

class TableDouble {
private:

    std::string name;
    std::string leftName;
    std::string rightName;
    size_t leftSize;
    size_t rightSize;
    std::vector<std::pair<std::string, std::string>> values;
    std::stringstream out;


    void fillValue(const std::string& value, const size_t& len);

    void fillChar(const size_t& n, char c);

    void line(size_t len);

    void fillRow(const std::string& left, const std::string& right);

public:

    TableDouble(std::string name, std::string leftName, std::string rightName, size_t leftSize = 0,
                size_t rightSize = 0);


    template<class U, class V>
    void addValue(const U& left, const V& right, const std::string& markLeft = "", const std::string& markRight = "") {
        std::stringstream l;
        std::stringstream r;
        l << markLeft << left;
        r << markRight << right;
        auto lStr = l.str();
        auto rStr = r.str();
        leftSize = std::max(leftSize, 2 + lStr.size());
        rightSize = std::max(rightSize, 2 + rStr.size());
        values.emplace_back(lStr, rStr);
    }

    void print(std::ostream& output);
};

