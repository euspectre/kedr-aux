# join_synonyms.pl file_with_synonyms
#
# Join rows for synonym functions in functions usage statictic.
#
# Accept table with statistic in INPUT stream, and out the same table,
# but join rows for synonym functions, listed in 'file_with_excludes',
# into one row.
#
# Each synonyms group should be listed in the file each at the new line.
# Surrounding space are stripped.
# Empty lines and lines started with '#' are ignored.
#
# Synonyms should be exclusive, so for every machine and every group
# no more than one function from the group should exists on the machine.
#
# Name of row header for synonyms is formed from the first synonym
# in the group.

$usage_string = "Usage:\n\n\tjoin_synonyms.pl file_with_synonyms\n\n";

$#ARGV == 0 or die($usage_string);

open(my $synonyms_file, $ARGV[0]) or die("Cannot open file '$ARGV[0]' for read.");

my %synonym_map = ();
for my $line (<$synonyms_file>)
{
	$line =~ s/^\s*(.*)\s*$/\1/;
	$line ne "" or next;
	$line !~ /^#/ or next;
	my @synonyms = split(/\s+/, $line);
	my $main_synonym = $synonyms[0];
	for my $synonym (@synonyms)
	{
		#[] mark out synonyms group
		$synonym_map{$synonym} = "[$main_synonym]";
	}
}
# Reconstruct statistic data
(my $function_header, my $total_header, my @machines) = split(/\s+/, <STDIN>);
#machines with 'total'
my @machines_all = ($total_header, @machines);

my %functions_usage = ();

for my $line (<STDIN>)
{
	(my $function, my @usage_all) = split(/\s+/, $line);
	if(not exists $synonym_map{$function})
	{
		$functions_usage{$function} = {};
		for my $machine (@machines_all)
		{
			$functions_usage{$function}->{$machine} = shift @usage_all;
		}
		next;
	}
	my $synonym = $synonym_map{$function};
	if(not exists $functions_usage{$synonym})
	{
		$functions_usage{$synonym} = {};
		for my $machine (@machines_all)
		{
			$functions_usage{$synonym}->{$machine} = shift @usage_all;
		}
		next;
	}
	#real join
	my $total_value = shift @usage_all;
	for my $machine (@machines)
	{
		my $old_value = $functions_usage{$synonym}->{$machine};
		my $new_value = shift @usage_all;
		if($new_value !~ /^\d+$/ or $new_value == 0)
		{
			#current function is not used on the current machine
			#do not overwrite usage value
			next;
		}
		if($old_value =~ /^\d+$/ and $old_value != 0)
		{
			print(STDERR
				"Synonym '$synonym' is used on machine '$machine' in two or more instances.\n");
		}
		$functions_usage{$synonym}->{$machine} = $new_value;
	}
	#Only 'total' field is added to previous value
	$functions_usage{$synonym}->{$total_header} += $total_value;
}

#print collected data(as in concatenate_statictic.pl)
printf("%-30s", $function_header);
for my $machine (@machines_all)
{
	printf(" %s", "$machine");
}
print("\n");
#sort %functions_usage by 'Total' field.
my @functions_sorted = sort
 {$functions_usage{$b}->{$total_header}
	<=> $functions_usage{$a}->{$total_header}}
	keys %functions_usage;

for my $function (@functions_sorted)
{
	printf("%-30s", "$function");
	for my $machine (@machines_all)
	{
		printf(" %".length($machine)."s",
			$functions_usage{$function}->{$machine});
	}
	print("\n");
}