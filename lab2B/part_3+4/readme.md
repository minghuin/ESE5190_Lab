# Part 3+4 Sequencer

In this part, I used python to implement a sequencer, which is a middleware between the user and the REPL. Here are the features of this sequencer.


(1) This sequencer utilizes the PySerial library to receive data from the REPL program running on the microprocessor. The REPL program print some information and the sequencer could read that from the serial port. Similarly, the sequencer program can send some charactors to control the REPL program.

(2) This sequencer has to functions: replay and record. Firstly, this sequencer will ask user to choose the operation mode.

a. if the mode is record, the sequencer will ask for the length of the recording and starts recording from the serial console. Once the length of the recording has reached, the recording will stop and the recorded data will be saved to a predefined path in JSON format. In future, this mode can also ask for a listening address of the register and the mask.

b. if the mode is replay, the sequencer will ask for the path of the JSON file and validize the path. If the path is valid, then the sequencer will read the file and asks user for loop times. After the loop times has been input, the sequencer will keep writing data to the serial port based on the user input to replay the data. Currently, it can only show the data by turning on and off the LED. Additionally, for the part 4, this sequencer now support the transition data replay.

(3) Awaiting features

Part 3: IO-Remaping

PS: the saved JSON format can be used as a user command.

Part 4: output rate adjustment

