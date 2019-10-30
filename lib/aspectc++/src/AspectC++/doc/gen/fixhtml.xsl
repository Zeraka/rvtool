<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xhtml="http://www.w3.org/1999/xhtml"
  xmlns="http://www.w3.org/1999/xhtml"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  exclude-result-prefixes="xhtml xsl xs">

  <xsl:output method="xml" version="1.0" encoding="UTF-8" doctype-public="-//W3C//DTD XHTML 1.1//EN" doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" indent="yes"/>

  <!-- identity template: copy all nodes by default -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <!-- insert a link to an external style sheet -->
  <xsl:template match="xhtml:head">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
      <link rel="StyleSheet" href="ac-manual.css" type="text/css"/>
    </xsl:copy>
  </xsl:template>

  <!-- remove font setting in table of contents -->
  <xsl:template match="xhtml:a[@class='tocentry']/xhtml:span">
    <xsl:apply-templates/>
  </xsl:template>

  <!-- fix line break problem in table caption -->
  <xsl:template match="xhtml:div[@class='float-caption-Standard float-caption float-caption-standard']/xhtml:div[@class='plain_layout']">
    <xsl:apply-templates/>
  </xsl:template>

  <!-- rename images -->
  <xsl:template match="xhtml:img">
    <img style="{@style}">
      <xsl:attribute name="src">
        <xsl:value-of select="substring-after(@src,'_doc_')" />
      </xsl:attribute>
      <xsl:attribute name="alt">
        <xsl:value-of select="substring-after(@alt,'_doc_')" />
      </xsl:attribute>
    </img>
  </xsl:template>

  <!-- remove index: export seems broken -->
  <xsl:template match="xhtml:div[@class='index section']"/>
    
</xsl:stylesheet>
