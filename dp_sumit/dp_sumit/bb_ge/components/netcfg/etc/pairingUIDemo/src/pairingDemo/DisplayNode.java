/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pairingDemo;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.font.FontRenderContext;
import java.awt.image.BufferedImage;
import java.util.ArrayList;

/**
 *
 * @author Eddy
 */
public class DisplayNode extends UIObject
{
    public boolean isOnline = false;
    public String macAddress = "";
    public String ipAddress = "";
    public String name = "";
    public ArrayList<String> pairedMACAddresses = new ArrayList<String>();
    public boolean selected = false;
    public boolean paired = false;
    public DisplayNode pairedNode = null;
    public boolean showPopUp = false;
    public boolean pulsing = true;
    float pulseTime = 0;
    
    public enum Type
    {
        network_switch,
        lex,
        rex
    }
    
    public Type type=Type.network_switch;
    
    float vx=0;
    float vy=0;
    int target_x;
    int target_y;
    float max_speed = (float)0.20;
    
    BufferedImage networkSwitch;
    BufferedImage selectedComputerUnpaired;
    BufferedImage unselectedComputerUnpaired;
    BufferedImage selectedComputerPaired;
    BufferedImage unselectedComputerPaired;
    BufferedImage selectedUSBUnpaired;
    BufferedImage unselectedUSBUnpaired;
    BufferedImage selectedUSBPaired;
    BufferedImage unselectedUSBPaired;
    Font defaultFont = new Font("Arial", Font.PLAIN, 16);
    
    public DisplayNode()
    {
        x=325;
        y=190;
        target_x = (int)x;
        target_y = (int)y;
        
        networkSwitch = loadImage("/resources/network_switch.png");
        unselectedComputerUnpaired = loadImage("/resources/unselected_computer_unpaired.png");
        selectedComputerUnpaired = loadImage("/resources/selected_computer_unpaired.png");
        unselectedComputerPaired = loadImage("/resources/unselected_computer_paired.png");
        selectedComputerPaired = loadImage("/resources/selected_computer_paired.png");
        unselectedUSBUnpaired = loadImage("/resources/unselected_usb_unpaired.png");
        selectedUSBUnpaired = loadImage("/resources/selected_usb_unpaired.png");
        unselectedUSBPaired = loadImage("/resources/unselected_usb_paired.png");
        selectedUSBPaired = loadImage("/resources/selected_usb_paired.png");
        
        width = unselectedComputerUnpaired.getWidth();
        height = unselectedComputerUnpaired.getHeight();
    }
    
    public void draw( Graphics g, float deltaTime )
    {
        vx += (target_x-x)/100000*deltaTime;
        vx += (Math.random()-0.5)/5000*deltaTime;
        vx -= (vx/1000) * deltaTime;
        
        vy += (target_y-y)/100000*deltaTime;
        vy += (Math.random()-0.5)/5000*deltaTime;
        vy -= (vy/1000) * deltaTime;
        
        // Speed limit
        float speed = (float)Math.sqrt( vx*vx+vy*vy );
        if (speed>max_speed)
        {
            vx = vx/(float)(speed/max_speed);
            vy = vy/(float)(speed/max_speed);
        }
        
        x += vx*deltaTime;
        y += vy*deltaTime;
        
        BufferedImage nodeImage = null;
        if (type==Type.lex)
        {
            if (paired)
            {
                if (selected)
                    nodeImage = selectedComputerPaired;
                else
                    nodeImage = unselectedComputerPaired;
            }
            else
            {
                if (selected)
                    nodeImage = selectedComputerUnpaired;
                else
                    nodeImage = unselectedComputerUnpaired;
            }
        }
        else if (type==Type.rex)
        {
            if (paired)
            {
                if (selected)
                    nodeImage = selectedUSBPaired;
                else
                    nodeImage = unselectedUSBPaired;
            }
            else
            {
                if (selected)
                    nodeImage = selectedUSBUnpaired;
                else
                    nodeImage = unselectedUSBUnpaired;
            }
        }
        else
            nodeImage = networkSwitch;
            
        width = nodeImage.getWidth();
        height = nodeImage.getHeight();
        
        pulseTime += deltaTime/250;
        int scale = (int)(Math.sin(pulseTime)*5);
        
        if (!pulsing)
            g.drawImage(nodeImage, (int)x, (int)y, null);
        else
            g.drawImage(nodeImage, (int)x+scale, (int)y+scale, width-2*scale, height-2*scale, null);
        
        g.setFont( defaultFont );
        g.setColor(Color.black);
        if (name.length()>0)
        {
            FontRenderContext context = ((Graphics2D)g).getFontRenderContext();
            int textwidth = (int)(defaultFont.getStringBounds(name, context).getWidth());
            g.drawString( name, (int)x+width/2-textwidth/2, (int)y+140 );
        }
        
        /*
        if (showPopUp && type!=Type.network_switch)
        {
            
            g.drawString( "MAC:"+macAddress, (int)x+10, (int)y+83 );
            g.drawString( "IP:"+ipAddress, (int)x+25, (int)y+98 );
        }*/
    }
    
    public void pairNode( DisplayNode pairNode )
    {
        pairedNode = pairNode;
        paired = true;
        pairedNode.pairedNode = this;
        pairedNode.paired = true;
    }
    
    public void unpairNode()
    {
        if (pairedNode!=null)
        {
            pairedNode.pairedNode = null;
            pairedNode.paired = false;
        }
        pairedNode = null;
        paired = false;
    }
    
    public void cancelSelection()
    {
        selected = false;
        
        if (pairedNode!=null)
        {
            pairedNode.selected = selected;
        }
    }
    
    public void toggleSelection()
    {
        selected = !this.selected;
                
        if (pairedNode!=null)
        {
            pairedNode.selected = selected;
        }
    }
}
