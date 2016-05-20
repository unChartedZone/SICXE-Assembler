/* Sho Miyauchi, Karah Hui, Steven Grimes, Christian Valdez
 * Team Michigan
 * file_parser.cc
 * masc0869
 * prog1
 * CS530, Spring 2016
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>
#include "file_parser.h"
#include "file_parse_exception.h"

using namespace std;

file_parser::file_parser(string file) {
    file_name = file;
    line_number = 0;
}

file_parser::~file_parser() {
}



void file_parser::read_file() {
    ifstream infile(file_name.c_str());
    string line;
    vector <string> contents;
    if(!infile)
        throw file_parse_exception("Error opening file");
    while(!infile.eof()) {
        getline(infile,line);
        contents.push_back(line);
    }
    infile.close();
    parse_file(contents);
}

string file_parser::get_token(unsigned int row, unsigned int column) {
    if(row >= size())
        throw file_parse_exception("Invalid parameters, row is out of bounds");

    Line tmpLine = lines[row];
    if(column == 0)
        return tmpLine.label;

    else if(column == 1)
        return tmpLine.opcode;

    else if(column == 2)
        return tmpLine.operand;

    else if(column == 3)
        return tmpLine.comment;

    else
        print_error("Invalid parameters, column is out of bounds, at line ", (row+1));
}

void file_parser::print_file() {
    //Formatting Constant
    int WIDTHSPACE = 13;

    //Print column headers
    cout << file_name << endl;
    cout.setf(ios::left);
    cout << setw(WIDTHSPACE) << "Label "
        << setw(WIDTHSPACE) << "Opcode "
        << setw(WIDTHSPACE) << "Operand "
        << "Comments " << endl;

    //Print assembly line by line, separated by token type
    for(unsigned int i = 0; i < (unsigned)size(); i++) {
        cout.setf(ios::left);
        cout << setw(WIDTHSPACE) << lines[i].label
            << setw(WIDTHSPACE) << lines[i].opcode
            << setw(WIDTHSPACE) << lines[i].operand
            << lines[i].comment << endl;
    }
}

int file_parser::size() {
    return lines.size();
}

void file_parser::parse_file(vector<string> contents) {
    string str;
    vector<string> tokens;

    for(unsigned int i = 0; i < contents.size(); i++) {
        line_number++;
        string cLine = contents[i];
        int periodPosition;

        Line newLine;
        newLine.label = "";
        newLine.opcode = "";
        newLine.operand = "";
        newLine.comment = "";

        //separate (delimit) by spaces and tabs
        string delimiters = " \t";

        if(cLine[0] == '.') {
            newLine.comment = cLine;
            lines.push_back(newLine);
            continue;
        }

        int end_quote = period_in_quote(cLine);
        if(end_quote != -1) {
            periodPosition = cLine.find('.', end_quote);
        }
        else
            periodPosition = cLine.find('.');

        if(periodPosition != -1 && periodPosition != 0 && (cLine[periodPosition - 1] != ' '))
            if(cLine[periodPosition - 1] != '\t')
                print_error("Invalid line, needs at least one whitespace between tokens, at line ", line_number);

        //Find comments
        //Had to remove unsigned to fix a bug
        if(periodPosition != string::npos) {
            newLine.comment = cLine.substr(periodPosition);
            cLine.erase(periodPosition);
        }

        unsigned int last = cLine.find_first_not_of(delimiters,0);
        unsigned int first = cLine.find_first_of(delimiters,last);
        string token = " ";
        while(first != -1 || last != -1) {
            token = cLine.substr(last,first-last);
            tokens.push_back(token);
            last = cLine.find_first_not_of(delimiters,first);
            first = cLine.find_first_of(delimiters,last);
        }
        if(tokens.empty()) {
            lines.push_back(newLine);
            continue;
        }
        //Check label at column 0
        if(cLine[0] != ' ' && cLine[0] != '\t') {
            if(!isalpha(tokens[0].at(0)))
                print_error("Invalid label, first character must be a letter, at line ", line_number);
            string str = tokens[0];
            for(int i = 0; i < str.length(); i++)
                if(!isalnum(str[i]))
                    print_error("Invalid label, no non-letters/digits at line ", line_number);

            newLine.label = tokens[0];
            tokens.erase(tokens.begin());
        }

        if(tokens.empty()) {
            lines.push_back(newLine);
            continue;
        }
        newLine.opcode = tokens[0];
        if(!newLine.opcode.empty() && newLine.opcode[0] == '.'){
            newLine.comment = tokens[0];
            newLine.opcode = "";
        }
        tokens.erase(tokens.begin());
        if(tokens.empty()) {
            lines.push_back(newLine);
            continue;
        }

        string quote;
        string operand;

        //no quotes, but token size > 1
        //checks comments without periods, throws exception
        if(cLine.find('\'') == string::npos && tokens.size() > 1)
            print_error("Invalid line at line ", line_number);

        if(tokens[0].find('\'') != string::npos){
            unsigned int front = cLine.find('\'');
            unsigned int end = cLine.find_last_of('\'');
            if(end == front){
                print_error("Invalid operand, missing quotes, at line ", line_number);
            }
            quote = cLine.substr(front,(end - front) + 1);
            int aposPos = tokens[0].find('\'');
            operand = tokens[0].substr(0,aposPos);
            newLine.operand = operand + quote;
            if(!newLine.operand.empty() && newLine.operand[0] == '.'){
                newLine.comment = tokens[0];
                newLine.operand = "";
            }
        }

        else if(tokens.size() == 1) {
            newLine.operand = tokens[0];
            if(!newLine.operand.empty() && newLine.operand[0] == '.'){
                newLine.comment = tokens[0];
                newLine.operand = "";
            }
        }
        lines.push_back(newLine);
        tokens.clear(); //To reset vector tokens
    }
}

int file_parser::period_in_quote(string quote) {
    unsigned int front = quote.find('\'');
    unsigned int end = quote.find_last_of('\'');

    //viable quotes
    if(front != end) {
        if(quote.find('.') != -1)	//do find a period
            return end;
    }
    return -1;				//no period no quotes
}

void file_parser::print_error(string s, unsigned int line_number) {
    stringstream ss;
    ss << line_number;
    string linestr = ss.str();
    throw file_parse_exception(s + linestr + '.');
}