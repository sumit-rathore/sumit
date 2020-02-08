
def configureNetwork(ip, subnetMask, defaultGateway):
    '''
    Sends and iCmd to the target to configure the IP address, subnet mask
    and default gateway of the device.
    '''
    try:
        currentDevice.iCmds['NET_COMPONENT']['NET_icmdSetIPv4Configuration'](_ipToNumeric(ip),
           _ipToNumeric(subnetMask), _ipToNumeric(defaultGateway))
    except Exception as ex:
        # show the error that's occurred; this is quite similar to the
        #     exception handling in CobsInterpreter
        print "Could not execute SetIPv4Configuration command.\n" + \
            "Please check the paramters i.e. IP address, subnet mask and default gateway."


def _ipToNumeric(ip):
    error = 'Expected a string containing 4 numbers (0-255) seperated by periods'
    quad = ip.split('.')
    # We reverse the list so that we can easily compute the number representing
    # the whole IP
    quad.reverse()
    if len(quad) != 4:
        raise ValueError(error)
    for i in range(len(quad)):
        try:
            temp = int(quad[i])
        except ValueError as e:
            raise ValueError(error)
        if temp > 255 or temp < 0:
            raise ValueError(error)
        quad[i] = temp * (256 ** i)
    print sum(quad)
    return sum(quad)
