<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<!-- don't truncate manpage titles for long function names -->
<xsl:param name="man.th.title.max.length" select="33" />

<!-- don't moan about missing metadata -->
<xsl:param name="refentry.meta.get.quietly" select="1" />

<!-- base URL for relative links -->
<xsl:param name="man.base.url.for.relative.links">https://cygwin.com/cygwin-ug-net/</xsl:param>

</xsl:stylesheet>
