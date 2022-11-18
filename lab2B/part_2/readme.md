# Task 2: REPL

In this task, I implemented a REPL which could be used to operate register directly. Here is how it works:

(1) Input 'r'/'R' at any time will set the REPL to read mode.

a. Firstly, under the read mode, the REPL will wait for the register address in 8 hex numbers. you could input 0-9,A-F or a-f to input the address. During this phase, the REPL will have the isAddress flag as 1 and the isMask flag as 0.

b. Secondly, after the the REPL will wait for the mask input in 8 hex numbers. You could also type 0-9,A-F or a-f in the console to input your mask. During this phase, the REPL will have the isMask flag as 1 and isAddress flag as 0.

c.Finally, after the mask has been input, the REPL will extract the value from the register and wait for the next address input.

(2) Input 'w'/'W' at any time will set the REPL to write mode.

a. Firstly, under the write mode, the first steps are the same with the read mode. The address and mask input is needed.

b. Secondly, after the address and mask has been obtained by the REPL, both the isAddress flag and the isMask flag will be reset as 0 and the REPL is waiting for the writing content in the format of 8 hex numbers.

c. Finally, after the writing content has been input, the REPL will write the value directly to the register address we typed. What needs to be mentioned specifically is that some registers don't allow bitwise operation. These registers are treated differently by 32-bit whole register writing. Other registers are operated bitwisely based on the mask.

(3) Based on the requirement of the part 4, I have just changed the REPL output to the transition only and this will be shown in the part 6. That means that the REPL will print the register value only when the value in the register changes.



Examples:

[Read Example](https://github.com/minghuin/ESE5190_Lab/blob/main/lab2B/part_2/part2-REPL-read.gif)

[Write Example](https://github.com/minghuin/ESE5190_Lab/blob/main/lab2B/part_2/part2-REPL-write.gif)