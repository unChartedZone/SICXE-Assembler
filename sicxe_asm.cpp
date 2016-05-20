/* Karah Hui, Sho Miyauchi, Christian Valdez, Steven Grimes
 * Team Michigan
 * sicxe_asm.cpp
 * masc0869
 * prog4
 * CS530, Spring 2016
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "file_parser.h"
#include "file_parse_exception.h"
#include "opcodetab.h"
#include "opcode_error_exception.h"
#include "symtab.h"
#include "symtab_exception.h"
#include "sicxe_asm.h"

using namespace std;

int main(int argc, char *argv[]) {
    if(argc != 2) {
        cout << "Error, you must supply the name of the file to process "
            << "at the command line." << endl;
        //exit(1);
    }
    string filename = argv[1];
    try {
        sicxe_asm assembler(filename);
        assembler.first_pass();
        assembler.second_pass();
        assembler.make_output_file(filename);
        assembler.make_executable_file(filename);
        return 0;
    }
    catch(file_parse_exception excpt) {
        cout << "Sorry, error. " << excpt.getMessage() << endl;
    }
    catch(opcode_error_exception excpt) {
        cout << "Sorry, error. " << excpt.getMessage() << endl;
    }
    catch(symtab_exception excpt) {
        cout << "Sorry, error. " << excpt.getMessage() << endl;
    }
}

sicxe_asm::sicxe_asm(string file_name):parser(file_name) {
    LOCCTR = 0x00000;
    PC = 0x00000;
    starting_address = 0x00000; //Just in case
    length_of_program = 0x0000; //Just in case
    start_found = false;
    end_found = false;
    first_pass_completed = false;
    base_found = false;
    addresses.clear();
    object_codes.clear();
}

sicxe_asm::~sicxe_asm() {
}

void sicxe_asm::first_pass() {
    parser.read_file();

    //assigning addresses
    int i = 0;

    while(!end_found) {
        string label = parser.get_token(i,0);
        string opcode = parser.get_token(i,1);
        string operand = parser.get_token(i,2);

        string temp = capitalize(opcode);	//used to check for EQU directive
        if(label != "" && temp != "EQU") {
            if(symbols.contains_symbol(label)) {
                message = "Duplicate symbol " + label + " on line: ";
                print_error('s',message, i+1);
            }
            else {
                symbols.insert_symbol(label, LOCCTR,"relative");
            }
        }
        if(!start_found && label != "" && temp != "START") {
            if(label != "" && opcode != "") {
                message = "Line of code detected before START directive on line ";
                print_error('f',message,i+1);
            }
            message = "Label " + label + " detected before START directive on line ";
            print_error('f',message,i+1);
        }
        if(opcode == "") {
            insert_into_address();
            i++;
            continue;
        }
        if(run_directs(label, opcode, operand, i+1)) {
            i++;
            continue;
        }
        if(!start_found) {
            message = "Opcode detected before START directive on line ";
            print_error('f', message, i+1);
        }
        insert_into_address();
        int offset = table.get_instruction_size(opcode);
        LOCCTR += offset;
        i++;
    }
    while(addresses.size() < parser.size()) {
        int num_line = addresses.size();
        string check_label = parser.get_token(num_line,0);
        string check_opcode = parser.get_token(num_line,1);
        string check_operand = parser.get_token(num_line,2);
        if(check_label != "") {
            if(check_label != "" && check_opcode != "") {
                message = "Line of code detected after END directive on line ";
                print_error('f',message,i+1);
            }
            message = "Label " + check_label + " detected after END directive on line ";
            print_error('f',message,num_line + 1);
        }
        if(check_opcode != "") {
            message = "Opcode " + check_opcode + " detected after END directive on line ";
            print_error('f',message,num_line + 1);
        }
        if(check_operand != "") {
            message = "Operand " + check_operand + " detected after END directive on line ";
            print_error('f',message,num_line + 1);
        }
        insert_into_address();
    }
    first_pass_completed = true;
}

void sicxe_asm::insert_into_address() {
    stringstream stream;
    stream << setfill('0') << setw(5) << hex << LOCCTR;
    string number = stream.str();
    number = capitalize(number);
    addresses.push_back(number);
}

void sicxe_asm::second_pass() {
    for(unsigned int i = 0; i < addresses.size(); i++) {
        PC = string_to_int("$" + addresses[i]);
        string label = parser.get_token(i,0);
        string opcode = parser.get_token(i,1);
        string operand = parser.get_token(i,2);

        if(label.empty() && opcode.empty()) {
            object_codes.push_back("");
            continue;
        }
		if(opcode.empty()) {
			object_codes.push_back("");
			continue;
		}
        if(run_directs(label,opcode,operand,i)) {
            continue;
        }

        int format = table.get_instruction_size(opcode);
        string machine_code = table.get_machine_code(opcode);
        //Check what format the opcode is
        if(format == 1) {
            format_one(opcode,operand,i+1);
            continue;
        }
        if(format == 2) {
            format_two(opcode,operand,i+1);
            continue;
        }
        if(format >= 3) {
            PC = string_to_int("$" + addresses[i+1]);
            format_three_and_four(i+1,format,machine_code,operand);
            continue;
        }
    }
}

bool sicxe_asm::run_directs(string label,string directive,string operand,int line_num) {
    directive = capitalize(directive);
    if(directive == "START") {
        if(first_pass_completed) {
            object_codes.push_back("");
            return true;
        }
        program_name = label;
        start_directive(operand, line_num);
        return true;
    }
    if(directive == "END") {
        if(first_pass_completed) {
            object_codes.push_back("");
            return true;
        }
        if(!start_found) {
            message = "END of program detected without starting address "
                "ever declared on line ";
            print_error('f',message,line_num);
        }
        end_directive(operand);
        return true;
    }
    if(directive == "BYTE") {
        if(first_pass_completed) {
            generate_byte(line_num,operand);
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        byte_directive(operand, line_num);
        return true;
    }
    if(directive == "WORD") {
        if(first_pass_completed) {
            generate_word(line_num, operand);
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        word_directive(operand, line_num);
        return true;
    }
    if(directive == "RESW") {
        if(first_pass_completed) {
            object_codes.push_back("");
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        resw_directive(operand, line_num);
        return true;
    }
    if(directive == "RESB") {
        if(first_pass_completed) {
            object_codes.push_back("");
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        resb_directive(operand, line_num);
        return true;
    }
    if(directive == "EQU") {
        if(first_pass_completed) {
            object_codes.push_back("");
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        equ_directive(label,operand,line_num);
        return true;
    }
    if(directive == "BASE") {
        if(first_pass_completed) {
            object_codes.push_back("");
            base_found = true;
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        base_directive(operand,line_num);
        return true;
    }
    if(directive == "NOBASE") {
        if(first_pass_completed) {
            object_codes.push_back("");
            base_found = false;
            return true;
        }
        if(!start_found) {
            message = "Directive " + directive + " came before START on line ";
            print_error('f',message,line_num);
        }
        nobase_directive(line_num);
        return true;
    }
    else {
        return false;
    }
}

void sicxe_asm::start_directive(string operand,int line_num) {
    start_found = true;
    if(operand.empty()) {
        message = "Empty operand in START directive on line ";
        print_error('f',message,line_num);
    }
    if(operand[0] != '$') {
        if(operand.find_first_not_of("0123456789") != -1) {
            message = "Invalid operand in START directive on line ";
            print_error('f',message,line_num);
        }
    }
    //if true, then the operand is in hex format
    if(operand[0] == '$') {
        if(operand.find_first_not_of("0123456789abcdefABCDEF",1) != -1) {
            message = "Invalid operand in START directive on line ";
            print_error('f',message,line_num);
        }
        //operand = operand.erase(0,1);
        int offset = string_to_int(operand);
        starting_address = offset;
        insert_into_address();
        LOCCTR += offset;
        return;
    }
    insert_into_address();
    int offset = string_to_int(operand);
    starting_address = offset;
    LOCCTR += offset;
    return;
}

void sicxe_asm::end_directive(string operand) {
    end_found = true;
    while(!cascading_references.empty()){
        reference current_ref;
        current_ref = cascading_references.top();
        cascading_references.pop();
        if(symbols.contains_symbol(current_ref.operand)){
            symbols.insert_symbol(current_ref.label,symbols.get_value(current_ref.operand),symbols.get_status(current_ref.operand));
        }
        else{
            message = "Invalid operand on line ";
            print_error('o',message,current_ref.line_num);
        }
    }
    insert_into_address();
}

void sicxe_asm::byte_directive(string operand,int line_num) {
    if(operand.empty()) {
        message = "Empty operand " + operand + "of BYTE directive on line ";
        print_error('f',message,line_num);
    }
    string newOperand = capitalize(operand);
    if(newOperand[0] == 'C') {
        int firstApos = newOperand.find_first_of('\'');
        if(firstApos != 1) {
            message = "Invalid operand " + operand + " of BYTE directive on line ";
            print_error('f',message,line_num);
        }
        int length = ((newOperand.find_last_of('\'') - firstApos) - 1);
        insert_into_address();
        LOCCTR += length;
    }
    else if(newOperand[0] == 'X') {
        int firstApos = newOperand.find_first_of('\'');
        if(firstApos != 1) {
            message = "Invalid operand " + operand + " of BYTE directive on line ";
            print_error('f',message,line_num);
        }
        int length = ((newOperand.find_last_of('\'') - firstApos) - 1);
        string quote = newOperand.substr(newOperand.find_first_of('\'')+1,length);
        int check = quote.find_first_not_of("0123456789abcdefABCDEF");
        if(check != -1) {
            message = "Invalid operand " + operand + "of BYTE directive on line ";
            print_error('f',message,line_num);
        }
        if(length % 2 != 0) {
            message = "Invalid operand length " + operand + "of BYTE directive on line ";
            print_error('f',message,line_num);
        }
        insert_into_address();
        LOCCTR += length/2;
    }
    else {
        message = "Invalid operand " + operand + "of BYTE directive on line ";
        print_error('f',message,line_num);
    }
}

void sicxe_asm::word_directive(string operand,int line_num) {
    int pound_sign = operand.find('#');
    if(operand.empty()) {
        message = "No operand detected with WORD directive on line ";
        print_error('f',message,line_num);
    }
    if(pound_sign != -1) {
        message = "# found in operand field with WORD directive on line ";
        print_error('f',message,line_num);
    }
    insert_into_address();
    LOCCTR += 3;
}

void sicxe_asm::resw_directive(string operand, int line_num) {
    if(operand.empty()) {
        message = "Empty operand of RESW directive on line ";
        print_error('f',message,line_num);
    }
    if(operand[0] == '#') {
        message = "# detected in " + operand + " of RESW directive on line ";
        print_error('f',message,line_num);
    }
    insert_into_address();
    int offset = string_to_int(operand);
    offset *= 3;
    LOCCTR += offset;
}

void sicxe_asm::resb_directive(string operand,int line_num) {
    if(operand.empty()) {
        message = "Empty operand of RESB directive on line ";
        print_error('f',message,line_num);
    }
    if(operand[0] == '#') {
        message = "# detected in " + operand + " of RESB directive on line ";
        print_error('f',message,line_num);
    }
    insert_into_address();
    int offset;
    if(operand[0] == '$')
        offset = string_to_int(operand);
    else

    if(operand.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")!= 0) {
        message = "Character detected, hex not specified with operand on line ";
        print_error('f',message,line_num);
    }
    else {
        offset = string_to_int(operand);
    }
    LOCCTR += offset;
}

void sicxe_asm::equ_directive(string label,string operand,int line_num) {
    if(operand.empty()) {
        message = "Empty operand in EQU directive on line ";
        print_error('f',message,line_num);
    }
    if(symbols.contains_symbol(label)) {
        message = "Duplicate label " + label + "of EQU directive on line ";
        print_error('f',message,line_num);
    }
    if(label.empty()) {
        message = "Need label for EQU directive on line ";
        print_error('s',message,line_num);
    }
    if(operand[0] == '#') {
        message = "# detected in the EQU directive on line ";
        print_error('f',message, line_num);
    }
    if(symbols.contains_symbol(operand)) {
        symbols.insert_symbol(label, symbols.get_value(operand),"relative");
        insert_into_address();
    }
    else if(operand[0] == '$') {
        int address = string_to_int(operand);
        symbols.insert_symbol(label, address,"absolute");
        insert_into_address();
    }
    else if(isalpha(operand[0]) != 0) {
        reference newref;
        newref.label = label;
        newref.operand = operand;
        newref.line_num = line_num;
        cascading_references.push(newref);
        insert_into_address();
    }
    else if(operand.find_first_not_of("0123456789") == -1) {
        int address = string_to_int(operand);
        symbols.insert_symbol(label, address,"absolute");
        insert_into_address();
    }
    else {
        message = "Unknown operand: " + operand + " of EQU directive on line ";
        print_error('f',message,line_num);
    }
}

void sicxe_asm::base_directive(string operand,int line_num) {
    if(operand.empty()) {
        message = "Empty operand in BASE directive on line ";
        print_error('f',message,line_num);
    }
    if(operand.find('#') != -1) {
        message = "# detected in " + operand + "of BASE directive on line ";
        print_error('f',message,line_num + 1);
    }
    insert_into_address();
    //BASE = operand;
    //base_found = true;
}

void sicxe_asm::nobase_directive(int line_num) {
    insert_into_address();
    base_found = false;
}

/**************Second Pass Methods****************/
/*************************************************/

void sicxe_asm::format_one(string opcode, string operand,int line_num) {
    if(!operand.empty()) {
        message = "Error operand " + operand + " detected with format 1 on line ";
        print_error('f',message,line_num);
    }
    string temp = table.get_machine_code(opcode);
    object_codes.push_back(temp);
}

void sicxe_asm::format_two(string opcode, string operand,int line_num) {
    if(operand.empty()){
        message = "Empty operand on line ";
        print_error('o',message,line_num);
    }
    int instruction = 0;
    int code = convert_machine_code(table.get_machine_code(opcode));
    if(code == 0xB4 || code == 0xA4 || code == 0xA8 || code == 0xB0 || code == 0xB8 ){
        outliers(code,operand,line_num);
        return;
    }
    if(operand.find(',') == -1){
        message = "Invalid number of registers on line ";
        print_error('o',message,line_num);
    }
    instruction = code << 8;
    string register1 = operand.substr(0,operand.find(','));
    string register2 = operand.substr(operand.find(',')+1);
    register1 = capitalize(register1);
    register2 = capitalize(register2);
    validate_register(register1,line_num);
    validate_register(register2,line_num);
    int reg1 = symbols.get_value(register1);
    int reg2 = symbols.get_value(register2);
    instruction |= reg1 << 4;
    instruction |= reg2;
    string mcode = format_hex_number(instruction,4);
    object_codes.push_back(mcode);
}

void sicxe_asm::format_three_and_four(int line_num,int size,string machine_code,string operand) {
    if(machine_code != "4C" && operand.empty()){
        message = "Empty operand on line ";
        print_error('o',message,line_num);
    }
    int immediate = operand.find('#');
    int indirect = operand.find('@');
    int indexed = operand.find(',');
    if(immediate != -1 && indirect != -1) {
        message = "$ & @ cannot be both used in operand " + operand + "on line ";
        print_error('f',message,line_num);
    }
    if(immediate == 0) {
        immediate_addressing(line_num,size,machine_code,operand);
        return;
    }
    if(indirect == 0) {
        indirect_addressing(line_num,size,machine_code,operand);
        return;
    }
    if(indexed != -1) {
        indexed_addressing(line_num,size,indexed,machine_code,operand);
        return;
    }
    //Operand has not indicated immediate,indirect,or indexed
    direct_addressing(line_num, size, machine_code, operand);
}

void sicxe_asm::immediate_addressing(int line_num,int size, string machine_code, string operand) {
    string temp = operand.substr(1);
    int instruction  = convert_machine_code(machine_code);
    instruction = instruction << 16 | ibit;
    int TA = symbols.get_value(temp);
    string mcode = "";
    if(TA == -1) {
        is_valid(line_num,temp,true,size);
        int num = string_to_int(temp);
        if(size == 4) {
            instruction |= ebit;
            instruction = instruction << 8;
            instruction |= num;
            mcode = format_hex_number(instruction,8);
        }
        else{
            instruction |= num;
            mcode = format_hex_number(instruction,6);
        }
        object_codes.push_back(mcode);
		if(machine_code == "68") {
			BASE = temp;
			check_offset(line_num,TA,instruction);
		}
        return;
    }
    if(size == 4) {
        instruction |= ebit;
        instruction = instruction << 8 | TA;
        create_mod_data(line_num, "05",temp);
        mcode = format_hex_number(instruction,8);
    }
    else {
        instruction = check_offset(line_num,TA,instruction);
        mcode = format_hex_number(instruction,6);
    }
    object_codes.push_back(mcode);
    if(machine_code == "68"){
        BASE = temp;
    }
}

void sicxe_asm::indirect_addressing(int line_num,int size,string machine_code, string operand) {
    string mcode = "";
    string temp = operand.substr(1);
    int TA = symbols.get_value(temp);
    int instruction = convert_machine_code(machine_code);
    instruction = instruction << 16 | nbit;
    if(TA == -1) {
        is_valid(line_num,temp,false,size);
        int offset = string_to_int(temp);
        if(size == 4) {
            instruction = (instruction | ebit) << 8 | offset;
            mcode = format_hex_number(instruction,8);
        }
        else {
            instruction |= offset;
            mcode = format_hex_number(instruction,6);
        }
        object_codes.push_back(mcode);
        if(machine_code == "68") {
            BASE = temp;
            check_offset(line_num,TA,instruction);
        }
        return;
    }
    if(size == 4) {
        instruction = (instruction | ebit) << 8 | TA;
        create_mod_data(line_num,"05", temp);
        mcode = format_hex_number(instruction,8);
    }
    else {
        instruction = check_offset(line_num,TA,instruction);
        mcode = format_hex_number(instruction,6);
    }
    object_codes.push_back(mcode);
    if(machine_code == "68") {
        BASE = temp;
        check_offset(line_num,TA,instruction);
    }
}

void sicxe_asm::indexed_addressing(int line_num,int size,int comma_loc,string machine_code, string operand) {
    string temp = operand.substr(0,comma_loc);
    string x = operand.substr(comma_loc+1);
    if(!x.compare("x") && !x.compare("X")) {
        message = "Index flag missing from operand " + operand + " on line ";
        print_error('f',message,line_num);
    }
    int TA = symbols.get_value(temp);
    int instruction = convert_machine_code(machine_code);
    instruction = instruction << 16 | nbit | ibit | xbit;
    string mcode = "";
    if(TA == -1) {
        is_valid(line_num,temp,false,size);
        int offset = string_to_int(temp);
        if(size == 4) {
            instruction = (instruction | ebit) << 8 | offset;
            mcode = format_hex_number(instruction,8);
        }
        else {
            instruction |= offset;
            mcode = format_hex_number(instruction,6);
        }
        object_codes.push_back(mcode);
        if(machine_code == "68") {
            BASE = temp;
            check_offset(line_num,TA,instruction);
        }
        return;
    }
    if(size == 4) {
        instruction = (instruction | ebit) << 8 | TA;
        create_mod_data(line_num,"05",temp);
        mcode = format_hex_number(instruction,8);
    }
    else {
        instruction = check_offset(line_num,TA,instruction);
        mcode = format_hex_number(instruction,6);
    }
    object_codes.push_back(mcode);
    if(machine_code == "68") {
        BASE = temp;
        check_offset(line_num,TA,instruction);
    }
}

void sicxe_asm::direct_addressing(int line_num, int size,string machine_code,string operand) {
    string mcode = "";
    int instruction = convert_machine_code(machine_code);
    instruction = instruction << 16 | nbit | ibit;
    int TA = symbols.get_value(operand);
    if(operand == "") {
        string mcode = format_hex_number(instruction,6);
        object_codes.push_back(mcode);
        if(machine_code == "68") {
            BASE = operand;
            check_offset(line_num,TA,instruction);
        }
        return;
    }
    if(TA == -1) {
        is_valid(line_num,operand,false,size);
        int offset = string_to_int(operand);
        if(size == 4) {
            instruction = (instruction | ebit) << 8 | offset;
            mcode = format_hex_number(instruction,8);
        }
        else {
            instruction |= offset;
            mcode = format_hex_number(instruction,6);
        }
        object_codes.push_back(mcode);
        if(machine_code == "68") {
            BASE = operand;
            check_offset(line_num,TA,instruction);
        }
        return;
    }
    if(size == 4){
        instruction = (instruction | ebit) << 8 | TA;
        create_mod_data(line_num,"05",operand);
        mcode = format_hex_number(instruction,8);
    }
    else{
        instruction = check_offset(line_num,TA,instruction);
        mcode = format_hex_number(instruction,6);
    }
    object_codes.push_back(mcode);
    if(machine_code == "68") {
        BASE = operand;
        check_offset(line_num,TA,instruction);
    }
}

void sicxe_asm::create_mod_data(int line_num, string size, string operand) {
    string check_status = symbols.get_status(operand);
    if(check_status == "relative") {
        modifiers mod;
        int modded_address = string_to_int("$" + addresses[line_num - 1]) + 1;
        string fixed_address = format_hex_number(modded_address, 5);
        mod.address = fixed_address;
        mod.size = size;
        mod_records.push(mod);
    }
}

void sicxe_asm::generate_byte(int line_num, string operand) {
    char indicator = toupper(operand[0]);
    int firstApos = operand.find_first_not_of('\'');
    int length = ((operand.find_last_of('\'') - firstApos) - 2);
    stringstream stream;
    int answer = 0;
    string characters = operand.substr(firstApos+2,length);
    if(indicator == 'C') {
        for (int i = 0; i < characters.length(); i++) {
            int temp = (int)characters[i];
            stream << hex << temp;
        }
        string mcode = capitalize(stream.str());
        object_codes.push_back(mcode);
        return;
    }
    //else must be X
    else {
        answer = string_to_int("$"+characters);
        string mcode = format_hex_number(answer,length);
        object_codes.push_back(mcode);
        return;
    }
}

void sicxe_asm::generate_word(int line_num, string operand) {
    int temp = 00;
    string check_status = symbols.get_status(operand);
    if(check_status != "") {
        int value = symbols.get_value(operand);
        temp = (temp << 16) | value;
        string mcode = format_hex_number(temp,6);
        object_codes.push_back(mcode);
        if(check_status == "relative")
            create_mod_data(line_num + 1,"06",operand);
        return;
    }
    is_valid(line_num,operand,false,3); //Given size 3 by default
    int num = string_to_int(operand);
    temp = (temp << 16) | num;
    string mcode = format_hex_number(temp,6);
    object_codes.push_back(mcode);
}

void sicxe_asm::outliers(int code, string operand, int line_num){
    string test = "OUTLIER";
    string mcode = "";
    string register1;
    string register2;
    int reg1;
    int reg2;
    int instruction = 0;
    //Checks if opcode is CLEAR
    if(code == 0xB4){
        if(operand.find(',') != -1){
            message = "Invalid number of registers on line ";
            print_error('o',message,line_num);
        }
        validate_register(operand,line_num);
        instruction = code << 8;
        register1 = capitalize(operand);
        reg1 = symbols.get_value(register1);
        instruction |= reg1 << 4;
        mcode = format_hex_number(instruction,4);
    }
    //Checks if opcode is SVC
    else if(code == 0xB0){
        if(operand.find(',') != -1){
            message = "Invalid number of registers on line ";
            print_error('o',message,line_num);
        }
        validate_number(operand,line_num);
        int n = string_to_int(operand);
        if(n > 15){
            message = "Operand register is out of bounds on line ";
            print_error('o',message,line_num);
        }
        instruction = code << 8;
        instruction |= n << 4;
        mcode = format_hex_number(instruction,4);
    }
    //Checks TIXR
    else if(code == 0xB8){
        if(operand.find(',') != -1){
            message = "Invalid number of registers on line ";
            print_error('o',message,line_num);
        }
        validate_register(operand,line_num);
        instruction = code << 8;
        register1 = capitalize(operand);
        reg1 = symbols.get_value(register1);
        instruction |= reg1 << 4;
        mcode = format_hex_number(instruction,4);
    }
	//Checks SHIFTL
    else if(code == 0xA4){
        register1 = operand.substr(0,operand.find(','));
        register2 = operand.substr(operand.find(',')+1);
        validate_register(register1,line_num);
        register1 = capitalize(register1);
        reg1 = symbols.get_value(register1);
        validate_number(register2,line_num);
        int n = string_to_int(register2);
        if(n > 16){
            message = "Operand register " + register2 +"  is out of bounds on line ";
            print_error('o',message,line_num);
        }
        instruction = code << 8;
        instruction |= reg1 << 4;
        instruction |= (n-1);
        mcode = format_hex_number(instruction,4);

    }
	//Checks SHIFTR
    else if(code == 0xA8){
        register1 = operand.substr(0,operand.find(','));
        register2 = operand.substr(operand.find(',')+1);
        validate_register(register1,line_num);
        register1 = capitalize(register1);
        reg1 = symbols.get_value(register1);
        validate_number(register2,line_num);
        int n = string_to_int(register2);
        if(n > 16){
            message = "Operand register " + register2 +"  is out of bounds on line ";
            print_error('o',message,line_num);
        }
        instruction = code << 8;
        instruction |= reg1 << 4;
        instruction |= (n-1);
        mcode = format_hex_number(instruction,4);

    }
    object_codes.push_back(mcode);

}

void sicxe_asm::validate_register(string reg, int line_num){
    string delimiters = "AXLBSTF";
    string temp;
    temp = capitalize(reg);
    if(temp.length() == 1 && temp.find_first_not_of(delimiters) != -1){
        message = "Invalid register " + reg + " on line ";
        print_error('o',message,line_num);
    }
    else if(temp.length() >= 2 && temp != "PC" && temp != "SW"){
        message = "Invalid register " + reg + " on line ";
        print_error('o',message,line_num);
    }
    return;

}


int sicxe_asm::convert_machine_code(string machine_code) {
    int mcode;
    stringstream ss;
    ss << hex << machine_code;
    ss >> mcode;
    return mcode;
}

string sicxe_asm::format_hex_number(int n, int digits){
    stringstream s;
    s << hex << setw(digits) << setfill('0') << n; //int digits can be either 2,4,6,8
    string toReturn = s.str();
    toReturn = capitalize(toReturn);
    return toReturn;
}

/*****************Output File Methods**************************/
/**************************************************************/

void sicxe_asm::make_output_file(string file_name) {
    //Searches for the period in an .asm file
    long period_position = file_name.find(".asm");
    if(period_position == -1)
        throw file_parse_exception("Invalid file type.");

    //Creates .lis file extension
    string lis_file = file_name;
    lis_file = lis_file.erase(period_position);
    string file_location = lis_file + ".lis";

    //Starts stream to .lis file
    ofstream output_file;

    //Creates the file with the .lis extension
    output_file.open(file_location.c_str());

    //Redirects every cout to the .lis file
    streambuf *oldbuf = cout.rdbuf();

    cout.rdbuf(output_file.rdbuf());
    write_file(file_name);

    cout.rdbuf(oldbuf);	//resets cout to act normally
    output_file.close();	//close stream
}

void sicxe_asm::write_file(string file_name) {
    //Formatting Constant
    int WIDTHSPACE = 13;

    //Print filename
    cout << "\t\t\t\t   **" << file_name << "**  " << endl;
    cout.setf(ios::left);

    //Print .lis file headers
    cout << setw(WIDTHSPACE) << "Line #"
        << setw(WIDTHSPACE) << "Address"
        << setw(WIDTHSPACE) << "Label"
        << setw(WIDTHSPACE) << "Opcode"
        << setw(WIDTHSPACE) << "Operand"
        << setw(WIDTHSPACE) << "Machine Code"
        << endl;

    cout << setw(WIDTHSPACE) << "======"
        << setw(WIDTHSPACE) << "======="
        << setw(WIDTHSPACE) << "====="
        << setw(WIDTHSPACE) << "======"
        << setw(WIDTHSPACE) << "======="
        << setw(WIDTHSPACE) << "============"
        << endl;

    //Print .lis file contents (line #, address, label, opcode, operand)
    for(unsigned int i = 0; i < (unsigned)addresses.size(); i++) {
        cout.setf(ios::left);
        cout << setw(WIDTHSPACE) << i+1
            << setw(WIDTHSPACE) << addresses[i]
            << setw(WIDTHSPACE) << parser.get_token(i,0)
            << setw(WIDTHSPACE) << parser.get_token(i,1)
            << setw(WIDTHSPACE) << parser.get_token(i,2)
            << setw(WIDTHSPACE) << object_codes[i]
            << endl;
    }
}

/********************Executable FIle Methods*************/
/********************************************************/

void sicxe_asm::make_executable_file(string file_name) {
    long period_position = file_name.find(".asm");
    string executable_file = file_name.erase(period_position);
    executable_file = executable_file + ".obj";

    ofstream exe_file;
    exe_file.open(executable_file.c_str()); //Open file
    //Write to file here
    header_record(exe_file);
    text_record(exe_file);
    modification_record(exe_file);
    end_record(exe_file);
    exe_file.close(); //Close file
}

void sicxe_asm::header_record(ofstream& exe_file) {
    length_of_program = convert_machine_code(addresses.back());
    length_of_program -= starting_address;
    string address = format_hex_number(starting_address,6);
    string length_address = format_hex_number(length_of_program,6);
    program_name = capitalize(pad_spaces(program_name));
    program_name = program_name.substr(0,6);
    string line = "H" + program_name + address + length_address;
    exe_file << line << endl;
}

void sicxe_asm::text_record(ofstream& exe_file) {
    string line = "T";
    int counter = 0;
    counter = set_counter(counter);

    while(counter < object_codes.size()) {
        bool overrun_detected = false;
        bool reserve_detected = false;
        line = "T";
        line = line + format_hex_number(convert_machine_code(addresses[counter]),6);
        while(line.size() < 67 && counter < object_codes.size()) {
            string current = object_codes[counter];
            if(current.empty()) {
                check_for_reserve(line,exe_file,counter,reserve_detected);
            }
            if(reserve_detected)
                break;
            line = line + current;
            if(line.size() > 67) {
                line.erase(line.size() - current.size());
                write_text_line(exe_file,line);
                overrun_detected = true;
                break;
            }
            counter++;
        }
        if(overrun_detected || reserve_detected)
            continue;
        write_text_line(exe_file,line);
    }
}

void sicxe_asm::check_for_reserve(string line, ofstream& exe_file, int &counter, bool &reserve_detected) {
    string check_opcode = parser.get_token(counter, 1);
    check_opcode = capitalize(check_opcode);
    if (check_opcode == "RESW" || check_opcode == "RESB") {
        reserve_detected = true;
        write_text_line(exe_file, line);
        counter++;
        counter = set_counter(counter);
    }
}

void sicxe_asm::modification_record(ofstream& exe_file) {
    string line = "M";
    while (!mod_records.empty()) {
        line = "M"; //Resets the line
        modifiers current_mod = mod_records.front();
        mod_records.pop();
        line = line + current_mod.address + current_mod.size;
        exe_file << line << endl;
    }
}

void sicxe_asm::end_record(ofstream& exe_file) {
    string line = "E" + format_hex_number(starting_address,6);
    exe_file << line << endl;
}

string sicxe_asm::pad_spaces(string s) {
    int length = s.length();
    if(s.length() < 6) {
        int needed_spaces = 6 - length;
        s.resize(length+needed_spaces,' ');
        return s;
    }
    return s;
}

int sicxe_asm::set_counter(int counter) {
    for(unsigned int i = counter; i < object_codes.size();i++) {
        if(!object_codes[i].empty()) {
            counter = i;
            break;
        }
    }
    return counter;
}

void sicxe_asm::write_text_line(ofstream &exe_file, string line) {
    string all_machine_codes = line.substr(7);
    string length = format_hex_number((all_machine_codes.length()/2),2);
    line.insert(7,length);
    exe_file << line << endl;
}

/********************Helper Methods**********************/
/********************************************************/

string sicxe_asm::capitalize(string s) {
    for(unsigned int i = 0; i < s.length(); i++) {
        s[i] = toupper(s[i]);
    }
    return s;
}

int sicxe_asm::string_to_int(string s) {
    bool hex_indicated = false;
    if(s[0] == '#' && s[1] == '$') {
        s = s.erase(0,2);
        hex_indicated = true;
    }
    else if(s[0] == '#' && s[1] != '$') {
        s = s.erase(0,1);
        hex_indicated = false;
    }
    else if(s[0] == '$') {
        s = s.erase(0,1);
        hex_indicated = true;
    }
    istringstream instr(s);
    int n;
    if(hex_indicated)
        instr >> hex >> n;
    else
        instr >> n;
    return n;
}

void sicxe_asm::is_valid(int line_num,string operand, bool immediate, int size) {
    //#$1234
    //checks if valid hex digit for $ instructions
    string valid_hex_digits = "ABCDEFabcdef0123456789";
    if(operand[0] == '$') {
        if(operand.find_first_not_of(valid_hex_digits,1) != -1) {
            message = "Invalid digit in hex indicated operand " + operand + " on line ";
            print_error('f',message,line_num);
        }
    }

    //checks if valid number for decimal value
    else{
        int check2 = operand.find_first_not_of("0123456789");
        if(check2 != -1) {
            message = "Error invalid operand " + operand + " on line ";
            print_error('f',message,line_num);
        }
    }

    int c_value = string_to_int(operand);
    //checks if #C value is in range
    if(immediate == true){
        if(size == 3) {
            //if out of range
            if(c_value < -2048 || c_value > 2047){
                message = "Immediate value out of range on line ";
                print_error('o',message,line_num);
            }
        }
        //size == 4
        else {
            //if out of range
            if(c_value < -524288 || c_value > 524287){
                message = "Immediate value out of range on line ";
                print_error('o',message,line_num);
            }
        }
    }
    //checks if C value is in range
    else{
        if(size == 3) {
            if(c_value < 0 || c_value > 4095){
                message = "Immediate value out of range on line ";
                print_error('o',message,line_num);
            }
        }
        //size == 4
        else {
            if(c_value < 0 || c_value > 1048575){
                message = "Immediate value out of range on line ";
                print_error('o',message,line_num);
            }
        }
    }
    return;
}

int sicxe_asm::check_offset(int line_num, int TA, int instruction){
    int offset = TA - PC;
    if(offset >= -2048 && offset <= 2047){
        if(offset < 0){
            offset &= 0xFFF;
        }
        instruction |= pbit;
        instruction |= offset;
        return instruction;
    }
    if(base_found == true){
		int b;
		if(isalpha(BASE[0])) {
			if(!symbols.contains_symbol(BASE)) {
				message = "Cannot use base for displacement on line ";
				print_error('o',message,line_num);
			}
			b = symbols.get_value(BASE);
		}
		else {
			b = string_to_int(BASE);
		}
		//cout << "BASE is: " << BASE << endl;
		//cout << "Base is: " << b << endl;
        offset = TA - b;
        if(offset <= 4095 && offset >= 0){
            instruction |= bbit;
            instruction |= offset;
            return instruction;
        }
    }
    message = "Offset out of bounds, cannot do format 3 instruction on line ";
    print_error('s',message,line_num);
}

void sicxe_asm::validate_number(string operand, int line_num){
    string valid_hex_digits = "ABCDEFabcdef0123456789";
    if(operand[0] == '$') {
        if(operand.find_first_not_of(valid_hex_digits,1) != -1) {
            message = "Invalid digit in hex indicated operand " + operand + " on line ";
            print_error('f',message,line_num);
        }
    }

    //checks if valid number for decimal value
    else{
        int check2 = operand.find_first_not_of("0123456789");
        if(check2 != -1) {
            message = "Error invalid operand " + operand + " on line ";
            print_error('f',message,line_num);
        }
    }
}

void sicxe_asm::print_error(char type,string s,unsigned int line_number) {
    switch(type) {
        case 'f':{
            stringstream file;
            file << line_number;
            string file_line = file.str();
            throw file_parse_exception(s + file_line + '.');
        }
        case 'o':{
            stringstream op;
            op << line_number;
            string op_line = op.str();
            throw opcode_error_exception(s + op_line + '.');
        }
        case 's':{
            stringstream ss;
            ss << line_number;
            string sym_line = ss.str();
            throw symtab_exception(s + sym_line + '.');
        }
        default:
            return;
    }
}
