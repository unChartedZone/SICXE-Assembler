/* Karah Hui, Sho Miyauchi, Christian Valdez, Steven Grimes
 * Team Michigan
 * sicxe_asm.h
 * masc0869
 * prog4
 * CS530, Spring 2016
 */

#include <iostream>
#include <map>
#include "symtab.h"
#include "symtab_exception.h"

using namespace std;

symtab::symtab() {
    string registers[9] = {"A","X","L","B","S","T","F","PC","SW"};
    int reg_num[9] = {0,1,2,3,4,5,6,8,9};
    value newValue;
    for(int i = 0; i < 9; i++){
        newValue.address = reg_num[i];
        symbol_table[registers[i]] = newValue;
    }
 }

void symtab::insert_symbol(string symbol, int address,string status) {
    value newValue;
    newValue.address = address;
    newValue.status = status;
    symbol_table[symbol] = newValue;
}

bool symtab::contains_symbol(string symbol) {
    return symbol_table.find(symbol) != symbol_table.end();
}

int symtab::get_value(string symbol) {
    iter = symbol_table.find(symbol);
    if(iter != symbol_table.end())
        return iter->second.address;
    return -1;
}

string symtab::get_status(string symbol) {
    iter = symbol_table.find(symbol);
    if(iter != symbol_table.end())
        return iter->second.status;
    return "";
}
