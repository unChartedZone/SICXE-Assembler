/* Karah Hui, Sho Miyauchi, Christian Valdez, Steven Grimes
 * Team Michigan
 * sicxe_asm.h
 * masc0869
 * prog4
 * CS530, Spring 2016
 */

#ifndef SICXE_ASM_H
#define SICXE_ASM_H

#define nbit 0x20000
#define ibit 0x10000
#define xbit 0x8000
#define bbit 0x4000
#define pbit 0x2000
#define ebit 0x1000

#include <iostream>
#include <map>
#include <stack>
#include <queue>
#include "file_parser.h"
#include "opcodetab.h"
#include "symtab.h"

using namespace std;

class sicxe_asm {
	public:
		//ctor of the assembler.
		sicxe_asm(string);

		//dtor of the assembler.
		~sicxe_asm();

		//method that will run first pass of the assembler
		void first_pass();

		void second_pass();

		//create the .lis file, calling write_file to format properly
		void make_output_file(string);

		void make_executable_file(string);

	private:
		//your variables and private methods go here

		file_parser parser;	//to parse file and access individual tokens
		opcodetab table;	//loads a map of all the opcodes
		symtab symbols;	//map containing all user defined symbols

		long LOCCTR;	//used to assign addresses
		long PC;
		string BASE;
		string message;		//used for error messages

		int starting_address; //Used for header_record
		string program_name; //Used for header_record
		int length_of_program; //Used for header_record
		// int index_first_mcode; //Used for text_record

		//int TA; 	= target address
		bool start_found;	//true if start directive is found
		bool end_found;	//true once end directive is found
		bool first_pass_completed;
		bool base_found;

		//filled with all the addresses in hex format
		vector<string> addresses;

		//filled with all the machine codes in hex format
		vector<string> object_codes;

		//for cascading forward references
		struct reference{
			string label;
			string operand;
			int line_num;
		};

		struct modifiers {
			string address;
			string size;
		};

		stack <reference> cascading_references;

		queue <modifiers> mod_records;

		/*******First Pass Methods*******/

		//insert the location counter as a string into the addresses vector
		void insert_into_address();

		//checks if the directive has been found, if yes, then it executes
		//the respective method, and then returns true, else returns false
		bool run_directs(string,string,string,int);

		/*******Directive Methods*******/

		void start_directive(string,int);	//handles the START directive
		void end_directive(string);			//handles the END directive
		void byte_directive(string,int);	//handles the BYTE directive
		void word_directive(string,int);	//handles the WORD directive
		void resw_directive(string,int);	//handles the RESW directive
		void resb_directive(string,int);	//handles the RESB directive
		void equ_directive(string,string,int);	//handles the EQU directive
		void base_directive(string,int);
		void nobase_directive(int);

		/******Second Pass Methods******/

		void format_one(string,string,int);
		void format_two(string,string,int);
		void format_three_and_four(int,int,string,string);

		//These four methods can handle both
		//format 3 and format 4 instructions
		void immediate_addressing(int,int,string,string);
		void indirect_addressing(int,int,string,string);
		void indexed_addressing(int,int,int,string,string);
		void direct_addressing(int,int,string,string);

		void generate_byte(int,string);
		void generate_word(int,string);

		int convert_machine_code(string); //Can also be used to convert hexadecimal numbers
		string format_hex_number(int,int);
		void validate_register(string,int);
		void outliers(int,string,int);

		/*******Helper Methods*********/

		//converts string parameter to all capital characters
		string capitalize(string);


		//converts a string into an int,
		//also decides whether a hex representation is required or not
		int string_to_int(string);

		void is_valid(int,string,bool,int);

        void validate_number(string,int);

		int check_offset(int,int,int);

		void print_error(char,string,unsigned int);

		/*********Output File Method*********/
		//a modified version of print_file from the file_parser class,
		//used for writing to the .lis file
		//Modifications: removing comments, leaving room for addresses
		//and object code for later
		void write_file(string);
		/***********************************/

		/********Executable File Methods*********/

		void header_record(ofstream&);
		void text_record(ofstream&);
		void modification_record(ofstream&);
		void end_record(ofstream&);
		string pad_spaces(string);
		int set_counter(int);

		//Used in text record to insert the lenght of the machine codes of the record into the line
		//and the line is then inserted into the object file
		void write_text_line(ofstream &exe_file, string line);
	/****************************************/
	void check_for_reserve(string line, ofstream &exe_file, int &counter, bool &reserve_detected);
	void create_mod_data(int line_num, string size, string operand);
};

#endif
