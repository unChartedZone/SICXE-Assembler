prog    start   0
bsrch   equ     6000
offb    equ     8
        base    delta
        ldb     #delta
        lds     #3
        +ldx    @FOO
        +lda    beta,x
        add     gamma
        ldx     @zeta
first   NORM
        SVC     13
        HIO
        ldt     #alpha
        +ldt    #FOO
        LDA     @alpha
        LDA     @FOO
        addr    s,t
        shiftr  t,2
        shiftl  s,1
        tixr    t
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

alpha   resw    100
beta    resw    100
gamma   resw    100
delta   word    20
zeta    byte    C'EOF'
eta     byte    x'FFFFFE'
theta   byte    c'eof'
buffer  resb    3000
FOO     word    1
        end     prog
