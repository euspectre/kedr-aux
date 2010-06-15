# Take object filename,
# and extract names of functions, imported by this object file.

$#ARGV == 0 or die "Usage is:\n\n\timported functions.pl object_file\n\n";
my $filename = shift;

my @table = `readelf -Ws $filename`;

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