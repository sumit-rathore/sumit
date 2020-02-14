package pairingDemo;

import java.awt.image.BufferedImage;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import javax.imageio.ImageIO;

public class UIObject
{
    float x;
    float y;
    int width;
    int height;
    
    public boolean isInside(int mouse_x, int mouse_y)
    {
        // boundary test
        if (x<=mouse_x && x+width>mouse_x && y<=mouse_y && y+height>mouse_y)
            return true;
        
        return false;
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
    
    protected static BufferedImage loadImageFromFile( String file )
    {
        BufferedImage image = null;
        try
        {
            InputStream stream = new FileInputStream(file);
            image = ImageIO.read(stream);
        }
        catch (IOException e )
        {
            return null;
        }
        return image;
    }
}
