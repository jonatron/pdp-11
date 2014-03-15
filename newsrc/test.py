import wiringpi2
wiringpi2.mcp23017Setup(100, 0x20)
wiringpi2.mcp23017Setup(200, 0x21)
wiringpi2.mcp23017Setup(300, 0x22)
wiringpi2.mcp23017Setup(400, 0x23)
wiringpi2.mcp23017Setup(500, 0x24)

for i in range(100, 115+1):
	#print "setting input %s", str(i)
	wiringpi2.pinMode(i, 0) #0 = input
	wiringpi2.pullUpDnControl(i, 2) #2 = pull up   

for i in range(200, 208+1):
	#print "setting input %s", str(i)
	wiringpi2.pinMode(i, 0) #0 = input
	wiringpi2.pullUpDnControl(i, 2) #2 = pull up   

out = ""
out2 = ""
for i in range(100, 115+1):
	out += str(wiringpi2.digitalRead(i)) + "   "
	out2 += str(i) + " "
	#out += "%s = %s" % ( i, wiringpi2.digitalRead(i) )
print out2
print out
