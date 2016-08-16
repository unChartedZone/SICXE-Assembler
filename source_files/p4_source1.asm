



prog           start           0
bsrch          EQU             6000
OFFB           EQU             8
               lds             #3
               +ldx            @foo
                +lda           beta,x
                add            gamma
                ldx            @zeta
first          NORM
                SVC            13
                HIO
                BASE           foo
                +ldb           #foo
                ldt            #alpha
                +ldt           #foo
                lda            @alpha
                lda            foo
                addr           S,T
                shiftr         T,2
                shiftl         S,1
                tixr           T
                j              first
                +j             first
                Clear          T
                +ldt           #50000
                ldx            #0
                +jsub          bsrch
addl           lda            alpha,x
                add            beta,x
                sta            gamma,x
                addr           s,t
                compr          x,t
                +jlt           addl
                jlt            addl
                rsub


alpha          resw           100
beta           RESW           100
gamma          resw           100
delta          WORD           20
zeta           BYTE           C'EOF'
eta            byte           X'FE'
theta          byte           c'eof'
buffer         RESB           3000
foo            word           1
caleb          WORD           alpha  
                end            prog
