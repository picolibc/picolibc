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

 6. Commit release notes and meson.build

	$ git commit -m'Version <version>' README.md meson.build

 6. Tag release

	$ git tag -m'Version <version>' <version> main

 7. Push tag and branch to repositories

	$ git push origin main <version>

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

 3. Merge in main at the release tag

	$ git merge <release-tag>

 4. Update debian packaging to current standards

 5. Update debian/copyright from COPYING.picolibc

	$ cp COPYING.picolibc 

 6. Add new Debian change log entry

	$ dch -v <release>-1 -D unstable

 7. Commit debian changes to repository

	$ git commit -m'debian: Version <version>-1' debian/changelog

 8. Build debian packages

	$ gbp buildpackage

 9. Verify package results remain lintian-clean:

	$ lintian --info --display-info --pedantic picolibc_<version>-1_amd64.changes

 10. Tag release

	$ git tag -m'debian: Version <version>-1' <version>-1 debian

 11. Push tags and branches to salsa

	$ git push salsa main debian <version> <version>-1

 12. Sign and upload source changes:

	$ debsign picolibc_<version>-1_source.changes
	$ dput picolibc_<version>-1_source.changes
