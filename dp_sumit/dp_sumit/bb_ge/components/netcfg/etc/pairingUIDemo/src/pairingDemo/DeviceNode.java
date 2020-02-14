package pairingDemo;


import java.util.LinkedList;

public class DeviceNode
{
    // Device related members
    public Boolean selectState = false;
    public String macAddress;
    public String ipAddress;

    // For LEX, it contains a list of MAC Address of REXs with which it is
    // paired
    // For REX, the one LEX with which it is paired
    public LinkedList<String> pairedMACAddresses = new LinkedList<String>();

    public ExtenderType lexOrRex;

    public Integer supportedProtocol;
    
    public int missedPings = 0;
    
    public enum ExtenderType
    {
        LEX("LEX"),
        REX("REX");

        private String devString;

        ExtenderType(String devString)
        {
            this.devString = devString;
        }

        public String getString()
        {
            return this.devString;
        }
    }
}
