package pairingDemo;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import javax.swing.JOptionPane;

public class DeviceInterface 
{
    // XUSBNETCFG Commands
    static String NetConfigCommand = "." + File.separator + "xusbnetcfg.exe";
    static String RequestDeviceInfoCommand = "request_device_info";
    static String PairToDeviceCommand = "pair_to_device";
    static String RemoveDevicePairing = "remove_device_pairing";
    static String RequestExtendedDeviceInfoCommand = "request_extended_device_info";
    static String RequestFilterCommand = "use_filtering_strategy";
    
    static String BroadcastAddress = "192.168.0.255";
    
    // XUSBNETCFG Command Messages
    static String DevicePairingSuccessMessage = "Device paired successfully";
    static String DeviceAlreadyPairedMessage = "Device pairing failed because" +
            " either the device is already paired or all vports are already being used.";
    static String DeviceUnpairingSuccessMessage = "Device pairing removed successfully";
    static String DeviceAlreadyUnpairedMessage = "Cannot remove pairing since device 1" +
            " is not currently paired with device 2.";
    static String DeviceExtendedFailed = "Request Extended Device Information failed!";
    static String DeviceInfoFailed = "Request Device Info failed";
    static String DeviceFilterSuccessMessage = "filtering strategy set";    // TODO: Successful message is unknown
    
    protected String alreadyEstablishedMessage = "Connection Already Established";
    protected String pairingFailedMessage = "Connection Failed";
    protected String unpairingFailedMessage = "Un-Pairing Devices Failed";
    protected String alreadyUnpairedMessage = "Un-Paired Devices Already";
    protected String filterFailedMessage = "Device could not be set to the specified filtering strategy";
    
    public DeviceInterface()
    {
        autoDetectBroadcastIP();
    }
    
    static public void autoDetectBroadcastIP()
    {
        try
        {
            InetAddress localHost = Inet4Address.getLocalHost();
            NetworkInterface networkInterface = NetworkInterface.getByInetAddress(localHost);
            
            BroadcastAddress = networkInterface.getInterfaceAddresses().get(0).getBroadcast().toString().substring(1);
            
            Log.logger.info("Setting default broadcast address to "+BroadcastAddress);
        }
        catch( SocketException|UnknownHostException e )
        {
            e.printStackTrace();
        }
    }
    
    /*
    * Run and parse request device info command Creates new buttons and
    * adds the MAC Address and IP Address
    */
   public void runParseDeviceInfoCommand(HashMap<String, DeviceNode> tempDeviceNodeList)
           throws IOException
   {
       Log.logger.entering(this.getClass().toString(), "runParseDeviceInfoCommand");

       String command = NetConfigCommand
               + " "
               + RequestDeviceInfoCommand
               + " "
               + BroadcastAddress;

       Log.logger.info("Executing command: " + command);

       Process proc = Runtime.getRuntime().exec(command);

       BufferedReader reader = new BufferedReader(new InputStreamReader(proc
               .getInputStream()));
       String line = reader.readLine();
       String[] tokens = line.split(" ");

       // If there are no devices, don't parse device information strings
       if (tokens[4].equals("0"))
           return;

       int numberOfDevices = Integer.valueOf(tokens[4]);

       String allLines = line;

       while (line != null)
       {
           line = reader.readLine();
           allLines += line;
       }

       int requestDeviceInfoFailed = allLines
               .indexOf(DeviceInfoFailed);

       // Request Device Info failed
       if (requestDeviceInfoFailed != -1)
       {
           Log.logger.severe(DeviceInfoFailed);
           Log.logger.exiting(this.getClass().toString(), "runParseDeviceInfoCommand");
           return;
       }

       // Parse MAC Address
       String[] macAddresses = parseTokens(allLines,
               "MAC = ",
               " ",
               numberOfDevices);

       // Parse IP Address
       String[] ipAddresses = parseTokens(allLines,
               "IP = ",
               " ",
               numberOfDevices);

       // Parse Supported Protocol Version
       String[] supportedProtocol = parseTokens(allLines,
               "Supported Protocol Version = ",
               " ",
               numberOfDevices);

       for (int deviceIndex = 0; deviceIndex < numberOfDevices; deviceIndex++)
       {
           DeviceNode node = new DeviceNode();
           node.macAddress = macAddresses[deviceIndex];
           node.ipAddress = ipAddresses[deviceIndex];
           node.supportedProtocol = Integer.valueOf(supportedProtocol[deviceIndex]);

           tempDeviceNodeList.put(node.macAddress, node);

           String deviceName = null;

           /*for (ButtonDemo.MacToDeviceNames device : ButtonDemo.macToDeviceList_)
           {
               if (device.macAddress.equals(node.macAddress))
               {
                   deviceName = device.deviceName;
                   //button.setText(deviceName);
               }
           }

           if (deviceName == null)
           {
               //button.setText(unknownDevice);
           }*/
       }

       Log.logger.exiting(this.getClass().toString(), "runParseDeviceInfoCommand");
   }

   /*
    * Run and parse request extended device info command Adds the isPaired,
    * Paired MAC Address, and lexOrRex information
    */
   public void runParseExtendedDeviceInfoCommand(
           HashMap<String, DeviceNode> tempDeviceNodeList)
           throws IOException
   {
       Log.logger.entering(this.getClass().toString(), "runParseExtendedDeviceInfoCommand");

       for (Iterator<Map.Entry<String, DeviceNode>> deviceIterator = tempDeviceNodeList
               .entrySet().iterator(); deviceIterator.hasNext();)
       {
           DeviceNode node = deviceIterator.next().getValue();

           String command = NetConfigCommand
                   + " forced "
                   + String.valueOf(node.supportedProtocol)
                   + " "
                   + RequestExtendedDeviceInfoCommand
                   + " "
                   + node.ipAddress;

           Log.logger.info("Executing command: " + command);

           Process proc = Runtime.getRuntime().exec(command);

           BufferedReader reader = new BufferedReader(new InputStreamReader(proc
                   .getInputStream()));

           String line = reader.readLine();;

           int numberOfLines = 0;

           while (line != null)
           {
               if (!line.endsWith("\n"))
               {
                   line += "\n";
               }

               if (numberOfLines == 0)
               {
                   int requestExtendedDeviceFailed = line
                           .indexOf(DeviceExtendedFailed);

                   // If device was unplugged between the time
                   // when request_device_info was issued and
                   // request_extended_device_info was issued,
                   // request_extended_device_info will fail
                   if (requestExtendedDeviceFailed != -1)
                   {
                       deviceIterator.remove();
                       node = null;
                       Log.logger.severe(DeviceExtendedFailed);
                       break;
                   }
               }
               else if (numberOfLines == 1)
               {
                   // Parsing lexOrRex
                   String[] lexOrRexToken = parseTokens(
                           line,
                           "lexOrRex = ",
                           "\n",
                           1);

                   node.lexOrRex = DeviceNode.ExtenderType.valueOf(lexOrRexToken[0]);
               }
               else if (numberOfLines > 2)
               {
                   String[] pairedMACToken = parseTokens(
                           line,
                           "Paired MAC = ",
                           "\n",
                           1);
                   node.pairedMACAddresses.add(pairedMACToken[0]);
               }
               numberOfLines++;

               line = reader.readLine();
           }

           /*
           if (node != null)
           {
               final ButtonDemo.ExtenderButton newButton = button;
               final String toolTipString = setupExtenderToolTip(
                       button.macAddress,
                       button.ipAddress,
                       button.lexOrRex.getString(),
                       button.pairedMACAddresses);
               Runnable doWorkRunnable = new Runnable()
               {
                   public void run()
                   {
                       newButton.setToolTipText(toolTipString);
                   }
               };
               SwingUtilities.invokeLater(doWorkRunnable);

               // Place Device Label at the top for LEXs and bottom for REXs
               if (button.lexOrRex == ButtonDemo.ExtenderType.REX)
               {
                   button.setIcon(ScaledRexIcon);
                   button.setHorizontalTextPosition(SwingConstants.CENTER);
                   button.setVerticalTextPosition(SwingConstants.BOTTOM);
               }
               else
               {
                   button.setIcon(ScaledLexIcon);
                   button.setHorizontalTextPosition(SwingConstants.CENTER);
                   button.setVerticalTextPosition(SwingConstants.TOP);
               }
           }*/
       }

       Log.logger.exiting(this.getClass().toString(), "runParseExtendedDeviceInfoCommand");
   }
   
   /*
     * Remove Device Pairing:
     * Device1 IP ----------X----------> Device2 MAC
     *
     * Instruct device1 to unpair from device2
     */
    public boolean executeUnpairCommand(DeviceNode device1, String pairedMACAddress)
    {
        Log.logger.entering(this.getClass().toString(), "executeUnpairCommand");

        if (device1 != null)
        {
            String command = NetConfigCommand
                    + " forced "
                    + String.valueOf(device1.supportedProtocol)
                    + " "
                    + RemoveDevicePairing + " ";

            command += device1.ipAddress + " ";

            if (pairedMACAddress != null)
            {
                if (device1.supportedProtocol >= 2)
                {
                    command += pairedMACAddress;
                }

                Log.logger.info("Executing command: " + command);

                try
                {
                    Process proc = Runtime.getRuntime().exec(command);

                    BufferedReader reader = new BufferedReader(new InputStreamReader(proc
                            .getInputStream()));
                    String line = reader.readLine();

                    String allLines = line;

                    while (line != null)
                    {
                        line = reader.readLine();
                        allLines += line;
                    }

                    // check the output message and see if it is a success message
                    int successMessageIndex = allLines
                            .indexOf(DeviceUnpairingSuccessMessage);

                    // Success message not found
                    if (successMessageIndex == -1)
                    {
                        successMessageIndex = allLines
                                .indexOf(DeviceAlreadyUnpairedMessage);

                        if (successMessageIndex == -1)
                        {
                            Log.logger.severe(unpairingFailedMessage);
                            Log.logger.exiting(this.getClass().toString(), "executeUnpairCommand");
                            return false;
                        }
                        else
                        {
                            Log.logger.info(alreadyUnpairedMessage);
                        }
                    }
                    else
                    {
                        Log.logger.info(DeviceUnpairingSuccessMessage);
                    }
                }
                catch (IOException e)
                {
                    String errMsg = "Could not execute " + command;
                    Log.logger.severe(errMsg);
                    JOptionPane.showMessageDialog(PairingDemo.getFrame(),
                            errMsg,
                            "Command Execution Error",
                            JOptionPane.ERROR_MESSAGE);
                    Log.logger.exiting(this.getClass().toString(), "executeUnpairCommand");
                    return false;
                }
            }
            else
            {
                Log.logger.exiting(this.getClass().toString(), "executeUnpairCommand");
                return false;
            }
        }
        else
        {
            Log.logger.exiting(this.getClass().toString(), "executeUnpairCommand");
            return false;
        }

        Log.logger.exiting(this.getClass().toString(), "executeUnpairCommand");
        return true;
    }

    /*
     * Pair To Device:
     * Device1 IP ---------------------> Device2 MAC
     *
     * Instruct device1 to pair with device2
     */
    public Boolean executePairCommand(DeviceNode device1, DeviceNode device2)
    {
        Log.logger.entering(this.getClass().toString(), "executePairCommand");

        String command = NetConfigCommand
                + " forced "
                + String.valueOf(device1.supportedProtocol)
                + " "
                + PairToDeviceCommand + " ";

        command += device1.ipAddress + " " + device2.macAddress;

        Log.logger.info("Executing command: " + command);

        try
        {
            Process proc = Runtime.getRuntime().exec(command);

            BufferedReader reader = new BufferedReader(new InputStreamReader(proc
                    .getInputStream()));
            String line = reader.readLine();

            String allLines = line;

            while (line != null)
            {
                line = reader.readLine();
                allLines += line;
            }

            // check the output message and see if it is a success message
            int successMessageIndex = allLines
                    .indexOf(DevicePairingSuccessMessage);

            // Success message not found
            if (successMessageIndex == -1)
            {
                successMessageIndex = allLines
                        .indexOf(DeviceAlreadyPairedMessage);

                if(successMessageIndex == -1)
                {
                    Log.logger.severe(pairingFailedMessage);
                    Log.logger.exiting(this.getClass().toString(), "executePairCommand");
                    return false;
                }
                else
                {
                    Log.logger.severe(alreadyEstablishedMessage);
                    Log.logger.exiting(this.getClass().toString(), "executePairCommand");
                    return false;
                }
            }
            else
            {
                Log.logger.info(DevicePairingSuccessMessage);
            }
        }
        catch (IOException e)
        {
            String errMsg = "Could not execute " + command;
            JOptionPane.showMessageDialog(PairingDemo.getFrame(),
                    errMsg,
                    "Command Execution Error",
                    JOptionPane.ERROR_MESSAGE);
            Log.logger.exiting(this.getClass().toString(), "executePairCommand");
            return false;
        }

        Log.logger.exiting(this.getClass().toString(), "executePairCommand");
        return true;
    }
    
    
    /*
     * Pair To Device:
     * Device1 IP ---------------------> Device2 MAC
     *
     * Instruct device1 to pair with device2
     */
    public Boolean executeFilterCommand(DeviceNode device1, int filter_strategy)
    {
        Log.logger.entering(this.getClass().toString(), "executeFilterCommand");

        String command = NetConfigCommand
                + " forced "
                + String.valueOf(device1.supportedProtocol)
                + " "
                + RequestFilterCommand + " ";

        command += device1.ipAddress + " " + filter_strategy;
        
        Log.logger.info("Executing command: " + command);

        try
        {
            Process proc = Runtime.getRuntime().exec(command);

            BufferedReader reader = new BufferedReader(new InputStreamReader(proc
                    .getInputStream()));
            String line = reader.readLine();

            String allLines = line;

            while (line != null)
            {
                line = reader.readLine();
                allLines += line;
            }

            // check the output message and see if it is a success message
            int successMessageIndex = allLines
                    .indexOf(DeviceFilterSuccessMessage);

            // Success message not found
            if (successMessageIndex == -1)
            {
                Log.logger.severe(filterFailedMessage);
                Log.logger.exiting(this.getClass().toString(), "executeFilterCommand");
                return false;
            }
            else
            {
                Log.logger.info(DeviceFilterSuccessMessage);
            }
        }
        catch (IOException e)
        {
            String errMsg = "Could not execute " + command;
            JOptionPane.showMessageDialog(PairingDemo.getFrame(),
                    errMsg,
                    "Command Execution Error",
                    JOptionPane.ERROR_MESSAGE);
            Log.logger.exiting(this.getClass().toString(), "executeFilterCommand");
            return false;
        }

        Log.logger.exiting(this.getClass().toString(), "executeFilterCommand");
        return true;
    }
    
    
    /*
    * Extracts an array of the first X (where X is numberOfStrings) strings
    * found between firstPattern and secondPattern patterns in the input
    * string text
    */
   private String[] parseTokens(
           String text,
           String firstPattern,
           String secondPattern,
           int numberOfStrings)
   {
       Log.logger.entering(this.getClass().toString(), "parseTokens");

       String[] tokens = new String[numberOfStrings];

       String[] tempTokens = text.split(firstPattern);

       for (int stringIndex = 1; stringIndex <= numberOfStrings; stringIndex++)
       {
           String[] parsedTokens = tempTokens[stringIndex]
                   .split(secondPattern);

           tokens[stringIndex - 1] = parsedTokens[0];
       }

       Log.logger.exiting(this.getClass().toString(), "parseTokens");
       return tokens;
   }
}
