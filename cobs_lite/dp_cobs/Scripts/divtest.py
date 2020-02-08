passed = 1

for x in xrange(1, 1025):
    value = currentDevice.iCmdResps['GRG_COMPONENT']['divide16bit'].respSend(4096, x)
    quotient = int(value[0])
    remainder = int(value[1])
    if (quotient != 4096/x):
        print "Fail: expected a quotient of " + str(4096/x) + " and a remainder of " + str(4096 % x)
        print "Received a quotient of " + str(quotient) + " and a remainder of " + str(remainder)
        print "Failed at iteration " + str(x)
        passed = 0
        break
    else:
        if x % 10 == 0:
            print "Executed test " + str(x)

if passed == 1:        
    print "All divides from 1 to 1024 completed successfully"
else:
    print "Test failed"