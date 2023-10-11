#!/usr/bin/env perl

use strict;
use warnings;
use File::Temp qw/tempdir/;
use File::Spec::Functions qw/catfile/;

sub usage {
    print STDERR <<EOF;
Usage: $0 [options] <dim_0> <dim_1> <dim_2> ... <dim_n> <mode_1> ... <mode_n>

Creates n matrices with the given chain of dimensions

Options:
\t--dstdir\tput matrices in that directory (instead of a new subdirectory in /tmp)
\t--save-matrices-list\twrite the list of generated matrices to this file
\t--seed\tuse this random seed

The <mode_i> argument tells how the i-th matrix, of dimensions <dim_{i-1}> times <dim_i>, should be generated. Known modes are:
\tSIMPLESHIFTDOWN\tsub-diagonal matrix (truncated if rectangular)
\tSIMPLESHIFTRIGHT\tabove-diagonal matrix (truncated if rectangular)
\tSTRETCH\tstretch or shrink vector elements to the given dimension.
\tURANDOM<number>\trandomly filled rows with <number> elements per row
EOF
    exit 0 unless defined(my $msg = shift @_);
    print STDERR "\nError: $msg\n";
    exit 1;
}

my @dims=();
my @modes=();
my $dstdir;
my $matrices_list;

while (defined($_=shift @ARGV)) {
    if (/^\d+$/) {
        push @dims, $_;
        next;
    } elsif (/^[A-Z]+\d*$/) {
        push @modes, $_;
        next;
    } elsif (/--seed/) {
        defined(my $seed = shift(@ARGV)) or usage "missing argument to $_";
        $seed =~ /^\d+$/ or usage "bad seed: $seed";
        srand($seed);
    } elsif (/--dstdir/) {
        defined($dstdir = shift(@ARGV)) or usage "missing argument to $_";
    } elsif (/--save-matrices-list/) {
        defined($matrices_list = shift(@ARGV)) or usage "missing argument to $_";
    } elsif (/--help/) {
        usage;
    } else {
        usage "unknown argument $_";
    }
}

my $n = scalar @dims - 1;

usage "need at least 2 dimensions" if $n < 1;

if (!defined($dstdir)) {
    $dstdir = tempdir();
}

print "Creating matrices in $dstdir\n";

open(MLIST, ">$matrices_list") if defined $matrices_list;

for my $i (0..$n-1) {
    my $nrows = $dims[$i];
    my $ncols = $dims[$i+1];
    my $matrixname = "M$i";
    my $matrixpath = catfile($dstdir, $matrixname);
    my $mfile = $matrixpath . ".bin";
    my $rwfile = $matrixpath . ".rw.bin";
    my $cwfile = $matrixpath . ".cw.bin";
    print "Matrix $mfile has $nrows rows and $ncols columns\n";

    my @columns = (0) x $ncols;

    my $mode = shift @modes || 'SIMPLESHIFTRIGHT';

    open(MAT, "> :raw :bytes", $mfile);
    open(RW, "> :raw :bytes", $rwfile);
    for(my $rowid = 0 ; $rowid < $nrows ; $rowid++) {
        my @cols;

        if ($mode eq 'SIMPLESHIFTRIGHT') {
            if ($rowid + 1 < $ncols) {
                push @cols, $rowid+1;
            }
        } elsif ($mode eq 'SIMPLESHIFTDOWN') {
            if ($rowid >= 1) {
                push @cols, $rowid-1;
            }
        } elsif ($mode eq 'STRETCH') {
            my $j0 = int(($rowid * $ncols) / $nrows);
            my $j1 = int((($rowid+1) * $ncols) / $nrows);
            for(my $j = $j0 ; $j < $j1 ; $j++) {
                push @cols, $j;
            }
        } elsif ($mode =~ /URANDOM(\d+)/) {
            my $d = $1;
            for(my $j = 0 ; $j < $d ; $j++) {
                push @cols, int(rand()*$ncols);
            }
            @cols = sort @cols;
        }

        print RW pack("L", scalar @cols);
        print MAT pack("L*", (scalar @cols, @cols));
        for (@cols) {
            $columns[$_]++;
        }
    }
    close MAT;
    close RW;

    open(CW, "> :raw :bytes", $cwfile);
    print CW pack("L*", @columns);
    close CW;

    print MLIST "$mfile\n" if defined $matrices_list;
}

close MLIST if defined $matrices_list;