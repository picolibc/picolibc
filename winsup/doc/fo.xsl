<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:fo="http://www.w3.org/1999/XSL/Format"
		version="1.0">
	
	<!-- Import the standard DocBook stylesheet that this one is based on.
	     We use a web URL, but the local XML catalog should resolve this to
			 the local copy of the stylesheet, if it exists. -->
	<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/fo/docbook.xsl"/>

	<!-- Add page breaks before each sect1 -->
	<xsl:attribute-set name="section.level1.properties">
		<xsl:attribute name="break-before">page</xsl:attribute>
	</xsl:attribute-set>

	<!-- Rag-right lines -->
	<xsl:attribute-set name="root.properties">
		<xsl:attribute name="text-align">left</xsl:attribute>
	</xsl:attribute-set>

	<!-- Use a smaller font for code listings to increase the chances
	     that they can fit on a single sheet, to reduce FOP complaints
			 about being forced to split a listing across pages. -->
	<xsl:attribute-set name="monospace.verbatim.properties">
		<xsl:attribute name="font-size">85%</xsl:attribute>
	</xsl:attribute-set>

	<!-- Inform the DocBook stylesheets that it's safe to use FOP
	     specific extensions. -->
	<xsl:param name="fop1.extensions" select="1"/>

	<!-- autotoc.xsl customization to make refentry in sect1 appear in toc -->
<xsl:template match="sect1" mode="toc">
  <xsl:param name="toc-context" select="."/>

  <xsl:variable name="id">
    <xsl:call-template name="object.id"/>
  </xsl:variable>

  <xsl:variable name="cid">
    <xsl:call-template name="object.id">
      <xsl:with-param name="object" select="$toc-context"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:call-template name="toc.line">
    <xsl:with-param name="toc-context" select="$toc-context"/>
  </xsl:call-template>

  <xsl:variable name="depth.from.context" select="count(ancestor::*)-count($toc-context/ancestor::*)"/>

  <xsl:if test="$toc.section.depth > 1
                and $toc.max.depth > $depth.from.context">
    <fo:block id="toc.{$cid}.{$id}">
      <xsl:attribute name="margin-{$direction.align.start}">
        <xsl:call-template name="set.toc.indent"/>
      </xsl:attribute>

      <xsl:apply-templates select="refentry|sect2|qandaset[$qanda.in.toc != 0]"
                           mode="toc">
        <xsl:with-param name="toc-context" select="$toc-context"/>
      </xsl:apply-templates>
    </fo:block>
  </xsl:if>
</xsl:template>

	<!-- generate ansi rather than k&r style function synopses -->
	<xsl:param name="funcsynopsis.style" select="ansi" />

</xsl:stylesheet>
