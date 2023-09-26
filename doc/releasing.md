# Releasing Picolibc

Here are the current steps to perform when releasing a new version of
picolibc:

 1. Make sure the current code builds on all supported architectures:

   ** native
   ** 32-bit x86
   ** RISC-V
   ** ARM 32-bit
   ** ARM 64-bit (aarch64)
   ** Xtensa (ESP8266, ESP32)
 
 2. Test on architectures that can do so

    * native
    * 64-bit x86
    * 32-bit x86
    * RISC-V, all targets (qemu)
    * ARM 32-bit, almost all targets (qemu)
    * ARM 64-bit

 3. Use glibc test suite for RISC-V and ARM 32-bit:

	$ ./scripts/test-picolibc

    * Enables errno in the math functions using -Dwant-math-errno=true

    * Enables long-double io for RISC-V compiles using -Dio-long-double=true

    * Builds and installs minsize and release builds

    * Builds and runs glibc test suite against all four builds

 4. Test c++ builds using hello-world example on ARM

 5. Verify that COPYING.picolibc includes information
    about the current source files

 6. Add release notes to README.md
 
 7. Update meson.build and CMakeLists.txt with new version number

 8. Commit release notes and meson.build

	$ git commit -m'Version <version>' README.md meson.build CMakeLists.txt

 9. Use native configuration to build release:

	$ mkdir -p builds/build-native
	$ cd builds/build-native
        $ ../../scripts/do-native-configure
	$ ninja dist

 10. Tag release

	$ git tag -m'Version <version>' <version> main

 11. Use arm configuration to build bits for the Arm embedded toolkit:

        $ ./scripts/do-arm-tk
        $ scp builds/dist/picolibc-<version>-<arm-et-version>.zip keithp.com:/var/www/picolibc/dist/gnu-arm-embedded

 12. Push tag and branch to repositories

	$ git push origin main <version>

 13. Upload release to web site:

	$ scp build-native/meson-dist/* keithp.com:/var/www/picolibc/dist

 14. Create new release on picolibc wiki pasting in relevant README.md
     section.

 15. Create new release on github site, pasting in relevant README.md
     section. Upload release tar and arm embedded toolkit zip files.

 16. Email release message to mailing list. Paste in README.md section
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
