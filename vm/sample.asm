# sample.asm - contador decrescente (exemplo)
# MEM[0] será usado para a variável 'contador'

LDI R0 5        # R0 = 5
STORE R0 0      # MEM[0] = 5

loop_start:
LOAD R0 0       # R0 = MEM[0]
JZ R0 end       # se R0 == 0, pula para end
PRINT_MEM 0     # torcida(contador)
LOAD R0 0
LDI R1 1
SUB R0 R1       # R0 = R0 - 1
STORE R0 0
JMP loop_start

end:
HALT