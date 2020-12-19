; This is case 2 program.
; This case is used to recognize language L2 = {1^m x 1^n = 1^mn|m,n \in Natural\{0} }
; Input:  e.g. '11x111=111111'

; the finite set of states
#Q = {q10,q11,q20,q21,q30,q31,q32,q40,q41,q42,q_rej,q_rej_tail,q_acc,q_true,q_t,q_tr,q_tru,q_f,q_fa,q_fal,q_fals,q_false}

; the finite set of input symbols
#S = { 1, x, =}

; the complete set of tape symbols
#G = { 1, x,=,_,t, r, u, e, f, a, l, s}

; the start state
#q0 = q10

; the blank symbol
#B = _

; the set of final states
#F = {q_true}

; the number of tapes
#N = 3

; the transition function

; step1. Put the first 1's string of tape0 into tape1 and delete it in tape0
q10 x__ ___ r** q_rej
q10 =__ ___ r** q_rej
q10 1__ _1_ rr* q11
q10 ___ ___ *** q_rej_tail
q11 x__ ___ rl* q20
q11 =__ ___ r** q_rej
q11 1__ _1_ rr* q11
q11 ___ ___ *** q_rej_tail

; step2. Put the second 1's string of tape0 into tape2 and delete it in tape0
q20 x1_ ___ r** q_rej
q20 =1_ ___ r** q_rej
q20 11_ _11 r*r q21
q20 _1_ ___ *** q_rej_tail
q21 x1_ ___ r** q_rej
q21 =1_ _1_ r*l q30
q21 11_ _11 r*r q21
q21 _1_ ___ *** q_rej_tail

; step3. Put the point index of tape1 and tape2 to 0
q30 x11 ___ r** q_rej
q30 =11 ___ r** q_rej
q30 _11 ___ *** q_rej_tail
q30 111 111 *l* q31
q31 111 111 *l* q31
q31 1_1 1_1 *r* q32
q32 111 111 **l q32
q32 11_ 11_ **r q40

; step4. repeat: point of tape1 moves right one bit, 
;                points of tape0 and tape2 moves right syncly until point of tape2 reaches B
q40 111 1_1 *r* q41
q41 ___ ___ *** q_acc
q41 1_1 __1 r*r q41
q41 111 _11 r*r q41
q41 11_ 11_ **l q42
q41 __1 ___ *** q_rej_tail
q41 _1_ ___ *** q_rej_tail
q41 _11 ___ *** q_rej_tail
q41 x__ ___ r** q_rej
q41 =__ ___ r** q_rej
q41 x1_ ___ r** q_rej
q41 =1_ ___ r** q_rej
q41 x_1 ___ r** q_rej
q41 =_1 ___ r** q_rej
q41 x11 ___ r** q_rej
q41 =11 ___ r** q_rej
q41 1__ ___ r** q_rej
q42 111 111 **l q42
q42 11_ 11_ **r q40

; step5. q_rej=>q_rej_tail=>q_false   q_acc=>qtrue
q_rej ___ ___ *** q_rej_tail
q_rej 1__ ___ r** q_rej
q_rej x__ ___ r** q_rej
q_rej =__ ___ r** q_rej
q_rej_tail ___ f__ r** q_f
q_f ___ a__ r** q_fa
q_fa ___ l__ r** q_fal
q_fal ___ s__ r** q_fals
q_fals ___ e__ *** q_false

q_acc ___ t__ r** q_t
q_t ___ r__ r** q_tr
q_tr ___ u__ r** q_tru
q_tru ___ e__ *** q_true