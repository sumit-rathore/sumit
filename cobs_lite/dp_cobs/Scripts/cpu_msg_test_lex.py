import time
from cobs_logger import cbs_logger


lex = currentDevice
def test_lex_cpu_cmd_resp(timeout=60, polling_rate=1, expected_data=0xff):
    """
    Test LEX CPU message icmd response.
        * LEX sends a 2 words message data 0x1 and 0x2 to REX first.
        * Until received the expected data, lex keeps polling the message received from REX and
          increment the data by 1 and send back to REX.
        * This test is done by either LEX received the expected data or timeout reached.
    """
    # LEX sends 0x1 and 0x2 to REX.
    icmd_obj = lex.create_icmd('CPU_COMM_COMPONENT', 'CPU_COMM_SendCpuMessageIcmd', False, [0x1,
        0x2])
    lex.send_icmd(icmd_obj)

    start = time.time()
    while True:
        # Time out and break out the polling.
        timespan = time.time() - start
        if timespan > timeout:
            print "{} seconds timeout is reached.".format(timeout)
            break

        try:
            # Check the received data.
            icmd_obj = lex.create_icmd('CPU_COMM_COMPONENT', 'CPU_COMM_ReadCpuMessageIcmd', True) 
            (valid, data0, data1) = lex.send_icmd_wait_for_response(icmd_obj)
            print "Data read from LEX FIFO is {}, {}, {}.".format(valid, data0, data1)
            if valid:
                if data0 == expected_data or data1 == expected_data:
                    print "Lex received the expected data {}".format(expected_data)
                    break

                # Increment the data by 1 and send back to REX.
                data0 += 1
                data1 += 1
                icmd_obj = lex.create_icmd(
                                'CPU_COMM_COMPONENT',
                                'CPU_COMM_SendCpuMessageIcmd',
                                False, [data0, data1])
                lex.send_icmd(icmd_obj)

        except:
            cbs_logger.exception("Got an error when running lex cpu msg test")
            raise Exception("Failed to test lex CPU icmd response")

        # Set a polling rate.
        time.sleep(polling_rate)

    print "Lex test is done."

test_lex_cpu_cmd_resp()
