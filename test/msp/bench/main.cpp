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
	          "\n msp-bench [-r] [-n...] [--sequential] [--strategy] [--strategy2] file \n\n"
	          << "Benchmark multi-objective shortest path algorithms\n"
	          << "Options and arguments:\n"
	          << "-r\t\t Number of repetitions to be used for each benchmark\n"
	          << "-n\t\t Number of processors to be used for benchmarking\n"
	          << "-c\t\t A comment describing the benchmark run.\n"
	          << "--sequential\t\t Benchmark the sequential algorithm."
	          << " Benchmark will be run for n=1.\n"
	          << "--strategy\t Benchmark with BStrategyScheduler<DistKStrategyTaskStorage<128, Global>>.\n"
	          << "--s2klsm\t Benchmark with StrategyScheduler2 plus KLSMLocalityTaskStorage.\n"
	          << "--s2lsm\t Benchmark with StrategyScheduler2 plus LSMLocalityTaskStorage.\n"
	          << "--s2pareto\t Benchmark with StrategyScheduler2 plus ParetoLocalityTaskStorage.\n"
	          << "file\t\t Input file, containing a graph to run the benchmark on.\n\n";
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
	int s2klsm = 0;
	int s2lsm = 0;
	int s2pareto = 0;

	int comment = 0;

	while (1) {
		static struct option long_options[] = {
			{"sequential",      no_argument,       &sequential, 1},
			{"strategy", no_argument,       &strategy,   1},
			{"s2klsm", no_argument,       &s2klsm,   1},
			{"s2lsm", no_argument,       &s2lsm,   1},
			{"s2pareto", no_argument,       &s2pareto,   1},
			{"ncpus",    required_argument, 0,           'n'},
			{"reps",     required_argument, 0,           'r'},
			{"comment", required_argument, &comment, 'c'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long(argc, argv, "n:r:c:", long_options, &option_index);

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

		case 'c':
			opts.comment = std::string(optarg);
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
	opts.s2klsm  = (s2klsm != 0);
	opts.s2lsm   = (s2lsm != 0);
	opts.s2pareto   = (s2pareto != 0);

	if ((opts.strategy || opts.s2klsm || opts.s2lsm || opts.s2pareto) && opts.ncpus.size() == 0) {
		std::cerr <<
		          "Number of processors needs to be specified when --strategy, --s2klsm, --s2lsm or --s2pareto is given.\n";
		usage();
	}

	MspBenchmarks t;
	t.run_benchmarks(opts);

	return EXIT_SUCCESS;
}
