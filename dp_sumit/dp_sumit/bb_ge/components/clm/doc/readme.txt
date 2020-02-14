CLM README
==========

Operation
---------
The CLM IC module is a bit unique as in operation it will be reset everytime a new physical link is acquired.  This is done, because each link will have a separate clock (GMII is 125MHz, MII is 25MHz, etc), and the PLL's need to reconfigured, which requires a reset of the CLM module.  Higher level code is expected to call this CLM driver in the following scenarios, with the described environment

1) CLM_Init():
    * This should be called a program startup, and only ever called once
    * This expects the CLM to be enabled, but after this function is called the CLM can be placed back into reset until a link is established
2) CLM_Start():
    * This should be called after a new physical link is established, and after the PLL's are configured, and the CLM/CTM/CRM is taken out of reset
    * This will configure the CLM for the new link
    * NOTE: This is actually broken into 2 stages, a pre and post
        * The pre should be called after PLL's are configured the CLM is taken out of reset
        * The post should be called after the CTM/CRM is taken out of reset
        * This allows the CLM module to be configured for the correct link type before the phy domain is enabled
3) CLM_Stop():
    * This should be called after a physical link is lost
    * The CLM/CTM/CRM can be disabled before or after calling this function.  NOTE: XCTM should also be disabled


