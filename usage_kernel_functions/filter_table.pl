# filter_table.pl file_with_excludes
#
# Filter functions usage statictic.
#
# Accept table with statistic in INPUT stream, and out the same table,
# but without functions, listed in 'file_with_excludes'.
#
# Excluded functions should be listed in the file each at the new line.
# Surrounding space are stripped.
# Empty lines and lines started with '#' are ignored.

$usage_string = "Usage:\n\n\tfilter_table.pl file_with_excludes\n\n";

$#ARGV == 0 or die($usage_string);
my $exclude_filename = shift @ARGV;
open(my $exclude_file, $exclude_filename) 
	or die("Cannot open file '$exclude_filename' for read.");

my %functions_exclude = ();
for my $line (<$exclude_file>)
{
	$line =~ s/^\s*(\S*)\s*$/\1/;
	$line ne "" or next;
	$line !~ /^#/ or next;
	$functions_exclude{$line} = "exclude";
}
#shift header line(how??)
#print(readline);
my $is_header_line = 1;
for my $line (<STDIN>)
{
	chomp $line;
	if($is_header_line)
	{
		$is_header_line = 0;
		print "$line\n";
		next;
	}
	#filter rule
	$line =~ /^\s*(\S+)/ and exists $functions_exclude{$1} and next;

	print("$line\n");
}