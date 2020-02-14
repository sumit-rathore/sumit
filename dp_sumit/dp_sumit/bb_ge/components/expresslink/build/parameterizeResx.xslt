<?xml version="1.0" encoding="utf-8"?>
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Parameters for config.resx -->
  <xsl:param name="expresslink_release_name" select="''"/>

  <!-- Parameters for Resources.resx -->
  <xsl:param name="expresslink_build_path" select="''"/>


  <!-- Templates for config.resx -->
  <xsl:template match="value[parent::data[@name='ExpressLinkReleaseName']]">
    <xsl:copy><xsl:value-of select="$expresslink_release_name"/></xsl:copy>
  </xsl:template>


  <!-- Templates for Resources.resx -->
  <xsl:template match="value[parent::data[@name='leon_boot_flash']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='leon_boot_flash_icr']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='leon_boot_flash_w_recovery']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='flash_writer_lg']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='flash_writer_ge']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='flash_writer_ge_spartan']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='firmware_lg']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='firmware_ge']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='firmware_ge_spartan_lex']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="value[parent::data[@name='firmware_ge_spartan_rex']]">
    <xsl:copy>
      <xsl:value-of select="$expresslink_build_path"/><xsl:text>/</xsl:text>
      <xsl:value-of select="substring-after(text(), 'dummyPath\')"/>
    </xsl:copy>
  </xsl:template>


  <!-- This is the identity template from http://en.wikipedia.org/wiki/Identity_transform -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:transform>
