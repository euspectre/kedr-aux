# concatenate_statistic.pl [-c cut_number] files..
#
# Join statistic of kernel functions usage on several machines in one table.
# Rows in that table are functions,
# Columns in that table are machines names.
# Machine name is extract from name of file,
# containing usage statistic for that machine.
# (pattern - <filename> is [dir/]<machinename[.txt]>).
#
# Table also contain 'Total' column,
# with total usage of each function on all machines.
# Functions are sorted by that column in descend order.
#
# '-c cut_number' options tell script
# to perform only first 'cut_number' lines in each input table.

#hash <function - its statistic>
my %functions_usage = ();
#array of machine names
my @machines = ();
#only first $cut_number lines are processed in every table.
my $cut_number = 20000;

if($ARGV[0] eq "-c")
{
	shift @ARGV;
	$cut_number = shift @ARGV;
	$cut_number > 0 or die $usage_string;
}

my $usage_string = 
	"Usage:\n\n\tconcatenate_statistic.pl [-c cut_number] files..\n\n";

$#ARGV != -1 or die $usage_string;

#collect data
for my $filename (@ARGV)
{
	open(my $filehandle, $filename)
		or die "Cannot open file '$filename' for read.\n";
	my $count = 0;
	my $machine = $filename;
	if($machine =~ /(.*)\.txt/)
	{
		$machine = $1;
	}
	if($machine =~ /\/([^\/]+)$/)
	{
		$machine = $1;
	}
	push @machines, $machine;
	for my $line (<$filehandle>)
	{
		$count++;
		$count <= $cut_number or last;

		if($line !~ /^\s*(\S+)\s+(\d+)\s*/)
		{
			print("Ignore line '$line'\n");
			next;
		}
		my $function = $1;
		my $nusage = $2;

		exists $functions_usage{$function}
			or $functions_usage{$function} = {};

		$functions_usage{$function}->{$machine} = $nusage;
	}
	close $filehandle;
}
#total usage for each function
#as new "machine"
my $total_machine_name = "Total";
unshift(@machines, $total_machine_name);
for my $function (keys %functions_usage)
{
	my $count_total = 0;
	my $usage = $functions_usage{$function};
	for my $machine (keys %{$usage})
	{
		$count_total += $usage->{$machine};
	}
	$functions_usage{$function}->{$total_machine_name} = $count_total;
}

#print collected data
printf("%-30s", "Function");
for my $machine (@machines)
{
	printf(" %s", "$machine");
}
print("\n");
#sort %functions_usage by 'Total' field.
my @functions_sorted = sort
 {$functions_usage{$b}->{$total_machine_name}
	<=> $functions_usage{$a}->{$total_machine_name}}
	keys %functions_usage;

for my $function (@functions_sorted)
{
	printf("%-30s", "$function");
	for my $machine (@machines)
	{
		my $nusage = $functions_usage{$function}->{$machine};
		$nusage ne "" or $nusage = "-";
		printf(" %".length($machine)."s", "$nusage");
	}
	print("\n");
}