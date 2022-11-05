import serial
import time
import getch
import os
import json

def recordFromConsole(recordLength, sp):
    # set the REPL to read mode
    sp.write(b'R')
    consoleContent = []
    currentRecordDict = {}

    while len(consoleContent) < recordLength:
        if(sp.in_waiting > 0):
            serialString = sp.readline()
            serialStringDecoded = serialString.decode("ascii")
            commendElementLst = serialStringDecoded.split(" ")

            if serialStringDecoded != "\r\n":
                print(serialStringDecoded)
                if "Register" in serialStringDecoded:
                    # create a new record
                    currentRecordDict = {}
                    # save the register address
                    currentRecordDict["Address"] = commendElementLst[-1][2:10]
                elif commendElementLst[0] == 'Value:':
                    # save the register value
                    currentRecordDict["Value"] = commendElementLst[-1][2:10]
                elif "Read" in serialStringDecoded:
                    currentRecordDict["Read"] = commendElementLst[-1][0]
                elif "input:" in serialStringDecoded:
                    currentRecordDict["Input"] = commendElementLst[1]
                elif "write_content:" in serialStringDecoded:
                    currentRecordDict["WriteContent"] = commendElementLst[1][2:]
                elif "Mask" in serialStringDecoded:
                    currentRecordDict["MaskValue"] = commendElementLst[2][2:]
                elif "timeStamp" in serialStringDecoded:
                    currentRecordDict["TimeStamp"] = commendElementLst[-2]
                else:
                    pass
            print(currentRecordDict)

            if len(currentRecordDict.keys()) == 7 and currentRecordDict not in consoleContent:
                consoleContent.append(currentRecordDict)
                #    consoleContent.append(serialString)

    return consoleContent


def writeToConsole(loopNum, writeData, sp):
    # set the REPL to write mode
    sp.write(b'W')
    time.sleep(0.1)

    for i in range(loopNum):
        print('loop number '+ str(i) + " starts")
        for j in range(len(writeData) - 1):
            currentDataRecord = writeData[j]
            currentAddress = 'd0000010'
            if currentDataRecord['Value'][2] == '2':
                currentValue = '00000800'
            else:
                currentValue = '20000800'
            currentMask = currentDataRecord['MaskValue']

            currentRecordLastFor = int(writeData[j+1]['TimeStamp']) - int(writeData[j]['TimeStamp'])

            for k in range(currentRecordLastFor):
                # input the address
                AddressLst = list(currentAddress)
                for bit in AddressLst:
                    sp.write(bytes(bit , encoding="utf8"))
                    time.sleep(0.1)
                
                # input the mask
                if currentMask != "":
                    MaskLst = (currentMask)
                    for bit in MaskLst:
                        sp.write(bytes(bit , encoding="utf8"))
                        time.sleep(0.1)
                else:# if mask is null, use FFFFFFFF
                    for k in range(8):
                        sp.write(b'F')
                        time.sleep(0.1)
                        
                # input the value
                ValueLst = list(currentValue)
                for bit in ValueLst:
                    sp.write(bytes(bit , encoding="utf8"))
                    time.sleep(0.1)

    sp.write(b'R')




# initialize the serialPort
serialPort = serial.Serial(
    port= '/dev/ttyACM0',
    baudrate = 115200)

addressLst = list('D0000010')
maskLst = list('FFFFFFFF')
currentRecordDict = {}


# determine the mode, read or write 
mode = input("Plase choose the mode(record/replay):")
if(mode == 'record'):
    # setup the listening register
    # TODO read the input of address, mask
    for char in addressLst:
        serialPort.write(bytes(char, encoding="utf8"))
        time.sleep(0.1)
        
    for char in maskLst:
        serialPort.write(bytes(char, encoding="utf8"))
        time.sleep(0.1)
    
    while(1):
        needRecord = input("Plase input the length of recording:")
        if(needRecord.isdigit()):
            recordLen= int(needRecord)
            break
        else:
            pass
    
    print("start recording after 2 second")
    time.sleep(2)
    print("recording...")
    records = recordFromConsole(recordLength=recordLen, sp= serialPort)
    print("Record Completed")
    b = json.dumps(records)
    f2 = open('/home/ncm656112/ESE519_Lab/lab2A/Seuencer/recorded_data.json','w')
    f2.write(b)
    f2.close()
    print("Recording finished, please go to /home/ncm656112/ESE519_Lab/lab2A/Sequencer/recorded_data.json to check it")
elif(mode == 'replay'):
    # get the file path from the user input
    while(1):
        path = input("Plase input the absolute path to the recorded data file in json format")
        if(os.path.exists(path)):
            print("Path is valid")
            break
        else:
            pass
    
    # loop time
    while(1):
        loopTime = input("Plase input the loop Time:")
        if(loopTime.isdigit()):
            count= int(loopTime)
            break
        else:
            pass

    # TODO write the data to the serial terminal
    f3 = open(path,'r')
    data = json.load(f3)
    f3.close()

    writeToConsole(count, data, serialPort)
    
else:
    pass





        
        


    



    




