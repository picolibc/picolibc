<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<!-- don't truncate long manual names -->
<xsl:param name="man.th.extra3.max.length" select="45" />

<!-- don't moan about missing metadata -->
<xsl:param name="refentry.meta.get.quietly" select="1" />

<!-- generate ansi rather than k&r style function synopses -->
<xsl:param name="funcsynopsis.style" select="ansi" />

</xsl:stylesheet>
