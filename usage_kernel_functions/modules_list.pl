my @list = `PATH=$PATH:/sbin lsmod`;
shift @list; #remove header line
for my $line (@list)
{
	if($line =~ /^(\w+)/) {print "$1\n";}
}
