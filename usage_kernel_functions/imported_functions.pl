# Take object filename,
# and extract names of functions, exported by this object file.

# Verify number of arguments (should be 1 - object file).
$#ARGV == 0 or die "Usage is:\n\n\timported functions.pl object_file\n\n";
my $filename = shift;

my @table = `readelf -s $filename`;

for my $line (@table)
{
	if($line =~ /^\s*\S+\s+\S+\s+\S+\s+\S+\s+(\w+)\s+\S+\s+(\w+)\s+(\w+)/)
	{
		my $bind = $1;
		my $ndx = $2;
		my $name = $3;
		if(($bind =~ /^GLOBAL$/) and ($ndx =~ /^UND$/))
		{
			print "$name\n";
		}
	}
}