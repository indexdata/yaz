<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0">

  <xsl:output indent="yes" method="xml" version="1.0" encoding="UTF-8"/>

  <!--
./yaz-xmlquery -p '@and @attr 1=1016 @attr 4=2 @attr 6=3 the @attr 1=4 fish' > test.xml && xmllint -format test.xml && ./yaz-xmlquery -x test1.xml && xsltproc pqf2pqf.xsl test.xml |tee test2.xml && ./yaz-xmlquery -x test2.xml 

./yaz-xmlquery -p '@not @attr 1=1016 @attr 4=2 @attr 6=3 @attr 7=1 @attr 8=4 fish @attr 1=4 fish' > test.xml && xmllint -format test.xml && ./yaz-xmlquery -x test.xml && xsltproc pqf2pqf.xsl test.xml |tee test2.xml && ./yaz-xmlquery -x test2.xml 
  -->

  <!-- disable default templates -->
  <xsl:template match="text()"/>
  <xsl:template match="node()"/>

  <!-- identity stylesheet templates -->
  <!-- these parse pqf-xml input recursively and make identity operations -->
  <xsl:template match="/query">
    <query>
      <xsl:apply-templates/>
    </query>
  </xsl:template>

  <xsl:template match="rpn">
    <rpn>
      <xsl:attribute name="set">
        <xsl:value-of  select="@set"/>
      </xsl:attribute>
      <xsl:apply-templates/>
    </rpn>
  </xsl:template>

  <xsl:template match="operator">
    <operator>
      <xsl:attribute name="type">
        <xsl:value-of  select="@type"/>
      </xsl:attribute>
      <xsl:apply-templates/>
    </operator>
  </xsl:template>

  <xsl:template match="apt">
    <apt>
      <!-- no re-ordering @attr's if you use the following -->
      <!--
      <xsl:apply-templates select="attr"/>
      -->
      <xsl:apply-templates select="attr[@type=1]"/>
      <xsl:apply-templates select="attr[@type=2]"/>
      <xsl:apply-templates select="attr[@type=4]"/>
      <xsl:apply-templates select="attr[@type=5]"/>
      <xsl:apply-templates select="attr[@type=6]"/>
      <xsl:apply-templates select="attr[@type=7]"/>
      <xsl:apply-templates select="attr[@type=8]"/>
      <xsl:apply-templates select="attr[@type=9]"/>
      <xsl:apply-templates select="term"/>
    </apt>
  </xsl:template>

  <xsl:template match="attr">
    <xsl:copy-of select="."/>
  </xsl:template>

  <xsl:template match="term">
    <xsl:copy-of select="."/>
  </xsl:template>


  <!-- special rewrite templates
       these are kicking in when special conditions apply -->


  <!-- attribute rewrites --> 

  <!-- remove all @attr 6=3 with bracket syntax -->
  <!--
  <xsl:template match="attr[@type=6][@value=3]">
  </xsl:template>
  -->

  <!-- remove all @attr 6=4 with and syntax -->
  <!--
  <xsl:template match="attr[@type=6 and @value=4]">
  </xsl:template>
  -->

  <!-- rewrite all @attr 4=2 to @attr 4=1 -->
  <!--
  <xsl:template match="attr[@type=4][@value=2]">
    <attr type="4" value="1"/>
  </xsl:template>
  -->

  <!-- rewrite all @attr 1=1016 to @attr 1=1016 @attr 6=2 -->
  <!-- this will leave double @attr 6=? nodes, unless you remove all
       @attr 6=? nodes in some other template -->
  <!--
  <xsl:template match="attr[@type=1 and @value=1016]">
    <attr type="1" value="1016"/>
    <attr type="6" value="2"/>
  </xsl:template>
  -->


  <!-- rules depending on multiple attribute combinations -->
  
  <!-- whenever there is a <apt> containing an @attr 7 and an @attr 8,
       rewrite these and drop all @attr 3 .
       Notice that the selection rules can equally either be written 
       'attr/@type=7' or 'attr[@type=8]' with no difference -->
  <!--
  <xsl:template match="apt[attr/@type=7 and attr[@type=8]]">
    <apt>
      <xsl:apply-templates select="attr[@type=1]"/>
      <xsl:apply-templates select="attr[@type=2]"/>
      <xsl:apply-templates select="attr[@type=4]"/>
      <xsl:apply-templates select="attr[@type=5]"/>
      <xsl:apply-templates select="attr[@type=6]"/>
      <attr type="7" value="2"/>
      <attr type="8" value="5"/>
      <xsl:apply-templates select="attr[@type=9]"/>
      <xsl:apply-templates select="term"/>
    </apt>
  </xsl:template>
  -->

  <!-- whenever there is an apt containing an @attr 7=1, an @attr 8=4, and
       an @attr 1=? (of any value), let @attr 1=? pass unaltered, drop
       @attr 3=? totally, and rewrite @attr 7=1 and @attr 8=4 .
       Notice that this rule can equally be written either with 'and' 
       connecting the attribute type and value, or with a double '[]'.-->
  <!--
  <xsl:template match="apt[attr[@type=7 and @value=1] 
                       and attr[@type=8][@value=4] 
                       and attr[@type=1]] ">
    <apt>
      <xsl:apply-templates select="attr[@type=1]"/>
      <xsl:apply-templates select="attr[@type=2]"/>
      <xsl:apply-templates select="attr[@type=4]"/>
      <xsl:apply-templates select="attr[@type=5]"/>
      <xsl:apply-templates select="attr[@type=6]"/>
      <attr type="7" value="2"/>
      <attr type="8" value="5"/>
      <xsl:apply-templates select="attr[@type=9]"/>
      <xsl:apply-templates select="term"/>
   </apt>
  </xsl:template>
  -->


  <!-- term rewrites -->

  <!-- rewrite general term fish to squid -->
  <!--
  <xsl:template match="term[@type='general'][text()='fish']">
    <term type="general">squid</term>
  </xsl:template>
  -->

  <!-- operator rewrites -->

  <!-- remove 'not' operator, use first <apt> only -->
  <!-- 
  <xsl:template match="operator[@type='not']">
    <xsl:apply-templates select="apt[1]"/>
  </xsl:template>
  -->

  <!-- nasty rewrite 'not' operator to 'and' operator -->
  <!--
  <xsl:template match="operator[@type='not']">
    <operator>
      <xsl:attribute name="type">
        <xsl:value-of  select="'and'"/>
      </xsl:attribute>
      <xsl:apply-templates/>
    </operator>
  </xsl:template>
  -->

</xsl:stylesheet>


