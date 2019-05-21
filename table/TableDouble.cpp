#include <utility>


#include "TableDouble.h"

TableDouble::TableDouble(std::__cxx11::basic_string<char> name, std::__cxx11::basic_string<char> leftName,
                         std::__cxx11::basic_string<char> rightName, unsigned long leftSize,
                         unsigned long rightSize) : name(std::move(name)), leftName(std::move(leftName)),
                                                    rightName(std::move(rightName)),
                                                    leftSize(leftSize), rightSize(rightSize) {
    this->leftSize = std::max(this->leftName.size() + 2, leftSize);
    this->rightSize = std::max(this->rightName.size() + 2, rightSize);
}


void TableDouble::fillChar(const size_t& n, const char c) {
    for (int i = 0; i < n; ++i) {
        out << c;
    }
}

void TableDouble::line(size_t len) {
    out << '+';
    fillChar(len, '-');
    out << "+\n";

}

void TableDouble::fillValue(const std::string& value, const size_t& len) {
    auto spaces = (len - value.size()) / 2;
    fillChar(spaces, ' ');
    out << value;
    fillChar(len - spaces - value.size(), ' ');
}

void TableDouble::print(std::ostream& output) {
    out = std::stringstream();
    auto resLen = leftSize + rightSize + 1;
    line(resLen);
    out << '|';
    fillValue(name, resLen);
    out << "|\n";

    line(resLen);
    fillRow(leftName, rightName);
    line(resLen);

    for (const auto& lr: values) {
        fillRow(lr.first, lr.second);
    }
    line(resLen);
    output << out.str();
}

void TableDouble::fillRow(const std::string& left, const std::string& right) {
    out << '|';
    fillValue(left, leftSize);
    out << '|';
    fillValue(right, rightSize);
    out << "|\n";
}


