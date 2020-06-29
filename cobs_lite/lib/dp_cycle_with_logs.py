import argparse
import serial
import datetime
import time
import os
import struct
import gzip
import threading as Threading
import serial.tools.list_ports

import script_lib.icron_file_parser as ifp
import script_lib.loaded_icron_file as lif
import script_lib.icron_model as im
import script_lib.icron_ilog as ilg
import script_lib.icron_device_info as idi
import script_lib.icron_istatus as iis
import script_lib.log_decoder as decoder


run_once = 0

def performCycle(ser, port, time_delay, count):
    for counter in range (0, count):
        print("cycles remaining = ", count - counter)
        time.sleep(0.2)
        testCycle(ser)
        time.sleep(time_delay)

def testCycle(ser):
    icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_RestartDPStateMachine", False)
    executeIcmd(icmd_obj, ser)

def SwVersionIcmd(ser):
    icmd_obj = loaded_icron_file.create_icmd("TOPLEVEL_COMPONENT", "PrintSwVersion", False)
    executeIcmd(icmd_obj, ser)

def executeIcmd(icmd_obj, ser):
    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd, args=(icmd_obj, ser,))
    icmd_thread.start()
    icmd_thread.join()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--device", help="device to read from", default="COM5")
    parser.add_argument("-b", "--baud", help="baud rate in bps", default=460800, type=int)
    parser.add_argument("-t", "--time_delay", help="time delay for loop", default=1, type=int)
    parser.add_argument("-i", "--icron_file", help="icron file to be used")
    parser.add_argument("-c", "--count", help="repeat test this number of times", default=1, type=int)
    args = parser.parse_args()

    byte_list = []
    ts_bytes = ''

    outputFileDir = os.path.join(os.path.dirname(__file__), "logs")
    if not os.path.exists(outputFileDir):
        os.mkdir(outputFileDir)

    dateTimeStamp = datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S")
    os.mkdir(os.path.join(outputFileDir, dateTimeStamp))
    outputFilePath_1 = os.path.join(os.path.dirname(__file__), "logs", dateTimeStamp, datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S") + ".bin")
    outputFilePath_2 = os.path.join(os.path.dirname(__file__), "logs", dateTimeStamp, datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S") + ".gz")
    outputFilePath_3 = os.path.join(os.path.dirname(__file__), "logs", dateTimeStamp, datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S") + ".log")

    iparsed_file = ifp.IcronParsedFile(args.icron_file)
    loaded_icron_file = lif.Loaded_icron_file(iparsed_file, "blackbird", args.device, args.baud)

    with serial.Serial(args.device, args.baud) as ser, open(outputFilePath_1, mode='wb') as outputBinFile:
        print("\nLogging started. Saving as a bin file. \n Ctrl-C to stop.")
        ser.flushInput()
        ser.flushOutput()

        try:
            SwVersionIcmd(ser)
            while True:
                time.sleep(.1)
                outputBinFile.write((ser.read(ser.inWaiting())))
                outputBinFile.flush()
                if run_once == 0:
                    performCycle(ser, args.device, args.time_delay, args.count)
                    print("\n Cycling Finished. Saving as a bin file. \n Ctrl-C to stop logging.")
                    run_once = 1
                else:
                    print("\n Testing completed, terminating logging in 30 seconds")
                    termination_time = 30
                    time.sleep(30)
                    raise KeyboardInterrupt

        except KeyboardInterrupt:
            ser.close()
            with open(outputFilePath_1, "rb") as inputFile:
                with gzip.open(outputFilePath_2, mode='wb') as outputFile:
                    while True:
                        ser_byte = inputFile.read(1)
                        if ser_byte:
                            byte_list.append(ord(ser_byte))
                            if len(byte_list) == 24:
                                ts_bytes = struct.pack('!Q', int(time.time() * 1000000))
                                raw_bytes = struct.pack('{}B'.format(len(byte_list)), byte_list[0], byte_list[1], byte_list[2], byte_list[3], byte_list[4], byte_list[5], byte_list[6], byte_list[7], byte_list[8], byte_list[9], byte_list[10], byte_list[11], byte_list[12], byte_list[13], byte_list[14], byte_list[15], byte_list[16], byte_list[17], byte_list[18], byte_list[19], byte_list[20], byte_list[21], byte_list[22], byte_list[23])
                                byte_list = []
                                data_bytes = ts_bytes + raw_bytes
                                ts_bytes = ''

                                outputFile.write(data_bytes)
                                outputFile.flush()
                        else:
                            inputFile.close()
                            outputFile.close()
                            break
            print("Logging stopped, decoding the file now")

            log_file_decoder = decoder.IcronLogFileDecoder()

            for project in iparsed_file.projects:
                short_device_name = iparsed_file.get_short_device_name(project)
                ilog_model = im.IcronILogModel(
                                            project,
                                            iparsed_file.get_icomponent_json(project),
                                            iparsed_file.get_ilog_json(project),
                                            iparsed_file.get_severity_json(project))

                ilog_decoder = ilg.ILogDecoder(ilog_model)

                ichannel_model = im.IcronChannelIdModel(
                                            project,
                                            iparsed_file.get_ichannel_id_json(project))

                istatus_model = im.IcronIStatusModel(project, iparsed_file.get_istatus_json(project))

                istatus_decoder = iis.IStatusDecoder(istatus_model)

                ilog_channel_id = ichannel_model.ilog_channel_id

                istatus_channel_id = ichannel_model.istatus_channel_id

                printf_channel_id = ichannel_model.printf_channel_id

                device_info_channel_id = ichannel_model.program_status_channel_id

                log_file_decoder.register_channel_packet_attributes(
                                                        ilog_channel_id,
                                                        short_device_name,
                                                        ilg.parse_ilog, 
                                                        ilog_decoder.decode)

                log_file_decoder.register_channel_packet_attributes(
                                                        istatus_channel_id,
                                                        short_device_name,
                                                        iis.parse_istatus,
                                                        istatus_decoder.decode)

                log_file_decoder.register_channel_packet_attributes(
                                                        printf_channel_id,
                                                        short_device_name,
                                                        None,
                                                        None)

                log_file_decoder.register_channel_packet_attributes(
                                                        device_info_channel_id,
                                                        short_device_name,
                                                        None,
                                                        idi.decode_info_message)

            with open(outputFilePath_3, 'a', encoding='utf-8') as out_file:
                log_file_decoder.register_log_message_handler(out_file.write)
                log_file_decoder.decode(outputFilePath_2)
                log_file_decoder.remove_log_message_handler(out_file.write)
            os.startfile(os.path.join(outputFileDir, dateTimeStamp))
