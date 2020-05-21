# Releasing Picolibc

Here are the current steps to perform when releasing a new version of
picolibc:

 1. Make sure the current code builds on all supported architectures:

   ** native
   ** 32-bit x86
   ** RISC-V
   ** ARM 32-bit
   ** ARM 64-bit (aarch64)
   ** ESP8266 (xtensa-lx106)
 
 2. Test on architectures that can do so

    * native
    * 32-bit x86 (requires x86 host)
    * RISC-V, all targets (qemu)
    * ARM, thumb v7m only (qemu)

 3. Add release notes to README.md
 
 4. Update meson.build file with new version number

 5. Use native configuration to build release:

	$ mkdir build-native
	$ cd build-native
        $ ../do-native-configure
	$ ninja dist

 6. Tag release

	$ git tag -m'Version <version-number>' <version-number> master

 7. Upload release to web site:

	$ scp build-native/meson-dist/* keithp.com:/var/www/picolibc/dist

 8. Email release message to mailing list. Paste in README.md section
    about the new release.

## Debian Packages

Debian packaging information is contained on the 'debian' branch in
the main picolibc repository. It's designed to be build using 'gbp
buildpackage', the git-based debian package building system. Here's
how to build debian packages:

 1. Release upstream picolibc first

 2. Checkout debian branch

	$ git checkout debian

 3. Merge in master at the release tag

	$ git merge <release-tag>

 4. Update debian packaging to current standards

 5. Add new Debian change log entry

	$ dch -v <release>-1 -D unstable

 6. Commit debian changes to repository

 7. Build debian packages

	$ gbp buildpackage

 8. Verify package results remain lintian-clean:

	$ lintian --info --display-info --pedantic picolibc_<version>-1_amd64.changes

 9. Sign and upload source changes:

	$ debsign picolibc_<version>-1_source.changes
	$ dput picolibc_<version>-1_source.changes
