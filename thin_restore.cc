// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
// This file is part of the thin-provisioning-tools source.
//
// thin-provisioning-tools is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// thin-provisioning-tools is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with thin-provisioning-tools.  If not, see
// <http://www.gnu.org/licenses/>.

#include "emitter.h"
#include "human_readable_format.h"
#include "metadata.h"
#include "restore_emitter.h"
#include "xml_format.h"
#include "version.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

namespace {
	int restore(string const &backup_file, string const &dev) {
		try {
			// The block size gets updated by the restorer.
			metadata::ptr md(new metadata(dev, metadata::CREATE, 128, 0));
			emitter::ptr restorer = create_restore_emitter(md);
			ifstream in(backup_file.c_str(), ifstream::in);
			parse_xml(in, restorer);

		} catch (std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}

		return 0;
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options]" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-i|--input} input_file" << endl
		    << "  {-o|--output} {device|file}" << endl
		    << "  {-V|--version}" << endl;
	}
}

int main(int argc, char **argv)
{
	int c;
	const char *shortopts = "hi:o:V";
	string input, output;
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o'},
		{ "version", no_argument, NULL, 'V'},
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
			case 'h':
				usage(cout, basename(argv[0]));
				return 0;

			case 'i':
				input = optarg;
				break;

			case 'o':
				output = optarg;
				break;

			case 'V':
				cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
				return 0;

			default:
				usage(cerr, basename(argv[0]));
				return 1;
		}
	}

	if (argc != optind) {
		usage(cerr, basename(argv[0]));
		return 1;
	}

        if (input.empty()) {
		cerr << "No input file provided." << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	if (output.empty()) {
		cerr << "No output file provided." << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}

	return restore(input, output);
}
