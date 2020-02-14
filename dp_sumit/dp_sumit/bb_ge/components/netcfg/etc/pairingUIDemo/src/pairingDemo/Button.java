package pairingDemo;

import java.awt.Graphics;
import java.awt.image.BufferedImage;

public class Button extends UIObject
{
    BufferedImage button_normal;
    BufferedImage button_highlight;
    BufferedImage button_greyed;
    boolean greyed = true;
    boolean highlighted = false;
    float highlightTime;
    
    public Button(String normal, String highlight, String grey)
    {
        button_normal = loadImage(normal);
        button_highlight = loadImage(highlight);
        button_greyed = loadImage(grey);
        
        width = button_normal.getWidth();
        height = button_normal.getHeight();
    }
    
    public void highlight()
    {
        highlightTime = (float)250;
        highlighted = true;
    }
    
    public void draw( Graphics g, float deltaTime )
    {
        BufferedImage buttonImage = null;
        if (greyed)
        {
            buttonImage = button_greyed;
            highlighted = false;
        }
        else if (highlighted)
        {
            highlightTime-=deltaTime;
            if (highlightTime<0)
                highlighted = false;
            buttonImage = button_highlight;
        }
        else
            buttonImage = button_normal;
            
        g.drawImage( buttonImage, (int)x, (int)y, null );
    }
}
