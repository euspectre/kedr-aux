my %functions = ();
# If system store only gz-archive of module, extract this module here.
my $temp_module_path = "temp.ko";
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
    
    my $path_is_archive = "";

    if($module_path =~ /\.gz$/)
    {
        $path_is_archive = "yes";
        system("gunzip -c $module_path > $temp_module_path");
        $module_path = $temp_module_path;
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
    if($path_is_archive)
    {
        system("rm -f $temp_module_path");
    }
    
}
my @functions_sorted = sort {$functions{$b} <=> $functions{$a}} keys %functions;
for my $function (@functions_sorted)
{
    printf("%-30s %d\n", $function, $functions{$function});
}