package pairingDemo;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import javax.swing.JOptionPane;
import pairingDemo.DeviceNode.ExtenderType;

public class DeviceCommunicationTask extends Thread
{
    private boolean quit = false;
    private DeviceInterface deviceInterface;
    private HashMap<String, DeviceNode> deviceList = new HashMap<String, DeviceNode>();
    private ArrayList<NodeCommand> nodeCommands = new ArrayList<NodeCommand>();
    public int testNodes = 0;
    public boolean commandWaiting = false;
    
    public static final int DEFAULT_PORT = 20104;
    private DatagramSocket socket;
    
    public DeviceCommunicationTask(DeviceInterface deviceInterface)
    {
        this.deviceInterface = deviceInterface;
        
        try
        {
            socket = new DatagramSocket(DEFAULT_PORT);
        }
        catch( Exception ex )
        {
            System.out.println("Problem creating socket on port: " + DEFAULT_PORT );
        }

    }
    
    public void addTestNodes( HashMap<String, DeviceNode> newDeviceList )
    {
        for( int i=0; i<6; i++ )
        {
            // Add test nodes until we reach the desired amount
            if (newDeviceList.size()>=testNodes)
                return;
            
            DeviceNode testNode = new DeviceNode();
            testNode.supportedProtocol = 2;
            
            if (i==0)
            {
                testNode.ipAddress = "1.2.3.1";
                testNode.macAddress = "00:00:00:00:00:01";
                testNode.lexOrRex = ExtenderType.LEX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:02");
            }
            else if (i==1)
            {
                testNode.ipAddress = "1.2.3.2";
                testNode.macAddress = "00:00:00:00:00:02";
                testNode.lexOrRex = ExtenderType.REX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:01");
            }
            else if (i==2)
            {
                testNode.ipAddress = "1.2.3.3";
                testNode.macAddress = "00:00:00:00:00:03";
                testNode.lexOrRex = ExtenderType.REX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:04");
            }
            else if (i==3)
            {
                testNode.ipAddress = "1.2.3.4";
                testNode.macAddress = "00:00:00:00:00:04";
                testNode.lexOrRex = ExtenderType.LEX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:03");
            }
            else if (i==4)
            {
                testNode.ipAddress = "1.2.3.5";
                testNode.macAddress = "00:00:00:00:00:05";
                testNode.lexOrRex = ExtenderType.REX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:06");
            }
            else if (i==5)
            {
                testNode.ipAddress = "1.2.3.6";
                testNode.macAddress = "00:00:00:00:00:06";
                testNode.lexOrRex = ExtenderType.LEX;
                testNode.pairedMACAddresses.add("00:00:00:00:00:05");
            }
            
            newDeviceList.put(testNode.macAddress, testNode);
        }
    }
    
    public void run()
    {
        while(!quit)
        {
            try
            {
                HashMap<String, DeviceNode> newDeviceList = new HashMap<String, DeviceNode>();

                // Request device info and load devices into new device list
                deviceInterface.runParseDeviceInfoCommand(newDeviceList);
                
                // Request extended device info for the new device list
                deviceInterface.runParseExtendedDeviceInfoCommand(newDeviceList);
                
                addTestNodes(newDeviceList);
                
                synchronized(this)
                {
                    // Look for stale nodes
                    Iterator<Map.Entry<String, DeviceNode>> iterator = deviceList.entrySet().iterator();
                    while( iterator.hasNext() )
                    {
                        Map.Entry<String, DeviceNode> deviceNodeEntry = iterator.next();
                        DeviceNode oldNode = (DeviceNode)deviceNodeEntry.getValue();
                        oldNode.missedPings++;
                        
                        if (oldNode.missedPings>3)
                        {
                            iterator.remove();
                        }
                    }
                    
                    // Merge lists
                    for(String deviceMacAddress : newDeviceList.keySet())
                    {
                        DeviceNode newNode = newDeviceList.get(deviceMacAddress);
                        deviceList.put(deviceMacAddress, newNode);
                    }
                    
                    
                }
                
                for (String deviceMacAddress : newDeviceList.keySet())
                {
                    DeviceNode node = newDeviceList.get(deviceMacAddress);
                    Log.logger.info("Node: "+deviceMacAddress+" - "+node.ipAddress+" type:"+node.lexOrRex);
                }
                
                Log.logger.info("Devices Found="+newDeviceList.size());
                
                commandWaiting = false;
                
                while(!nodeCommands.isEmpty())
                {
                    commandWaiting = true;
                    NodeCommand nodeCommand = nodeCommands.remove(0);
                    
                    Log.logger.info("Node Command: "+nodeCommand.command+" source MAC"+nodeCommand.source_mac+" target MAC"+nodeCommand.source_mac);
                    DeviceNode source_node = newDeviceList.get(nodeCommand.source_mac);
                    if (nodeCommand.command==NodeCommand.Command.pair)
                    {
                        DeviceNode target_node = newDeviceList.get(nodeCommand.target_mac);
                        if (source_node!=null && target_node!=null)
                        {
                            if (deviceInterface.executePairCommand(source_node,target_node))
                                source_node.pairedMACAddresses.add( target_node.macAddress );
                        }
                    }
                    else if (nodeCommand.command==NodeCommand.Command.unpair)
                    {
                        if (source_node!=null)
                        {
                            if (deviceInterface.executeUnpairCommand(source_node,nodeCommand.target_mac))
                                source_node.pairedMACAddresses.clear();
                        }
                    }
                    else if (nodeCommand.command==NodeCommand.Command.filter)
                    {
                        if (source_node!=null)
                            deviceInterface.executeFilterCommand(source_node,2);
                    }
                    else if (nodeCommand.command==NodeCommand.Command.unfilter)
                    {
                        if (source_node!=null)
                            deviceInterface.executeFilterCommand(source_node,0);
                    }
                }
                
                if (!commandWaiting)
                {
                    DatagramPacket packet = new DatagramPacket (new byte[1], 1);
                    socket.setSoTimeout(500);
                    socket.receive(packet);
                    JOptionPane.showMessageDialog(PairingDemo.getFrame(),
                                "Receive Hotkey message from "+packet.getAddress(),
                                "HotKey",
                                JOptionPane.WARNING_MESSAGE);
                }
                //System.out.println("Received from: " + packet.getAddress () + ":" + packet.getPort ());
            }
            catch (IOException e)
            {
                if (!e.toString().contains("java.net.SocketTimeoutException"))
                    e.printStackTrace();
            }
        }
    }
    
    public void done()
    {
        quit = true;
    }
    
    public synchronized HashMap<String, DeviceNode> getDeviceList()
    {
        return deviceList;
    }
    
    public synchronized void addNodeCommands( ArrayList<NodeCommand> nodeCommands )
    {
        for(NodeCommand nodeCommand : nodeCommands)
            this.nodeCommands.add(nodeCommand);
    }
    
    public synchronized boolean doneNodeCommands()
    {
        return this.nodeCommands.isEmpty() && !commandWaiting;
    }
}
