passed = 1

for x in xrange(0, 256):
    for y in xrange(0, 256):
        value = currentDevice.iCmdResps['GRG_COMPONENT']['multiply16bit'].respSend(x, y)
        result = int(value[0])
        if result != (x * y):
            print "Fail: expected a result of " + str(x * y) 
            print "Received a result of " + str(result)
            print "Failed at iteration " + str(x) + ", " + str(y)
            passed = 0
            break
    if passed == 0:
        break
        
    print "Running outer loop " + str(x+1) + " of 256"        

if passed == 1:        
    print "All multiplications from (0 to 255) * (0 to 255) completed successfully"
else:
    print "Test failed"