subdir('include')

machine_dirs = {
		 'a29k': 'a29k',
		 'aarch64': 'aarch64',
		 'arc': 'arc',
		 'arm': 'arm',
		 'bfin': 'bfin',
		 'cr16': 'cr16',
		 'cris': 'cris',
		 'crisv32': 'cris',
		 'crx': 'crx',
		 'd10v': 'd10v',
		 'd30v': 'd30v',
		 'epiphany': 'epiphany',
		 'fido': 'm68k',
		 'fr30': 'fr30',
		 'frv': 'frv',
		 'ft32': 'ft32',
		 'h8300': 'h8300',
		 'h8500': 'h8500',
		 'hppa': 'hppa',
		 'i960': 'i960',
		 'i386': 'i386',
		 'i486': 'i386',
		 'i586': 'i386',
		 'i686': 'i386',
		 'iq2000': 'iq2000',
		 'lm32': 'lm32',
		 'm32c': 'm32c',
		 'm32r': 'm32r',
		 'm6811': 'm68hc11',
		 'm6812': 'm68hc11',
		 'm68hc11': 'm68hc11',
		 'm68hc12': 'm68hc11',
		 'm68k': 'm68k',
		 'm88k': 'm88k',
		 'm88110': 'm88k',
		 'microblaze': 'microblaze',
		 'mep': 'mep',
		 'mips': 'mips',
		 'mn10200': 'mn10200',
		 'mn10300': 'mn10300',
		 'moxie': 'moxie',
		 'msp430': 'msp430',
		 'mt': 'mt',
		 'nds32': 'nds32',
		 'nios2': 'nios2',
		 'nvptx': 'nvptx',
		 'or1k': 'or1k',
		 'or1knd': 'or1k',
		 'powerpc': 'powerpc',
		 'riscv': 'riscv',
		 'rl78': 'rl78',
		 'rx': 'rx',
		 'sh': 'sh',
		 'sh64': 'sh',
		 'sparc': 'sparc',
		 'c4x': 'tic4x',
		 'tic4x': 'tic4x',
		 'tic6x': 'tic6x',
		 'tic80': 'tic80',
		 'v850': 'v850',
		 'visium': 'visium',
		 'w65': 'w65',
		 'x86_64': 'x86_64',
		 'xc16x': 'xc16x',
		 'xstormy16': 'xstormy16',
		 'z8k': 'z8k',
		 'spu': 'spu',
	       }

libdirs = ['argz', 'ctype', 'errno', 'iconv', 'misc', 'search',
	   'signal', 'ssp', 'stdlib', 'string', 'time',
	   'xdr', 'locale']

if newlib_tinystdio
  libdirs += 'tinystdio'
else
  libdirs += ['stdio', 'reent' ]
endif

if newlib_supplied_syscalls
  lib_dirs += 'syscalls'
endif

libnames = libdirs

if machine_dirs.has_key(host_cpu)
  libdirs = [join_paths('machine', machine_dirs[host_cpu])] + libdirs
  libnames = ['machine'] + libnames
else
  warning('No libc machine directory for cpu ' + host_cpu)
endif

foreach libdir : libdirs
  subdir(libdir)
endforeach

foreach target : targets
  value = get_variable('target_' + target)
  libobjs = []

  foreach libname : libnames
    libobjs += get_variable('lib_' + libname + target).extract_all_objects(recursive:true)
  endforeach

  instdir = join_paths(get_option('libdir'), value[0])

  if target == ''
    libc_name = 'c'
  else
    libc_name = join_paths(target, 'libc')
  endif

  set_variable('lib_c' + target,
	       static_library(libc_name,
			      install : true,
			      install_dir : instdir,
			      pic: false,
			      objects : libobjs))
endforeach

