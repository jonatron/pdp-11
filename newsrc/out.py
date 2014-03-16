
import wiringpi2
wiringpi2.mcp23017Setup(100, 0x20)
wiringpi2.mcp23017Setup(200, 0x21)
wiringpi2.mcp23017Setup(300, 0x22)
wiringpi2.mcp23017Setup(400, 0x23)
wiringpi2.mcp23017Setup(500, 0x24)


num = 209
while True:
	for i in range(209, 215+1):
		wiringpi2.pinMode(i, 1) #1 = output
		wiringpi2.digitalWrite(i, 0)
	for i in range(300, 315+1):
		wiringpi2.pinMode(i, 1)
		wiringpi2.digitalWrite(i, 0)
	for i in range(400, 415+1):
		wiringpi2.pinMode(i, 1)
		wiringpi2.digitalWrite(i, 0)
	for i in range(500, 501+1):
		wiringpi2.pinMode(i, 1)
		wiringpi2.digitalWrite(i, 0)

	wiringpi2.digitalWrite(num, 1)	
	n = raw_input("Number?")
	num = int(n)

