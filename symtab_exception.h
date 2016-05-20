/* Steven Grimes, Sho Miyauchi, Karah Hui, Christian Valdez
   masc
   Team Michigan
   prog3
   CS530, Spring 2016
*/

#ifndef SYMTAB_EXCEPTION_H
#define SYMTAB_EXCEPTION_H
#include <string>

using namespace std;

class symtab_exception {

public:
    symtab_exception(string s) {
        message = s;
    }

    symtab_exception() {
        message = "An error has occurred";
    }

    string getMessage() {
        return message;
    }

private:
    string message;
};
#endif