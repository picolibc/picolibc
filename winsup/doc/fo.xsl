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
</xsl:stylesheet>
