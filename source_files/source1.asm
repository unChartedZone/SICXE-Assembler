.sample program for SICXE architecture
.

prog    start   $1000    .start of sample program
        base    alpha
        lds     #3
        ldt     #300
        +ldx     #034343
addlf   +lds    alpha    .loop control
        add     beta
        sta     gamma
        addr    s,x
        compr   t
        jlt     addlf
zeta    EQU     addlf

        .Hello World
. storage allocation section

golf    byte    C'EOF'
alpha   resw    2
beta    resw    10
char	word	C'EOF'
delta	resb	200
gamma   resb    10
        end     prog    .end of sample program
