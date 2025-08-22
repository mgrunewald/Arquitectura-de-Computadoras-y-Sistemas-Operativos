.text
mov X1, 0x1000
lsl X1, X1, 16 
mov X10, 0xABCD
stur X10, [X1, 0x2]
ldurh X11, [X1, 0x2]
HLT 0