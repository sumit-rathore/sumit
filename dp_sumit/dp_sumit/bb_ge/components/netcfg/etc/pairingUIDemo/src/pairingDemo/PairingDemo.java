/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pairingDemo;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.logging.Level;
import javax.swing.JFrame;

/**
 *
 * @author Eddy
 */
public class PairingDemo implements KeyListener, WindowListener
{   
    boolean done = false;
    DeviceCommunicationTask deviceCommunicationTask = null;
    private static PairingDemo instance;
    private UserInterface ui;
    
    public PairingDemo()
    {
    }
    
    public void run()
    {
        Log.logger.setLevel(Level.INFO);
        
        DeviceInterface deviceInterface = new DeviceInterface();
        
        ui = new UserInterface();
        ui.addKeyListener(this);
        ui.addWindowListener(this);
        
        deviceCommunicationTask = new DeviceCommunicationTask(deviceInterface);
        deviceCommunicationTask.start();
        
        long oldTime = System.currentTimeMillis();
        long nextTime = oldTime+(long)16.667;
        
        while(!done)
        {
            // Get the device list first in case the the thread is locked and it takes a while.
            HashMap<String, DeviceNode> deviceList = deviceCommunicationTask.getDeviceList();
            
            // Pass device commands from UI to communication task
            ArrayList<NodeCommand> nodeCommands = ui.getNodeCommandList();
            deviceCommunicationTask.addNodeCommands(nodeCommands);
            
            long newTime = System.currentTimeMillis();
            long deltaTime = newTime-oldTime;
            if (deltaTime>100)
            {
                // Don't let animation do too large a step if the computer gets busy
                deltaTime = 100;
            }
            
            if (nextTime<=newTime)
            {
                oldTime = newTime;
                nextTime += (long)16.667;
                // Frame skip
                if (nextTime<newTime)
                {
                    nextTime = newTime+(long)16.667;
                }
                ui.updateNodes(deviceList, deviceCommunicationTask.doneNodeCommands());
                
                try
                {
                    ui.draw(deltaTime);
                }
                catch (Exception e)
                {
                    Log.logger.warning("Drawing skipped, could be resizing?");
                    e.printStackTrace();
                }
            }
            
            

            
            try {
                Thread.sleep((int)1);
           } catch (InterruptedException e) {}
        }
        
        deviceCommunicationTask.done();
    }
    
    public void windowClosing(WindowEvent e) { done = true; }
    public void windowClosed(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowActivated(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}
    public void keyPressed( KeyEvent e ) { }
    public void keyReleased( KeyEvent e ) { }
    public void keyTyped( KeyEvent e )
    {
        char c = e.getKeyChar();
        if (c == KeyEvent.VK_ESCAPE)
        {
            done = true;
        }
        else if (c == '0')
            deviceCommunicationTask.testNodes = 0;
        else if (c == '1')
            deviceCommunicationTask.testNodes = 1;
        else if (c == '2')
            deviceCommunicationTask.testNodes = 2;
        else if (c == '3')
            deviceCommunicationTask.testNodes = 3;
        else if (c == '4')
            deviceCommunicationTask.testNodes = 4;
        else if (c == '5')
            deviceCommunicationTask.testNodes = 5;
        else if (c == '6')
            deviceCommunicationTask.testNodes = 6;
        else if (c == 'f' || c == 'F')
            ui.toggleFullScreen();
        e.consume();
    }
    
    public static JFrame getFrame()
    {
        return instance.ui;
    }
    
    public static void main(String[] args) 
    {
        instance = new PairingDemo();
        instance.run();
        System.exit(0);
    }
    
}
