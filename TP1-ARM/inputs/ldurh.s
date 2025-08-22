.text
mov X1, 0x1000          
lsl X1, X1, 16          
mov X2, 0x1234     
stur X2, [X1, 0x0]     
ldurh W3, [X1]   
mov X4, X3            

HLT 0
