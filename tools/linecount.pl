

my $lines = 0;
my $chars = 0;

traverse_dirs($ARGV[0]);
print "Lines of code: $lines\n";
print "Bytes of code: $chars\n";
exit(0);

sub traverse_dirs {
	my ($dir) = @_;

	opendir(DIR, $dir) or die "Error opening directory, $dir\n";
	my @dirs = readdir(DIR);
	closedir(DIR);

	foreach $file (@dirs) {
		next if ($file eq "." or $file eq "..");
		if (-d "$dir/$file") {
			traverse_dirs("$dir/$file");
		}
		elsif ($file =~ /^(.*)\.(c|cpp|h|pm|pl)$/) {
			count_lines("$dir/$file");
		}
	}
}

sub count_lines {
	my ($file) = @_;

	open(FILE, $file) or die "Unable to open file, $file\n";
	while (<FILE>) {
		$lines++;
		$chars += length;
	}
	close(FILE);
}

