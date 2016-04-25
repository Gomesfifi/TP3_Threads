#!/usr/bin/perl -w

use warnings;
use strict;
use autodie;
use Sys::Hostname;

use Test::Simple tests => 18;

my $tsp="../build/ensitsp -q";
my @valg=("", "valgrind -q --tool=drd","valgrind -q --tool=helgrind");

my @allnthreads= (1, 2, 4);
my @allncities= (15, 18);
my @allseeds= (12387);
my %solutions= ( 
    "15_12387" => 328, 
    "15_12388" => 286,
    "16_12387" => 343, 
    "16_12388" => 319,
    "17_12387" => 353, 
    "17_12388" => 322,
    "18_12387" => 356, 
    "18_12388" => 332,
);

my $machine= hostname();

for my $nthreads ( @allnthreads ) {
    for my $seed ( @allseeds ) {
	for my $ncities ( @allncities ) {
	    for my $valgrind ( @valg ) {
		my $command="${valgrind} ${tsp} ${ncities} ${seed} ${nthreads}";
		open my $expe, "-|", $command;
		my $date = localtime();
		while ( my $line= <$expe> ) {
		    if ($line =~ /coupures/) {
			chomp($line);
			my @val = split(" ",$line);
			my $nvilles = $val[3];
			my $graine = $val[6];
			my $nthreads = $val[12];
			my $longueur = $val[9];
			
			my $clef = "${ncities}_${seed}";
			my $attendu = $solutions{$clef};
			ok($longueur == $attendu, "${valgrind} ${ncities} ${seed} ${nthreads} $longueur == $attendu");
			
		    }
		}
		close($expe)
	    }
	}
    }
}

