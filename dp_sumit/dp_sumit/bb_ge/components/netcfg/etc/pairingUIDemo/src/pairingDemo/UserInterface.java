package pairingDemo;

import com.jhlabs.awt.ShapeStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.CubicCurve2D;
import java.awt.geom.Ellipse2D;
import java.awt.image.BufferStrategy;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.imageio.ImageIO;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import pairingDemo.DeviceNode.ExtenderType;

public class UserInterface extends JFrame implements MouseListener, MouseMotionListener, ActionListener
{
    private ArrayList<NodeCommand> nodeCommands = new ArrayList<NodeCommand>();
    
    GraphicsDevice device;
    int framecount = 0;
    
    int width = 800;
    int height = 600;
    int page_width = width;
    int page_height = height;
    int old_page_width = width;
    int old_page_height = height;
    int left_spawn_x=-200;
    int right_spawn_x=width+200;
    int spawn_y = height/2;
    Insets insets;
    
    BufferedImage background;
    BufferedImage logo;
    BufferedImage usbLogo;
    BufferedImage footer;
    BufferedImage controlPanel;
    ArrayList<DisplayNode> displayNodes = new ArrayList<DisplayNode>();
    boolean show_unpair = false;
    Button pair = new Button("/resources/button_pair.png","/resources/button_pair_selected.png","/resources/button_pair_greyed.png");
    Button unpair = new Button("/resources/button_unpair.png","/resources/button_unpair_selected.png","/resources/button_unpair_selected.png");
    Button filter = new Button("/resources/button_filter.png","/resources/button_filter_selected.png","/resources/button_filter_greyed.png");
    Button unfilter = new Button("/resources/button_unfilter.png","/resources/button_unfilter_selected.png","/resources/button_unfilter_greyed.png");
    Button settings = new Button("/resources/button_settings.png","/resources/button_settings.png","/resources/button_settings.png");
    Font infoFont = new Font("Arial", Font.PLAIN, 12);
    
    JButton networkAddressButton;
    JButton mapMacButton;
    JButton restoreDefaultsButton;
    protected static LinkedList<MacToDeviceNames> macToDeviceList_ = new LinkedList<MacToDeviceNames>();
    static String SettingsFile = "." + File.separator + "Settings.xml";
            
    float footer_offset = 0;
    
    public UserInterface()
    {
        loadSettings();
        
        GraphicsEnvironment env = GraphicsEnvironment.getLocalGraphicsEnvironment();
        this.device = env.getDefaultScreenDevice();
        
        addMouseListener(this);
        addMouseMotionListener(this);
        
        setUndecorated(false);
        setResizable(true);
        this.setSize(width,height);
        setVisible(true);
        
        validate();
        this.createBufferStrategy(2);
        
        insets = this.getInsets();
        
        background = UIObject.loadImage("/resources/background-content-test.jpg");
        
        logo = UIObject.loadImageFromFile("logo.png");
        if (logo==null)
            logo = UIObject.loadImage("/resources/icron_logo.png");
        usbLogo = UIObject.loadImage("/resources/ExtremeUSB_logo_final.png");
        footer = UIObject.loadImage("/resources/footer-dots1.png");
        
        controlPanel = UIObject.loadImage("/resources/control_panel.png");
        unpair.greyed = false;
        displayNodes.add(new DisplayNode());
    }
    
    public synchronized void toggleFullScreen()
    {
        Window fsw = device.getFullScreenWindow();

        this.setVisible(false);
        this.dispose();

        boolean goFullScreen = fsw != this;

        setUndecorated(goFullScreen);
        setResizable(!goFullScreen);
        if (goFullScreen) {
            device.setFullScreenWindow(this);
            validate();
        } else {
            device.setFullScreenWindow(null);
            //pack();
            this.setSize(width,height);
            setVisible(true);
        }
        
        this.createBufferStrategy(2);
    }
    
    private void pulseNetworkNode()
    {
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.network_switch)
            {
                node.pulsing=true;
                node.pulseTime=0;
            }
        }
    }
    
    private void clearSelected()
    {
        for(DisplayNode node : displayNodes)
            node.cancelSelection();
        
        pair.greyed = true;
        unpair.greyed = true;
        filter.greyed = true;
        unfilter.greyed = true;
    }
    
    private void showFilterWarning()
    {
        JOptionPane.showMessageDialog(this,
            "Please unplug and plug in the device for filter changes to take effect.",
            "Action Required",
            JOptionPane.WARNING_MESSAGE);
    }
    
    public void mousePressed(MouseEvent e)
    {
        // See if we clicked on any of the nodes
        int x=e.getX();
        int y=e.getY();
        
        // Handle Right Click
        if (SwingUtilities.isRightMouseButton(e))
        {
            return;
        }
        
        // Left Click
        for(DisplayNode node : displayNodes)
        {
            if (node.isInside(x,y) && node.isOnline)
            {
                node.toggleSelection();
                
                if (node.paired)
                {
                    // Unselect all other nodes
                    for(DisplayNode checkNode : displayNodes)
                    {
                        if (node!=checkNode && node.pairedNode!=checkNode)
                            checkNode.cancelSelection();
                    }
                }
                else
                {
                    // Unselect are same type nodes
                    for(DisplayNode checkNode : displayNodes)
                    {
                        if (node!=checkNode && node.type==checkNode.type)
                            checkNode.cancelSelection();
                    }
                }
            }
        }
        
        // See which nodes types are selected.
        DisplayNode rex_selected = null;
        DisplayNode lex_selected = null;
        show_unpair = false;
        filter.greyed = true;
        unfilter.greyed = true;
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.lex && node.selected)
            {
                if (node.paired)
                    show_unpair = true;
                lex_selected = node;
            }
            else if (node.type==DisplayNode.Type.rex && node.selected)
            {
                if (node.paired)
                    show_unpair = true;
                rex_selected = node;
            }
        }
        
        if (rex_selected!=null || lex_selected!=null)
        {
            filter.greyed = false;
            unfilter.greyed = false;
        }
        
        if (rex_selected!=null && lex_selected!=null)
            pair.greyed = false;
        else
            pair.greyed = true;
        
        if (!show_unpair && pair.isInside(x, y) && !pair.greyed)
        {
            pair.highlight();
            rex_selected.pairNode(lex_selected);
            rex_selected.cancelSelection();
            synchronized(this)
            {    
                nodeCommands.add( NodeCommand.createPairCommand(rex_selected.macAddress,lex_selected.macAddress) );
                nodeCommands.add( NodeCommand.createPairCommand(lex_selected.macAddress,rex_selected.macAddress) );
            }
            clearSelected();
            pulseNetworkNode();
        }
        else if (show_unpair && unpair.isInside(x, y))
        {
            unpair.highlight();
            for(DisplayNode node : displayNodes)
            {
                if (node.selected)
                {
                    synchronized(this)
                    {    
                        // Although we don't support one to many pairing, devices could already be in that state
                        if (rex_selected!=null)
                            for( String mac_pair : rex_selected.pairedMACAddresses )
                                nodeCommands.add( NodeCommand.createUnpairCommand(rex_selected.macAddress,mac_pair) );
                        if (lex_selected!=null)
                            for( String mac_pair : lex_selected.pairedMACAddresses )
                                nodeCommands.add( NodeCommand.createUnpairCommand(lex_selected.macAddress,mac_pair) );
                    }
                    pulseNetworkNode();
                    node.cancelSelection();
                    node.unpairNode();
                }
            }
            show_unpair = false;
            clearSelected();
        }
        else if (!filter.greyed && filter.isInside(x,y))
        {
            filter.highlight();
            for(DisplayNode node : displayNodes)
                if (node.selected)
                    nodeCommands.add( NodeCommand.createFilterCommand(node.macAddress) );
            pulseNetworkNode();
            //showFilterWarning();
            clearSelected();
        }
        else if (!unfilter.greyed && unfilter.isInside(x,y))
        {
            unfilter.highlight();
            for(DisplayNode node : displayNodes)
                if (node.selected)
                    nodeCommands.add( NodeCommand.createUnfilterCommand(node.macAddress) );
            pulseNetworkNode();
            //showFilterWarning();
            clearSelected();
        }
        else if (settings.isInside(x,y))
        {
            createSettingPopUp();
        }
    }  
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public void mouseClicked(MouseEvent e) {}
        
    public void mouseDragged(MouseEvent e) {}
    public void mouseMoved(MouseEvent e) 
    {
        int x=e.getX();
        int y=e.getY();
        
        for(DisplayNode node : displayNodes)
        {
            if (node.isInside(x,y))
            {
                node.showPopUp = true;
            }
            else
            {
                node.showPopUp = false;
            }
        }
    }
    
    public void actionPerformed(ActionEvent e)
    {
        if ("setNetworkAddress".equals(e.getActionCommand()))
        {
            Window win = SwingUtilities.getWindowAncestor(networkAddressButton);
            win.setVisible(false);
            
            String newNetworkAddress = (String) JOptionPane.showInputDialog(this,
                    "Enter Network Broadcast Address:",
                    "Set Network Address",
                    JOptionPane.PLAIN_MESSAGE,
                    null,
                    null,
                    DeviceInterface.BroadcastAddress);

            if (validateNetworkAddress(newNetworkAddress))
            {
                DeviceInterface.BroadcastAddress = newNetworkAddress;
                writeSettingsToFile();
            }
            else
            {
                JOptionPane.showMessageDialog(this,
                        "Please Enter a valid Network Address",
                        "Set Network Address",
                        JOptionPane.ERROR_MESSAGE);
            }
        }
        else if ("mapMACToDeviceMap".equals(e.getActionCommand()))
        {
            Window win = SwingUtilities.getWindowAncestor(mapMacButton);
            win.setVisible(false);
            
            createMacMappingPopup();
        }
        else if ("setDefaults".equals(e.getActionCommand()))
        {
            Window win = SwingUtilities.getWindowAncestor(restoreDefaultsButton);
            win.setVisible(false); 
            
            int reply = JOptionPane.showConfirmDialog(null, "This will delete all MAC Mapped Device names and reset the Broadcast Network Address.  Are you sure you want to continue?", "Are you sure?", JOptionPane.YES_NO_OPTION);
            
            if (reply==JOptionPane.YES_OPTION)
            {
                DeviceInterface.BroadcastAddress = "";
                macToDeviceList_.clear();

                writeSettingsToFile();
                DeviceInterface.autoDetectBroadcastIP();
            }
        }
    }
    
    private boolean validateNetworkAddress(String ip)
    {
        Log.logger.entering(this.getClass().toString(), "validateNetworkAddress");

        String PATTERN = "^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
                + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";
        Pattern pattern = Pattern.compile(PATTERN);
        Matcher matcher = pattern.matcher(ip);

        Log.logger.exiting(this.getClass().toString(), "validateNetworkAddress");
        return matcher.matches();
    }
    
    /*
     * Write MAC to Device Names XML file
     *
     * Called from Event Dispatch Thread
     */
    private void writeSettingsToFile()
    {
        Log.logger.entering(this.getClass().toString(), "writeSettingsToFile");

        try
        {
            DocumentBuilderFactory fact = DocumentBuilderFactory.newInstance();
            DocumentBuilder parser = fact.newDocumentBuilder();
            Document doc = parser.newDocument();

            Node root = doc.createElement("settings");
            doc.appendChild(root);

            Node bcAddr = doc.createElement("broadcastAddress");
            bcAddr.appendChild(doc.createTextNode(DeviceInterface.BroadcastAddress));
            root.appendChild(bcAddr);

            Node units = doc.createElement("units");
            root.appendChild(units);

            for (MacToDeviceNames device : macToDeviceList_)
            {
                Node unitElement = doc.createElement("unit");

                Node macElement = doc.createElement("mac");
                macElement.appendChild(doc.createTextNode(device.macAddress));
                unitElement.appendChild(macElement);

                Node deviceElement = doc.createElement("name");
                deviceElement
                        .appendChild(doc.createTextNode(device.deviceName));
                unitElement.appendChild(deviceElement);

                units.appendChild(unitElement);
            }

            // set up a transformer
            TransformerFactory transfac = TransformerFactory.newInstance();
            Transformer trans = transfac.newTransformer();
            trans.setOutputProperty(OutputKeys.INDENT, "yes");
            trans.setOutputProperty("{http://xml.apache.org/xslt}indent-amount",
                    "2");

            Source source = new DOMSource(doc);
            trans.transform(source,
                    new StreamResult(new File(SettingsFile)));
        }
        catch (Exception ex)
        {
            System.err.println("+============================+");
            System.err.println("|        XML Error           |");
            System.err.println("+============================+");
            System.err.println(ex.getClass());
            System.err.println(ex.getMessage());
            System.err.println("+============================+");
        }

        Log.logger.exiting(this.getClass().toString(), "writeSettingsToFile");
    }
    
    /*
     * Parse XML file that contains MAC Address to Device Name mapping
     */
    private static void loadSettings()
    {
        Log.logger.entering("DeviceDiscoveryTask", "loadSettings");

        try
        {
            File xmlFile = new File(SettingsFile);
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory
                    .newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(xmlFile);
            doc.getDocumentElement().normalize();

            DeviceInterface.BroadcastAddress = doc.getElementsByTagName("broadcastAddress").item(0).getFirstChild().getNodeValue();

            NodeList nList = doc.getElementsByTagName("units").item(0).getChildNodes();
            for (int i = 0; i < nList.getLength(); i++)
            {
                Node node = nList.item(i);
                if (node.getNodeType() == Node.ELEMENT_NODE)
                {
                    Element element = (Element) node;
                    String mac = getTagValue("mac", element);
                    String name = getTagValue("name", element);
                    macToDeviceList_.add(new MacToDeviceNames(mac, name));
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        Log.logger.exiting("DeviceDiscoveryTask", "loadSettings");
    }
    
    /*
     * Parse XML tag
     *
     * Called from Event Dispatch Thread
     */
    private static String getTagValue(String tag, Element element)
    {
        NodeList nList = element.getElementsByTagName(tag).item(0)
                .getChildNodes();

        Node value = nList.item(0);
        return value.getNodeValue();
    }
    
    protected static BufferedImage loadImage( String file )
    {
        BufferedImage image = null;
        try
        {
            InputStream stream = PairingDemo.class.getResourceAsStream(file);
            image = ImageIO.read(stream);
        }
        catch (IOException e )
        {
            throw new RuntimeException ("Unable to load Image: "+e);
        }
        return image;
    }
    
    public void updateNodes(HashMap<String, DeviceNode> deviceList, boolean doneCommands )
    {
        // Mark all nodes as offline until they're found in the list
        for(DisplayNode node : displayNodes)
        {
            node.isOnline = false;
            node.paired = false;
            node.pairedNode = null;
            if (doneCommands)
                node.pulsing = false;
        }
        
        for (String deviceMacAddress : deviceList.keySet())
        {
            DeviceNode deviceNode = deviceList.get(deviceMacAddress);
            
            DisplayNode displayNode = null;
            for(DisplayNode node : displayNodes)
            {
                if (node.macAddress.equals(deviceMacAddress))
                {
                    displayNode = node;
                    break;
                }
            }
            
            if (displayNode==null)
            {
                // Create a new display node for this device
                displayNode = new DisplayNode();
                displayNode.macAddress = deviceMacAddress;
                
                displayNode.y = spawn_y;
                if (deviceNode.lexOrRex==ExtenderType.LEX)
                {
                    displayNode.x = left_spawn_x;
                    displayNode.type=DisplayNode.Type.lex;
                }
                else
                {
                    displayNode.x = right_spawn_x;
                    displayNode.type=DisplayNode.Type.rex;
                }
                displayNodes.add(displayNode);
            }
            
            displayNode.isOnline = true;
            displayNode.ipAddress = deviceNode.ipAddress;
            
            displayNode.pairedMACAddresses.clear();
            displayNode.paired = false;
            displayNode.pairedNode = null;
            for( String mac_pair : deviceNode.pairedMACAddresses )
            {
                displayNode.pairedMACAddresses.add( mac_pair );
                displayNode.paired = true;
            }
            
            for (MacToDeviceNames deviceNames : macToDeviceList_)
            {
                if (deviceNames.macAddress.equals(displayNode.macAddress))
                    displayNode.name = deviceNames.deviceName;
            }
        }
        
        // Link Display Nodes based on device pairs
        for(DisplayNode node : displayNodes)
        {
            // For now we don't allow 1 to many pairing
            if (node.pairedMACAddresses.size()>0 && node.pairedNode==null)
            {
                for(DisplayNode searchNode : displayNodes)
                {
                    if (searchNode.macAddress.equals( node.pairedMACAddresses.get(0) ) && searchNode.pairedNode==null)
                    {
                        // Don't doubly link, singly linked pairs.
                        if (searchNode.paired==true)
                        {
                            node.pairedNode = searchNode;
                            searchNode.pairedNode = node;
                            searchNode.paired = true;
                        }
                    }
                }
            }
        }
        
        // Place node Targets
        int lexPlaced = 0;
        int rexPlaced = 0;
        for(DisplayNode node : displayNodes)
        {
            if (node.isOnline)
            {
                // Percentage of screen
                double distance_from_sides = 0.026;
                double distance_from_top = 0.030;
                double vertical_spacing = 0.27;
                
                if (node.type==DisplayNode.Type.lex)
                {
                    if (lexPlaced<3)
                    {
                        node.target_x = insets.left+(int)(page_width*distance_from_sides);
                        node.target_y = insets.top+lexPlaced*(int)(page_height*vertical_spacing)+(int)(page_height*distance_from_top);
                    }
                    else
                    {
                        // Extra nodes, leave them on the sides
                        node.target_x = left_spawn_x;
                    }
                    lexPlaced++;
                }
                else
                {
                    if (rexPlaced<3)
                    {
                        node.target_x = insets.left+page_width-(int)(page_width*distance_from_sides)-node.width;
                        node.target_y = insets.top+rexPlaced*(int)(page_height*vertical_spacing)+(int)(page_height*distance_from_top);
                    }
                    else
                    {
                        // Extra nodes, leave them on the sides
                        node.target_x = right_spawn_x;
                    }
                    rexPlaced++;
                }
            }
            else
            {
                // Node is offline, send it out.
                if (node.type==DisplayNode.Type.lex)
                {
                    node.target_x = left_spawn_x;
                    node.target_y = spawn_y;
                }
                else if (node.type==DisplayNode.Type.rex)
                {
                    node.target_x = right_spawn_x;
                    node.target_y = spawn_y;
                }
                else
                {
                    // Network switch Node
                    node.target_x = insets.left+page_width/2-node.width/2;
                    node.target_y = insets.top+(int)(page_height*0.4)-node.height/2;
                }
            }
        }
    }
    
    private void resize( int old_width, int old_height, int new_width, int new_height )
    {
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.network_switch)
                node.x = (((float)node.x+node.width/2)*(float)new_width/(float)old_width)-node.width/2;
            else if (node.type==DisplayNode.Type.rex)
                node.x = (((float)node.x+node.width)*(float)new_width/(float)old_width)-node.width;
            else
                node.x = (float)node.x*(float)new_width/(float)old_width;
            
            if (node.type==DisplayNode.Type.network_switch)
                node.y = (((float)node.y+node.height/2)*(float)new_height/(float)old_height)-node.height/2;
            else
                node.y = (float)node.y*(float)new_height/(float)old_height;
        }
    }
    
    public synchronized void draw(float deltaTime)
    {
        BufferStrategy bufferStrategy = this.getBufferStrategy();
        Graphics g = bufferStrategy.getDrawGraphics();
        
        insets = this.getInsets();
        
        Rectangle bounds = this.getBounds();
        page_width = bounds.width - insets.right - insets.left;
        page_height = bounds.height - insets.top - insets.bottom;
        
        if (page_width!=old_page_width || page_height!=old_page_height)
        {
            resize(old_page_width,old_page_height,page_width,page_height);
        }
        old_page_width=page_width;
        old_page_height=page_height;
        
        float scale = (float)page_width/(float)background.getWidth();
        int background_height = (int)(background.getHeight()*scale);
        
        // Stretch background
        g.drawImage(background, 
                insets.left,insets.top,insets.left+page_width, insets.top+background_height, 
                0,0,background.getWidth(),background.getHeight(), 
                null);
        
        g.setColor(Color.white);
        g.fillRect(0,background_height,bounds.width,bounds.height-background_height);
        
        int footer_width = (int)(footer.getWidth()*scale);
        int footer_height = (int)(footer.getHeight()*scale);
        int footer_padding = 25;
        
        g.drawImage(logo, insets.left+25, insets.top+page_height-logo.getHeight()-25, null);
        g.drawImage(usbLogo, insets.left+page_width-usbLogo.getWidth()-25, insets.top+page_height-usbLogo.getHeight()-25, null);
        
        footer_offset += deltaTime/18;
        if (footer_offset>footer.getWidth()/2)
            footer_offset -= footer.getWidth()/2;
        
        g.drawImage(footer, 
                insets.left,page_height-footer_height-footer_padding,insets.left+footer_width,page_height-footer_padding, 
                (int)footer_offset,0,footer.getWidth()+(int)footer_offset,footer.getHeight(), 
                null);
        
        int control_panel_top = insets.top + page_height - controlPanel.getHeight() - 10;
        
        
        // Draw curves
        g.setColor(Color.gray);
        CubicCurve2D c;
        
        // Find the network switch node position
        int switch_x = 0;
        int switch_y = 0;
        int switch_height = 0;
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.network_switch)
            {
                switch_x = (int)node.x+node.width/2;
                switch_y = (int)node.y+node.height/2;
                switch_height = node.height;
            }
        }
        
        // Draw line from control panel to network switch node
        //int middle = insets.left+page_width/2;
        //((Graphics2D)g).setStroke(new BasicStroke(8f));
        //c = new CubicCurve2D.Double(middle,control_panel_top,middle,control_panel_top-30,middle,control_panel_top-30,switch_x,switch_y+switch_height/2);
        //((Graphics2D)g).draw(c);
        
        // Draw paired node curves
        ((Graphics2D)g).setStroke(
            new ShapeStroke(
                new Shape[] {
                    new Rectangle(0, 0, 5, 5),
                    new Ellipse2D.Float(0, 0, 5, 5)
                },
                7.5f
            )
	);
        int line_count = 0;
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.lex && node.paired && node.isOnline && node.pairedNode!=null && node.pairedNode.isOnline)
            {
                if (line_count==0)
                    g.setColor(Color.red);
                else if (line_count==1)
                    g.setColor(Color.blue);
                else
                    g.setColor(Color.yellow);
                line_count++;
                
                int source_x = (int)node.x+node.width/2;
                int source_y = (int)node.y+node.height/2;
                int dest_x = (int)node.pairedNode.x+node.width/2;
                int dest_y = (int)node.pairedNode.y+node.height/2;
                
                c = new CubicCurve2D.Double(source_x,source_y,switch_x,switch_y,switch_x,switch_y,dest_x,dest_y);
                ((Graphics2D)g).draw(c);
            }
        }
        g.setColor(Color.black);
        
        // Draw Nodes
        for(DisplayNode node : displayNodes)
        {
            if (node.type==DisplayNode.Type.network_switch)
                node.target_x = insets.left + page_width/2 - node.width/2;
            node.draw( g, deltaTime );
        }
        
        g.setFont( infoFont );
        g.setColor(Color.black);
        // Draw tool tip info
        for(DisplayNode node : displayNodes)
        {
            if (node.showPopUp && node.type!=DisplayNode.Type.network_switch)
            {
                int left = insets.left+page_width/2-controlPanel.getWidth()/2+10;
                String nodeType = "Unknown";
                if (node.type==DisplayNode.Type.lex)
                    nodeType = "LEX";
                else if (node.type==DisplayNode.Type.rex)
                    nodeType = "REX";
                g.drawString( "Device Type: "+nodeType, left, control_panel_top-56 );
                g.drawString( "IP Address: "+node.ipAddress, left, control_panel_top-42 );
                g.drawString( "MAC Address: "+node.macAddress, left, control_panel_top-28 );
                
                
                String pairedAddresses = "";
                for( String macAddress : node.pairedMACAddresses )
                {
                    if (pairedAddresses.length()>0)
                        pairedAddresses += ",";
                    pairedAddresses += macAddress;
                }
                g.drawString( "Paired MAC Addresses: "+pairedAddresses, left, control_panel_top-14 );
            }
        }
        
        // Draw Control Panel
        g.drawImage(controlPanel, insets.left+page_width/2-controlPanel.getWidth()/2, control_panel_top, null);
        
        float control_panel_button_start_height = 60;
        float control_panel_button_space_height = 80;
        
        if (show_unpair)
        {
            unpair.x = insets.left+page_width/2-unpair.width-10;
            unpair.y = control_panel_top+control_panel_button_start_height;
            unpair.draw(g, deltaTime);
        }
        else
        {
            pair.x = insets.left+page_width/2-pair.width-10;
            pair.y = control_panel_top+control_panel_button_start_height;
            pair.draw(g, deltaTime);
        }
        
        settings.x = insets.left+page_width/2-pair.width-10;
        settings.y = control_panel_top+pair.height+control_panel_button_space_height;
        settings.draw(g, deltaTime);
        
        filter.x = insets.left+page_width/2+10;
        filter.y = control_panel_top+control_panel_button_start_height;
        filter.draw(g, deltaTime);
        
        unfilter.x = insets.left+page_width/2+10;
        unfilter.y = control_panel_top+filter.height+control_panel_button_space_height;
        unfilter.draw(g, deltaTime);
        
        bufferStrategy.show();
        g.dispose();
    }
    
    public synchronized ArrayList<NodeCommand> getNodeCommandList()
    {
        // Clone the list
        ArrayList<NodeCommand> nodeCommandList = new ArrayList<NodeCommand>();
        for( NodeCommand nodeCommand : nodeCommands)
        {
            nodeCommandList.add(nodeCommand);
        }
        
        // Empty original list
        nodeCommands.clear();
        
        return nodeCommandList;
    }
    
    private void createSettingPopUp()
    {
        mapMacButton = new JButton("Map MAC to Device Names");
        mapMacButton.setActionCommand("mapMACToDeviceMap");
        mapMacButton.addActionListener(this);
        networkAddressButton = new JButton("Set Network Address");
        networkAddressButton.setActionCommand("setNetworkAddress");
        networkAddressButton.addActionListener(this);
        restoreDefaultsButton = new JButton("Restore Defaults");
        restoreDefaultsButton.setActionCommand("setDefaults");
        restoreDefaultsButton.addActionListener(this);
        
        JComponent[] components = new JComponent[]
        {
            mapMacButton,
            networkAddressButton,
            restoreDefaultsButton
        };
        
        JOptionPane.showMessageDialog(this,
            components,
            "Settings",
            JOptionPane.PLAIN_MESSAGE);
    }
    
    private void createMacMappingPopup()
    {
        Object[][] data = new Object[8][8];

        String[] columnNames =
        { "Device Name", "MAC Address" };
        
        int index = 0;
                
        for (MacToDeviceNames device : macToDeviceList_)
        {
            data[index][0] = device.deviceName;
            data[index][1] = device.macAddress;
            index++;
        }

        JTable table = new JTable(data, columnNames);
        table.putClientProperty("terminateEditOnFocusLost",
                Boolean.TRUE);

        JComponent[] tableInputs_ = new JComponent[]
        { table.getTableHeader(), table };
        
        JOptionPane.showMessageDialog(this,
                tableInputs_,
                "Setup Device Names and MAC Addresses",
                JOptionPane.PLAIN_MESSAGE);

        JTable result_table = ((JTable) tableInputs_[1]);

        int numberFilledEntries = 0;

        // Update the list
        for (int tableRow = 0; tableRow < table.getRowCount(); tableRow++)
        {
            String newMacAddress = (String) result_table.getValueAt(tableRow, 1);
            String newDeviceName = (String) result_table.getValueAt(tableRow, 0);

            if (newMacAddress != null && newDeviceName != null
                && !newMacAddress.equals("") && !newDeviceName.equals(""))
            {
                // Add new entries
                if (numberFilledEntries >= macToDeviceList_.size())
                {
                    macToDeviceList_.add(new MacToDeviceNames(newMacAddress,newDeviceName));
                }
                else
                {
                    MacToDeviceNames device = macToDeviceList_.get(numberFilledEntries);

                    String oldMacAddress = device.macAddress;
                    String oldDeviceName = device.deviceName;

                    if (!oldDeviceName.equals(newDeviceName))
                    {
                        device.deviceName = newDeviceName;
                    }

                    if (!oldMacAddress.equals(newMacAddress))
                    {
                        device.macAddress = newMacAddress;
                    }
                }
                numberFilledEntries++;
            }
        }
        // Add extra entries that were removed
        if (macToDeviceList_.size() > numberFilledEntries)
        {
            while (macToDeviceList_.size() != numberFilledEntries)
            {
                macToDeviceList_.remove(numberFilledEntries);
            }
        }
        
        // Write XML file
        writeSettingsToFile();
    }
    
    
}
