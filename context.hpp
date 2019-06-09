#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <sys/ucontext.h>

#include <map>
#include <string>

const std::map<int, std::string> regs = {
    {REG_R8, "R8"},
    {REG_R9, "R9"},
    {REG_R10, "R10"},
    {REG_R11, "R11"},
    {REG_R12, "R12"},
    {REG_R13, "R13"},
    {REG_R14, "R14"},
    {REG_R15, "R15"},
    {REG_RDI, "RDI"},
    {REG_RSI, "RSI"},
    {REG_RBP, "RBP"},
    {REG_RBX, "RBX"},
    {REG_RDX, "RDX"},
    {REG_RAX, "RAX"},
    {REG_RCX, "RCX"},
    {REG_RSP, "RSP"},
    {REG_RIP, "RIP"},
    {REG_EFL, "EFL"},
    {REG_CSGSFS, "CSGSFS"},
    {REG_ERR, "ERR"},
    {REG_TRAPNO, "TRAPNO"},
    {REG_OLDMASK, "OLDMASK"},
    {REG_CR2, "CR2"},
};

#endif // CONTEXT_HPP_
