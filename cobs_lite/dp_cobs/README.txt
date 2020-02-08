Overview
    Cobs is the new developing and debugging software tool written in IronPython and .Net for the
    Blackbird project. The implementation of Cobs is based the existing design of Hobbes, the
    software tool for Goldenears project.

Requirments
    Developers should be able to use Cobs to debug Blackbird FW from Icron log, program FW and FPGA
    into Blackbird chips, and view/modify registers values of the chip in real time. Due to the
    fact that GE ASIC is connected to Blackbird downstream and shares the same UART port with
    Blackbird, FW and Cobs are required to create virtual channels through packetization in UART
    driver that separates the communication between Cobs and the Blackbird device.

Changes
    There are no significant changes in the existing Icron ilog and icommand protocol as well as in
    register view other than the fact that they are built on the top of Icron packet layer.
    However, we change how Icron files are loaded into Cobs and how ilog is decoded and icmd is
    encoded by introducing new json format and Icron modules. Also, the XMODEM used in GE for
    device programming is replaced with a new Icron customized programming protocol for Blackbird,
    but XMODEM still applies for Goldenears programmed and handled by Blackbird.

Design
    Cobs still inherits the architecture of Hobbes. There is no clear distinguished interface
    between GUI view and controller function. CobsInterpreter.py is the Cobs startup window driver
    that is response for connecting Cobs to device and loading .icron file executed by users.
    Device.py contains the GUI view of device window and interface that physically controls the
    device. To deal with the common serial port shared by BB and GE, serial_port_manager.py is
    introduced. It provides register_listener() and remove_listener() APIs to manage device clients
    that are connected to the serial port. Serial port manger subscribes to the serial port
    DataReceived event. When the serial port has received data from device, the
    serial_port_data_received_handler() will be invoked by passing all received bytes to the
    depacketizer, in which Icron defined packets will be parsed from raw bytes. Inside the
    depacketizer, it has its own APIs for managing clients that are expected to handle received
    packets. The depacketizer utilizes the pyevent module to trigger the packet received event and
    pass the packet to registered clients through the serial port manager once a packet is found.
    On the transmitter side, clients will directly call serial_port_manger.write() to send data to
    Blackbird device. One has to keep in mind that all transmitted data are in packet format
    before they are sent out.

Implementation
    Throughout the Cobs project, an icron_lib library has been created in order to implement Icron's
    specific communication protocols including ilog, icmd, istatus, programming, crc,
    de/packetization and system command and device info. Also, icron_file_parser and icron_model
    are introduced for loading Icron FW defined device database into Cobs such as ilog, icmd,
    istatus, iregister and channel id definitions. The client of Icron modules in Cobs is
    implemented in Device.py. When users click add device button in startup window, the icron files
    will be loaded and parsed first and return projects defined in icron_header.json file. For each
    project returned, a deice client object will be instantiated from DeviceClient class. For Cobs,
    there are two expected projects 'goldenears' and 'blackbird' so as the device clients, which
    means each device client has its own associated device view, model and controller. However,
    there is no separation among those three major GUI software components in device client as the
    architecture is inherited from Hobbes. After devices loads icron file, launch_device_window()
    is called to initialized the device window view. Through device.connect(),
    packet_received_handler() is registered to serial_port_manager for handling received packets
    passed from depacketizer. register_packet_handler() allows each specific packet handler to be
    assigned into ipacket_handlers a dictionary used to store and invoke packet handlers. When
    device client receives a packet, a timestamp is added and passed to the corresponding packet
    handler along with the packet according to its response id and channel id. Note that the device
    client will receive all packets regardless whether the packet is only specific to goldenears or
    blackbird. This is because pyevent can only broadcast but not multicast the event at least it
    requires some customization. Also, receiving all packets may be useful for ExCOMM, the external
    tool, which only launches one device client for one serial port. For sending command to device,
    device client has two APIs: send_icmd() and send_device_command(). They have very similar
    designs and implementations. send_icmd() is based on the Icron icmd protocol and for internal use
    only, whereas send_device_command() can be exposed external customers. They both have its own
    for sending and then polling result returned from device, namely send_icmd_wait_for_response()
    and send_device_command_wait_for_response() respectively.
