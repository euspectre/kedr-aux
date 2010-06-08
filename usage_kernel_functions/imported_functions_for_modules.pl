my %functions = ();
for my $module (@ARGV)
{
    #Regular path to modprobe is /sbin/modprobe, but on some systems /sbin is not in $PATH
    my $module_path = `PATH=\$PATH:/sbin modprobe -l $module`;
    chomp $module_path;
    if($module_path eq "")
    {
        print(STDERR "Cannot find module $module.\n");
        next;
    }
    #On some systems `modprobe -l` return relative path.
    if($module_path =~ /^[^\/]/)
    {
        $module_path = "/lib/modules/`uname -r`/$module_path";
    }
    
    my @functions_local = `perl imported_functions.pl $module_path`;
    for my $function (@functions_local)
    {
        chomp $function;
        if($functions{$function})
        {
            $functions{$function} = $functions{$function} + 1;
        }
        else
        {
            $functions{$function} = 1;
        }
    }
}
my @functions_sorted = sort {$functions{$b} <=> $functions{$a}} keys %functions;
for my $function (@functions_sorted)
{
    printf("%-30s %d\n", $function, $functions{$function});
}