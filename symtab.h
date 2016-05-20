/* Karah Hui, Sho Miyauchi, Christian Valdez, Steven Grimes
 * Team Michigan
 * sicxe_asm.h
 * masc0869
 * prog4
 * CS530, Spring 2016
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <iostream>
#include <map>
#include "symtab_exception.h"

using namespace std;


class symtab {
 public:
    // ctor
    // creates a new dictionary structure and loads all the user defined
    // symbols in the program into the table. Will use a map for this.
    symtab();

    //Inserts the symbol and value into the symtab
    void insert_symbol(string,int,string);

    //Looks for a symbol in the symtab just to make sure there are
    //no duplicate symbols added.
    bool contains_symbol(string);

    // takes a user defined symbol and returns the corresponding value
    // (address or value assigned when using "EQU")
    // throws an exception if the user defined symbol is not found in
    // the table
    int get_value(string);

    string get_status(string);

 private:
    // your variables and private methods go here

    struct value {
        int address;
        string status; //Can only be relative or absolute
    };

    map<string,value> symbol_table;
    map<string,value>::iterator iter; //iterator for symtab
};


#endif
