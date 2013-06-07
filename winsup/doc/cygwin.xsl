<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version='1.0'>

<xsl:param name="chunker.output.doctype-public" 
  select="'-//W3C//DTD HTML 4.01 Transitional//EN'" />
<xsl:param name="html.stylesheet" select="'docbook.css'"/>
<xsl:param name="use.id.as.filename" select="1" />
<xsl:param name="root.filename" select="@id" />

</xsl:stylesheet>
