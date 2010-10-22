#! /usr/bin/perl

use warnings;
#
# Usage: verify_allocations.pl
#
# Read input and interpret it as a trace.
# Verify, whether locking mechanizm is used consistency.
#
# If not, files inconsistent_locks.txt, inconsistent_unlocks.txt will contain all incosistences.

# Lines which failed to parse is stored to fail_to_parse.txt(for debugging purposes).

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
        log_failed_to_parse "function: $line";
        $function = "";
    }
    return $function;
}

# get_lock_address($*_(un)lock_*_line)
sub get_lock_address
{
    my ($line) = @_;
    my ($address) = ($line =~ /arguments:\s*\((\w+)/);
    if(! defined $address)
    {
        log_failed_to_parse "lock: $line";
        $address = "";
    }
    return $address;
}

# get_trylock_result($*_trylock_*_line)
sub get_trylock_result
{
    my ($line) = @_;
    my ($result) = ($line =~ /result:\s*(\w+)/);
    if(! defined $result)
    {
        log_failed_to_parse $line;
        $result = "";
    }

    
    return $result;
}

my $inconsistent_locks_filename = "inconsistent_locks.txt";
my $inconsistent_unlocks_filename = "inconsistent_unlocks.txt";


# log_inconsistent_lock($line)
sub log_inconsistent_lock
{
    my ($line) = @_;
    log_to_file($inconsistent_locks_filename, "$line\n");
}

# log_inconsistent_unlock($line)
sub log_inconsistent_unlock
{
    my ($line) = @_;
    log_to_file($inconsistent_unlocks_filename, "$line\n");
}

# For simplification function's switch
my %lock_types = (
    "_spin_lock_irqsave" => "spin_lock_irqsave",
    "_raw_spin_lock_irqsave" => "spin_lock_irqsave",
    "_spin_unlock_irqrestore" => "spin_unlock_irqrestore",
    "_raw_spin_unlock_irqrestore" => "spin_unlock_irqrestore",

    "mutex_lock" => "mutex_lock",
    "mutex_lock_interruptible" => "mutex_lock",
    "mutex_try_lock" => "mutex_try_lock",
    "mutex_unlock" => "mutex_unlock"
);

# map adress -> trace line with spin_lock_irqsave/spin_unlock_irq_restore
my %spin_lock_irqsave_addresses = ();
# map adress -> trace line with mutex_lock/mutex_unlock
my %mutex_lock_addresses = ();

my $inconsistent_locks_counter = 0;
my $inconsistent_unlocks_counter = 0;

foreach my $line (<STDIN>)
{
    chomp $line;
    my $func = get_function_name($line);
    if(! defined $func || ! defined $lock_types{$func}) { next; }

    my $address = get_lock_address($line);
    if($lock_types{$func} eq "spin_lock_irqsave")
    {
        if($spin_lock_irqsave_addresses{$address})
        {
            log_inconsistent_lock($spin_lock_irqsave_addresses{$address});
            $inconsistent_locks_counter++;
        }
        $spin_lock_irqsave_addresses{$address} = $line;
    }
    elsif($lock_types{$func} eq "spin_unlock_irqrestore")
    {
        if(! $spin_lock_irqsave_addresses{$address})
        {
            log_inconsistent_unlock $line;
            $inconsistent_unlocks_counter++;
        }
        undef $spin_lock_irqsave_addresses{$address};
    }

    elsif($lock_types{$func} eq "mutex_lock" || $lock_types{$func} eq "mutex_trylock")
    {
        if($lock_types{$func} eq "mutex_trylock")
        {
            if(! get_trylock_result($line)) { next; }
        }
        if($mutex_lock_addresses{$address})
        {
            log_inconsistent_lock($mutex_lock_addresses{$address});
            $inconsistent_locks_counter++;
        }
        $mutex_lock_addresses{$address} = $line;
    }
    elsif($lock_types{$func} eq "mutex_unlock")
    {
        if(! $mutex_lock_addresses{$address})
        {
            log_inconsistent_unlock $line;
            $inconsistent_unlocks_counter++;
        }
        undef $mutex_lock_addresses{$address};
    }

}

foreach my $line (values %spin_lock_irqsave_addresses)
{
    if($line)
    {
        log_inconsistent_lock $line;
        $inconsistent_locks_counter++;
    }
}

foreach my $line (values %mutex_lock_addresses)
{
    if($line)
    {
        log_inconsistent_lock $line;
        $inconsistent_locks_counter++;
    }
}


if($inconsistent_locks_counter || $inconsistent_unlocks_counter)
{
    print "Locks are inconsistent.\n";
    print "Files \"$inconsistent_locks_filename\" and \"$inconsistent_unlocks_filename\" contains lists of inconsitent calls.\n";
    exit 1;
}

print "Trace is consistent.\n";
