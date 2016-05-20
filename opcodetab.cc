/* Steven Grimes, Sho Miyauchi, Karah Hui, Christian Valdez
   masc
   Team Michigan
   prog2
   CS530, Spring 2016
*/

#include <iostream>
#include <vector>
#include <map>
#include "opcodetab.h"
#include "opcode_error_exception.h"

using namespace std;

opcodetab::opcodetab() {
    const int NUM_OF_OPCODES = 59;
    string mnemonics[NUM_OF_OPCODES] = {"ADD", "ADDF", "ADDR", "AND",
                                        "CLEAR", "COMP", "COMPF", "COMPR", "DIV", "DIVF", "DIVR", "FIX",
                                        "FLOAT", "HIO", "J", "JEQ", "JGT", "JLT", "JSUB", "LDA", "LDB",
                                        "LDCH", "LDF", "LDL", "LDS", "LDT", "LDX", "LPS", "MUL", "MULF",
                                        "MULR", "NORM", "OR", "RD", "RMO", "RSUB", "SHIFTL", "SHIFTR",
                                        "SIO", "SSK", "STA", "STB", "STCH", "STF", "STI", "STL", "STS",
                                        "STSW", "STT", "STX", "SUB", "SUBF","SUBR", "SVC","TD","TIO","TIX",
                                        "TIXR","WD"};

    string machineCodes[NUM_OF_OPCODES] = {"18", "58", "90", "40", "B4",
                                           "28", "88", "A0", "24", "64", "9C", "C4", "C0", "F4", "3C", "30", "34",
                                           "38", "48", "00", "68", "50", "70", "08", "6C", "74", "04", "D0", "20",
                                           "60", "98", "C8", "44", "D8", "AC", "4C", "A4", "A8", "F0", "EC", "0C",
                                           "78", "54", "80", "D4", "14", "7C", "E8", "84", "10", "1C", "5C", "94",
                                           "B0", "E0", "F8", "2C", "B8", "DC"};

    int sizes[NUM_OF_OPCODES] = {3, 3, 2, 3, 2, 3, 3, 2, 3, 3, 2, 1, 1, 1, 3, 3, 3, 3, 3,
                                 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 3, 3, 2, 3, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                 3, 3, 2, 2, 3, 1, 3, 2, 3};

    for (int i = 0; i < NUM_OF_OPCODES; i++) {
        ops newOps;
        newOps.machineCode = machineCodes[i];
        newOps.size = sizes[i];
        m.insert(pair<string, ops>(mnemonics[i], newOps));

    }
}

string opcodetab::get_machine_code(string opcode) {
    if(opcode.empty())
        throw opcode_error_exception("empty opcode, can't get machine code.");
    if(opcode[0] == '+' && opcode.size() != 1){
        string newOpcode = opcode.substr(1);
        newOpcode = convert(newOpcode);
        if(!opcode_exists(newOpcode))
            throw opcode_error_exception("opcode " + opcode + " does not exist");
        if(m.find(newOpcode)-> second.size == 3)
            return m.find(newOpcode) -> second.machineCode;
        else
            throw opcode_error_exception("opcode " + opcode + " is set to invalid format 4.");
    }
    string newOpcode = convert(opcode);
    if (!opcode_exists(newOpcode))
        throw opcode_error_exception("opcode " + opcode + " does not exist.");
    return m.find(newOpcode)->second.machineCode;
}

int opcodetab::get_instruction_size(string opcode) {
    if(opcode.empty())
        throw opcode_error_exception("empty opcode, can't get size.");
    if(opcode[0] == '+' && opcode.size() != 1){
        string newOpcode = opcode.substr(1);
        newOpcode = convert(newOpcode);
        if(!opcode_exists(newOpcode))
            throw opcode_error_exception("opcode does not exist.");
        if(m.find(newOpcode)-> second.size == 3)
            return 4;
        else
            throw opcode_error_exception("opcode " + opcode + "is invalid, cannot be format 4.");
    }
    string newOpcode = convert(opcode);
    if (!opcode_exists(newOpcode))
        throw opcode_error_exception("getting instruction size of " + opcode);
    return m.find(newOpcode)->second.size;
}

bool opcodetab::opcode_exists(string opcode) {
    if (m.find(opcode) == m.end())
        return false;
    return true;
}

string opcodetab::convert(string s) {
    for (unsigned int i = 0; i < s.length(); i++) {
        s[i] = toupper(s[i]);
    }
    return s;
}




