#include <unistd.h>
#include <string.h>
#include <getopt.h>

/* Written 2000 by Werner Almesberger */

static const char *__resume;


int getopt_long(int argc,char *const argv[],const char *optstring,
  const struct option *longopts,int *longindex)
{
    char *here;

    optarg = NULL;
    if (!__resume) {
	if (argc == optind || *argv[optind] != '-') return -1;
	if (argv[optind][1] == '-') {
	    const struct option *opt;

	    optarg = strchr(argv[optind],'=');
	    if (optarg) optarg++;
	    for (opt = longopts; opt->name &&
	      (optarg || strcmp(opt->name,argv[optind]+2)) &&
	      (!optarg || strlen(opt->name) != optarg-argv[optind]-3 ||
	      strncmp(opt->name,argv[optind]+2,optarg-argv[optind]-3));
	      opt++);
	    optind++;
	    if (!opt->name) return '?';
	    if ((opt->has_arg == no_argument && optarg) ||
	      (opt->has_arg == required_argument && !optarg)) return ':';
	    if (longindex) *longindex = opt-longopts;
	    if (!opt->flag) return opt->val;
	    *opt->flag = opt->val;
	    return 0;
	}
	else {
	    __resume = argv[optind]+1;
	}
    }
    here = strchr(optstring,*__resume);
    if (!here) {
	optind++;
	__resume = NULL;
	return '?';
    }
    if (here[1] != ':') {
	if (!*++__resume) __resume = NULL;
    }
    else {
	if (__resume[1]) optarg = (char *) __resume+1;
	else {
	    optarg = (char *) argv[++optind];
	    if (optind == argc || *argv[optind] == '-') {
		optind++;
		__resume = NULL;
		return ':';
	    }
	}
	__resume = NULL;
    }
    if (!__resume) optind++;
    return *here;
}
