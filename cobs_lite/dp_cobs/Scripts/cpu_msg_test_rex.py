import time
from cobs_logger import cbs_logger

rex = currentDevice
def test_cpu_cmd_resp(timeout=60, polling_rate=1, expected_data=0x1a):
    """
    Test REX CPU message icmd response.
        * Until received the expected data, Rex keeps polling the message received from LEX and
          increment the data by 1 and send back to LEX.
        * This test is done by either REX received the expected data or timeout reached.
    """
    start = time.time()
    while True:
        # Time out and break out the polling.
        timespan = time.time() - start
        if timespan > timeout:
            print "{} seconds timeout is reached.".format(timeout)
            break

        try:
            # Check the received data.
            icmd_obj = rex.create_icmd('CPU_COMM_COMPONENT', 'CPU_COMM_ReadCpuMessageIcmd', True) 
            (valid, data0, data1) = rex.send_icmd_wait_for_response(icmd_obj)
            print "Data read from REX FIFO is {}, {}, {}.".format(valid, data0, data1)
            if valid:
                if data0 == expected_data or data1 == expected_data:
                    icmd_obj = rex.create_icmd(
                                    'CPU_COMM_COMPONENT',
                                    'CPU_COMM_SendCpuMessageIcmd',
                                    False,
                                    [0xff, 0xff])
                    rex.send_icmd(icmd_obj)
                    print "Rex received the expected data {}".format(expected_data)
                    break

                # Increment the data by 1 and send back to LEX.
                data0 += 1
                data1 += 1
                icmd_obj = rex.create_icmd(
                                'CPU_COMM_COMPONENT',
                                'CPU_COMM_SendCpuMessageIcmd',
                                False,
                                [data0, data1])
                rex.send_icmd(icmd_obj)

        except:
            cbs_logger.exception("Got an error when running rex cpu msg test")
            raise Exception("Failed to test REX CPU cmd response")

        # Set a polling rate.
        time.sleep(polling_rate)

    print "Rex test is done."

test_cpu_cmd_resp()
