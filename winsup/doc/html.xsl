<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version='1.0'>

<xsl:param name="chunker.output.doctype-public"
  select="'-//W3C//DTD HTML 4.01 Transitional//EN'" />
<xsl:param name="html.stylesheet" select="'docbook.css'"/>
<xsl:param name="use.id.as.filename" select="1" />
<xsl:param name="root.filename" select="@id" />
<xsl:param name="toc.section.depth" select="4" />

<!-- autotoc.xsl customization to make refentry in sect1 appear in toc -->
<xsl:template match="sect1" mode="toc">
  <xsl:param name="toc-context" select="."/>
  <xsl:call-template name="subtoc">
    <xsl:with-param name="toc-context" select="$toc-context"/>
    <xsl:with-param name="nodes" select="sect2|refentry
                                         |bridgehead[$bridgehead.in.toc != 0]"/>
  </xsl:call-template>
</xsl:template>

<!-- suppress refentry in toc being annotated with refpurpose -->
<xsl:param name="annotate.toc" select="0" />

<!-- generate ansi rather than k&r style function synopses -->
<xsl:param name="funcsynopsis.style" select="ansi" />

</xsl:stylesheet>
