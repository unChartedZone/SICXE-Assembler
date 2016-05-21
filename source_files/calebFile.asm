HELLO       START   033
FIRST       LDA     ZERO
            LDX     ZERO

TESTD       TD      STDIN
            JEQ     TESTD       . Loop until ready
            RD      STDIN       . Get input to see how many times to loop
            STA     NUMLOOP     . Save the user's input into NUMLOOP
STLOOP      STX     LOOPCNT     . Save how many times we've loops so far
            LDX     ZERO
            JSUB    PLOOP       . Print the string
            LDX     LOOPCNT     . Reload how many times we've looped so far
            TIX     NUMLOOP     . Increment number of loops so far
            JLT     STLOOP      . Loop again if less than NUMLOOP
            J       COPY        . Jump to COPY (0x1000) to test other program

NUMLOOP     RESW    1
LOOPCNT     RESW    1
STRING      BYTE    C'Hello, world!'
STRLEN      BYTE    X'0D'
STDIN       BYTE    X'00'
STDOUT      BYTE    X'01'
ZERO        WORD    0

. Print string subroutine
PLOOP       TD      STDOUT
            JEQ     PLOOP       . Loop until ready
            LDCH    STRING,X    . STDOUT is ready to print
            WD      STDOUT
            TIX     STRLEN
            .JLE     PLOOP       . Go again if string has more chars
            RSUB

EXIT        END    HELLO
