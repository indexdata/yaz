<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:marc="http://www.loc.gov/MARC21/slim"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  >
  <xsl:output method="text"/>
  <xsl:strip-space elements="*"/>
  
  <xsl:template name="printfield">
     <xsl:param name="name"/>
     <xsl:param name="value"/>
     <xsl:if test="string-length($value) &gt; 0">
       <xsl:text>%</xsl:text><xsl:value-of select="$name"/>
       <xsl:text> </xsl:text><xsl:value-of select="$value"/>
<xsl:text>
</xsl:text>
     </xsl:if>
  </xsl:template>

  <xsl:template match="marc:record">
    <xsl:variable name="title_medium" select="marc:datafield[@tag='245']/marc:subfield[@code='h']"/>
    <xsl:variable name="journal_title" select="marc:datafield[@tag='773']/marc:subfield[@code='t']"/>
    <xsl:variable name="electronic_location_url" select="marc:datafield[@tag='85
6']/marc:subfield[@code='u']"/>
    <xsl:variable name="fulltext_a" select="marc:datafield[@tag='900']/marc:subfield[@code='a']"/>
    <xsl:variable name="fulltext_b" select="marc:datafield[@tag='900']/marc:subfield[@code='b']"/>
    <xsl:variable name="medium">
      <xsl:choose>
	<xsl:when test="$title_medium">
	  <xsl:value-of select="translate($title_medium, ':[]/', '')"/>
	</xsl:when>
	<xsl:when test="$fulltext_a">
	  <xsl:text>Electronic Resource</xsl:text>
	</xsl:when>
	<xsl:when test="$fulltext_b">
	  <xsl:text>Electronic Resource</xsl:text>
	</xsl:when>
	<xsl:when test="$journal_title">
	  <xsl:text>article</xsl:text>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:text>Book</xsl:text>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- Medium -->
    <xsl:call-template name="printfield">
      <xsl:with-param name="name">0</xsl:with-param>
      <xsl:with-param name="value"><xsl:value-of select="$medium"/>
      </xsl:with-param>
    </xsl:call-template>

    <!-- Author -->
    <xsl:choose>
      <xsl:when test="marc:datafield[@tag='100' and @ind1='1']">
	<xsl:for-each select="marc:datafield[@tag='100']">
	  <xsl:call-template name="printfield">
	    <xsl:with-param name="name">A</xsl:with-param>
	    <xsl:with-param name="value">
	      <xsl:value-of select="marc:subfield[@code='a']"/>
	    </xsl:with-param>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:when>
      <xsl:when test="marc:datafield[@tag='700' and @ind1='1']">
	<xsl:for-each select="marc:datafield[@tag='700']">
	  <xsl:call-template name="printfield">
	    <xsl:with-param name="name">A</xsl:with-param>
	    <xsl:with-param name="value">
	      <xsl:value-of select="marc:subfield[@code='a']"/>
	    </xsl:with-param>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
      
    <xsl:for-each select="marc:datafield[@tag='245']">
      <!-- Title -->
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">T</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>

      <!-- Secondary title -->
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">B</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='b']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
    
    <xsl:for-each select="marc:datafield[@tag='260']">
      <!-- Place Published -->
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">C</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
      <!-- Publisher -->
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">I</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='b']"/>
	</xsl:with-param>
      </xsl:call-template>
      <!-- Year -->
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">D</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='c']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- Keywords -->
    <xsl:for-each select="marc:datafield[@tag='650']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">K</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- Callnumber -->
    <xsl:for-each select="marc:datafield[@tag='852']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">L</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='h']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- Pages -->
    <xsl:for-each select="marc:datafield[@tag='300']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">P</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- URL -->
    <xsl:for-each select="marc:datafield[@tag='856']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">U</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='u']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- Volume -->
    <xsl:for-each select="marc:datafield[@tag='245']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">V</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='n']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- Abstract -->
    <xsl:for-each select="marc:datafield[@tag='520']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">X</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- ISBN -->
    <xsl:for-each select="marc:datafield[@tag='020']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">@</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  
    <!-- ISSN -->
    <xsl:for-each select="marc:datafield[@tag='022']">
      <xsl:call-template name="printfield">
	<xsl:with-param name="name">@</xsl:with-param>
	<xsl:with-param name="value">
	  <xsl:value-of select="marc:subfield[@code='a']"/>
	</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>
  <xsl:template match="text()"/>
</xsl:stylesheet>
