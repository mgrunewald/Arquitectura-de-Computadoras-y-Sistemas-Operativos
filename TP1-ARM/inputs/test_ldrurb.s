.text
mov X1, 0x1000
lsl X1, X1, 16
mov X10, 0xAB
sturb W10, [X1, 0x6]
ldurb W15, [X1, 0x6]
HLT 0