/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#define NDEBUG 1

#include "MspBenchmarks.h"
#include "Util.h"

#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>

using namespace pheet;

static inline bool
is_file(char const* name)
{
	struct stat s;

	return (stat(name, &s) == 0 && (s.st_mode & S_IFREG));
}

static void
parse(char const* name,
      std::vector<std::string>& files)
{
	struct stat s;

	if (stat(name, &s) == 0) {
		if (s.st_mode & S_IFREG) {
			//it's a file
			files.push_back(name);
		} else {
			std::cerr << "\"" << name << "\" is not a file." << std::endl;
		}
	} else {
		std::cerr << "Skipping file \"" << name << "\" because it does not exist." << std::endl;
	}
}

static void
usage()
{
	std::cerr <<
	          "\n msp-bench [-r] [-n...] [--seq] [--strategy] [--strategy2] file \n\n"
	          << "Benchmark multi-objective shortest path algorithms\n"
	          << "Options and arguments:\n"
	          << "-r\t\t number of repetitions to be used for each benchmark\n"
	          << "-n\t\t number of processors to be used for benchmarking\n"
	          << "--seq\t\t benchmark the sequential algorithm."
	          << " Benchmark will be run for n=1.\n"
	          << "--strategy\t benchmark the parallel algorithm.\n"
	          << "--strategy2\t benchmark the parallel algorithm (Strategy2 scheduler variant).\n"
	          << "file\t\t input file, containing a graph to run the benchmark on.\n\n";
	exit(EXIT_FAILURE);
}

static int
parse_int(char const* str)
{
	int val;
	if (!msp::Util::parse_int(str, val)) {
		usage();
	}
	return val;

}

int
main(int argc, char** argv)
{
	int c;

	pheet::BenchOpts opts;

	int sequential = 0;
	int strategy = 0;
	int strategy2 = 0;

	while (1) {
		static struct option long_options[] = {
			{"seq",      no_argument,       &sequential, 1},
			{"strategy", no_argument,       &strategy,   1},
			{"strategy2", no_argument,       &strategy2,   1},
			{"ncpus",    required_argument, 0,           'n'},
			{"reps",     required_argument, 0,           'r'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long(argc, argv, "n:r:", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 0:
			//Nothing to do; option set the flag
			break;

		case 'n':
			opts.ncpus.push_back(parse_int(optarg));
			break;

		case 'r':
			opts.repetitions = parse_int(optarg);
			break;

		case '?':
			usage();

		default:
			usage();
		}
	}

	if (optind < argc) {
		parse(argv[optind], opts.files);
	} else {
		std::cerr << "No input file given." << std::endl;
		usage();
	}

	opts.sequential = (sequential != 0);
	opts.strategy   = (strategy != 0);
	opts.strategy2   = (strategy2 != 0);

	if ((opts.strategy || opts.strategy2) && opts.ncpus.size() == 0) {
		std::cerr <<
		          "Number of processors needs to be specified when --strategy or --strategy2 is given.\n";
		usage();
	}

	MspBenchmarks t;
	t.run_benchmarks(opts);

	return EXIT_SUCCESS;
}
