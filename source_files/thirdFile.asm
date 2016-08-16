
prog    start   0
bsrch   EQU     6000
OFFB    EQU     8
        BASE    delta
        LDB     #delta
        lds     #3
        +ldx    @foo
        +lda    beta,x
        add     gamma
        ldx     @zeta
first   NORM
        SVC     13
        HIO
        ldt     #alpha
        +ldt    #foo
        lda     @alpha
        lda     @foo
        addr    S,T
        shiftr  T,2
        shiftl  S,1
        tixr    T
        j       first
        +j      first
        Clear   T
        +ldt    #50000
        ldx     #0
        jsub   first
addl    lda     alpha,x
        add     beta,x
        sta     gamma,x
        addr    s,t
        compr   x,t
        +jlt    addl
        jlt     addl
        rsub


test    byte    C'12345'
test2   byte    x'7b'
alpha   resw    100
beta    RESW    100
gamma   resw    100
delta   WORD    20
zeta    BYTE    C'EOF'
eta     byte    X'FFFFFE'
theta   byte    c'Goodbye World!!!'
buffer  RESB    3000
foo     word    1
        end prog
