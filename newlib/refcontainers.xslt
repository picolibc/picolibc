<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <!-- trivial XSLT which removes the refentrycontainer layer -->
 <xsl:output method="xml" encoding="UTF-8" indent="yes" doctype-public="-//OASIS//DTD DocBook V4.5//EN" doctype-system="http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" />
 <xsl:strip-space elements="*"/>

 <!-- Whenever you match any node but refentrycontainer or any attribute -->
 <xsl:template match="node()[not(self::refentrycontainer)]|@*">
 <!-- Copy the current node -->
  <xsl:copy>
    <!-- Including any attributes it has and any child nodes -->
   <xsl:apply-templates select="node()|@*"/>
  </xsl:copy>
 </xsl:template>
</xsl:stylesheet>
