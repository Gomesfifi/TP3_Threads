#!/usr/bin/perl -w

use warnings;
use strict;
use autodie;
use Sys::Hostname;

my $fname= "mesures.csv";
my $tsp="../build/ensitsp";

my @allnthreads= (1, 2, 4, 8, 16, 32, 64);
my @allncities= (15..18);
my @allseeds= (12387, 12388);

die "Le fichier '${fname}' existe déjà !" if (-e $fname);

open my $fhh, ">", $fname;
print $fhh "date; machine; nvilles; graine; nthreads; longueur; temps; coupures\n";
close $fhh;

my $machine= hostname();

open my $fh, ">>", $fname;

for my $nthreads ( @allnthreads ) {
    for my $seed ( @allseeds ) {
	for my $ncities ( @allncities ) {
	    my $command="${tsp} ${ncities} ${seed} ${nthreads}";
	    print $command."\n";
	    open my $expe, "-|", $command;
	    my $date = localtime();
	    while ( my $line= <$expe> ) {
		chomp($line);
		my @val = split(" ",$line);
		print $fh join(";",$date,$machine,
				  $val[3],$val[6],$val[12],
				  $val[9],$val[15], $val[18])."\n"; 
	    }
	    close($expe)
	}
    }
}

close $fh;

# Genérer les figures
system("R --vanilla --no-save < ../mesures/visu.R");
