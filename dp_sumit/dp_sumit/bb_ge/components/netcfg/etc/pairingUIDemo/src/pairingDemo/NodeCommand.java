package pairingDemo;

public class NodeCommand 
{
    public enum Command
    {
        pair,
        unpair,
        filter,
        unfilter
    }
    
    Command command;
    String source_mac;
    String target_mac;
    
    private static NodeCommand makeCommand( Command command, String source_mac, String target_mac )
    {
        NodeCommand nodeCommand = new NodeCommand();
        nodeCommand.command = command;
        nodeCommand.source_mac = source_mac;
        nodeCommand.target_mac = target_mac;
        return nodeCommand;
    }
    
    public static NodeCommand createPairCommand( String source_mac, String target_mac )
    {
        return makeCommand(NodeCommand.Command.pair, source_mac, target_mac);
    }
    
    public static NodeCommand createUnpairCommand( String source_mac, String target_mac )
    {
        return makeCommand(NodeCommand.Command.unpair, source_mac, target_mac);
    }
    
    public static NodeCommand createFilterCommand( String source_mac )
    {
        return makeCommand(NodeCommand.Command.filter, source_mac, null);
    }
    
    public static NodeCommand createUnfilterCommand( String source_mac )
    {
        return makeCommand(NodeCommand.Command.unfilter, source_mac, null);
    }
}
