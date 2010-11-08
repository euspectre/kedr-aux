#! /usr/bin/perl

use warnings;
#
# Usage: verify_allocations.pl
#
# Read input and interpret is as a trace.
# Verify, whether __kmalloc and kfree calls are consistent.
# Also, calls to kmem_cache_alloc(), kmem_cache_alloc_notrace(), krealloc() are detected.
#
# If not, files unallocated_frees.txt, unfreed_allocations.txt will contain all incosistences.

# Lines which failed to parse is stored to fail_to_parse.txt.

# Auxiliary function for logging.
{
    my %fds = ();
    # log_to_file($filename, $string)
    # Destroy file at the first write, then write string to it(new-line symbol is not appended automatically)
    sub log_to_file
    {
        my ($filename, $string) = @_;
        # Lazy initialization
        if(! defined $fds{$filename})
        {
            open $fds{$filename}, "> $filename" || die "Cannot open file '$filename'.\n";
        }
        print { $fds{$filename} } "$string";
    }
}


# log_failed_to_parse($line)
sub log_failed_to_parse
{
    my ($line) = @_;
    log_to_file ("fail_to_parse.txt", "$line\n");
}


# get_function_name($trace_line)
sub get_function_name
{
    my ($line) = @_;
    my ($function) =  ($line =~ /called_(\w+)/);
    if(! defined $function)
    {
        log_failed_to_parse $line;
        $function = "";
    }
    return $function;
}

# get_malloc_address($kmalloc_trace_line)
sub get_malloc_address
{
    my ($line) = @_;
    my ($address) = ($line =~ /result:\s*(\w+)/);
    if(! defined $address)
    {
        log_failed_to_parse $line;
        $address = "";
    }

    
    return $address;
}

# get_free_address($kfree_trace_line)
sub get_free_address
{
    my ($line) = @_;
    if($line =~ /arguments:\s*\(+null/) { return "0"; }
    my ($address) = ($line =~ /arguments:\s*\((\w+)/);
    if(! defined $address)
    {
        log_failed_to_parse $line;
        $address = "";
    }

    return $address;
}

my $unallocated_frees_filename = "unallocated_frees.txt";
my $unfreed_allocations_filename = "unfreed_allocations.txt";


# log_unallocated_free($line)
sub log_unallocated_free
{
    my ($line) = @_;
    log_to_file($unallocated_frees_filename, "$line\n");
}

# log_unfreed_allocation($line)
sub log_unfreed_allocation
{
    my ($line) = @_;
    log_to_file($unfreed_allocations_filename, "$line\n");
}


# map adress -> allocation trace line
my %allocated_adresses = ();

my $unfreed_allocations_counter = 0;
my $unallocated_frees_counter = 0;

foreach my $line (<STDIN>)
{
    chomp $line;
    my $func = get_function_name($line);
    my $address;
    if("$func" eq "__kmalloc" || "$func" eq "kmem_cache_alloc" || "$func" eq "kmem_cache_alloc_notrace")
    {
        # Use same function for retrieve address
        $address = get_malloc_address($line);
        if($allocated_adresses{$address})
        {
            log_unfreed_allocation $allocated_adresses{$address};
            $unfreed_allocations_counter++;
        }
        $allocated_adresses{$address} = $line;
    }
    elsif("$func" eq "kfree")
    {
        $address = get_free_address($line);
        if("$address" eq "0") { next; } # free with null-argument
        if(! $allocated_adresses{$address})
        {
            log_unallocated_free $line;
            $unallocated_frees_counter++;
        }
        undef $allocated_adresses{$address};
    }
    elsif("$func" eq "krealloc")
    {
        #similar to kfree + __kmalloc
        $address = get_free_address($line);
        if(! $allocated_adresses{$address})
        {
            log_unallocated_free $line;
            $unallocated_frees_counter++;
        }
        undef $allocated_adresses{$address};

        $address = get_malloc_address($line);
        if($allocated_adresses{$address})
        {
            log_unfreed_allocation $allocated_adresses{$address};
            $unfreed_allocations_counter++;
        }
        $allocated_adresses{$address} = $line;
    }
    
}

foreach my $line (values %allocated_adresses)
{
    if($line)
    {
        log_unfreed_allocation $line;
        $unfreed_allocations_counter++;
    }
}

if($unfreed_allocations_counter || $unallocated_frees_counter)
{
    print "Trace is inconsistent.\n";
    print "Files \"$unfreed_allocations_filename\" and \"$unallocated_frees_filename\" contains lists of inconsitent calls.\n";
    exit 1;
}

print "Trace is consistent.\n";
