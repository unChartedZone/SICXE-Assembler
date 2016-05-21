

prog    start   0
bsrch   EQU     6000
OFFB    EQU     8
        BASE    delta
        LDB     #delta
        lds     3
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
        lda     foo
        addr    S,T
        shiftr  T,1
        shiftl  S,7
        tixr    T
        j       first
        +j      first
        clear   T
        +ldt    #50000
        ldx     #0
        +jsub   bsrch
addl    lda     alpha,x
        add     beta,x
        sta     gamma,x
        addr    s,t
        compr   x,t
        +jlt    addl
        jlt     addl
        rsub

alpha   resw    100
beta    RESW    100
gamma   RESW    100
delta   WORD    20
zeta    BYTE    C'EOF'
eta     byte    X'FFFFFE'
theta   BYTE    c'eof'
buffer  RESB    3000
foo     word    1
        end     prog
