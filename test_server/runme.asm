  main:
        addi $sp, $zero, 32764
        addi $a0, $zero, 10
        jal accm
        j   exit
  accm:
        addi  $sp, $sp, -8
        sw    $ra, 4($sp)  # save the return address
        sw    $a0, 0($sp)
        slti  $t0,$a0,1     # test for n < 1
        beq   $t0,$zero,L1  # if n >= 1, go to L1
        addi  $v0,$zero,0 # return 0
        addi  $sp,$sp,8   # pop 2 items off stack
        jr    $ra         # return to caller
  L1:
        addi $a0,$a0,-1
        jal accm         # call fact with (n –1)
        lw $a0, 0($sp) # return from jal: restore argument n lw $ra, 4($sp) # restore the return address
        lw $ra, 4($sp) # restore the return address
        addi $sp, $sp, 8 # adjust stack pointer to pop 2 items
        add $v0, $a0, $v0
        jr   $ra           # return to the caller
 
  exit:
        addi $t1, $zero, 10  #exit simulation if $t1=10
