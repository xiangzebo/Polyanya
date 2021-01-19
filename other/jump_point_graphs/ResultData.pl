#!/usr/bin/env perl

use strict;
use warnings;

use Class::Struct;
use Data::Dumper;

our($results_root, @stats, @algs, @benchmarks_preorder, @benchmarks_postorder, %stats_h, %algs_h, %benchmarks_h, $weighted_average);

my $precision = 4;

my $format_str = "%7.4f";

# Identifies the statistics.
struct Statistic => [
  desc => '$',  # Useful for reading the input
  abbr => '$',  # Useful for printing and referring to the statistics
  type => '$',  # (I)nstance count (unique), (M)ap, (Q)uery
  input_averaged => '$', # Whether the statistic is averaged or not TODO: remove input part
  table => '$',  # Whether to include the statistic in the table
  format => '$'
];

# Identifies the algorithms
struct Algorithm => [
  name => '$',  # Useful for reading the input
  abbr => '$'   # Useful for printing and referring to the algorithms
];

struct Benchmark => [
  name => '$',      # Name of the benchmark (could be a map, or a collection
                    # of maps (organized hierarchically)
  path => '$',      # The relative path of the benchmark.
                    # Read from results/path and written to summary/path
  vals => '%',      # Values in the benchmark, indexed by ($stat, $alg)
  children => '@',  # Children benchmarks in the hierarchy.
  inst_count => '$',# Number of instances in the benchmark.
  map_count => '$', # Number of maps in the benchmark.
  depth => '$'		# Depth in the hierarchy of benchmarks.
];

sub AddStatistic {
   my ($desc, $abbr, $type, $input_averaged, $table, $format) = (@_);
   push(@stats, Statistic->new(
    desc => $desc,
    abbr => $abbr,
    type => $type,
    input_averaged => $input_averaged,
    table => $table,
	format => $format));
   $stats_h{$abbr} = $stats[$#stats];
}

sub AddAlgorithm {
  my ($name, $abbr) = (@_);
  push(@algs, Algorithm->new(name => $name, abbr => $abbr));
  $algs_h{$abbr} = $algs[$#algs];
}

sub GetKey {
  my $stat_r = shift;
  my $alg_r = shift;
  
  return $stat_r->abbr . "---" . $alg_r->abbr;
}

sub GetStat {
  my $abbr = shift;
  for my $stat (@stats) {
    if ($abbr eq $stat->abbr) {
      return $stat;
    }
  }
  print "Stat $abbr not found!";
  return 0;
}

sub ReadBenchmarkFolder {
  my $benchmark = shift;
  my $results_folder = shift;
  my $stat_extension = shift;
  
  # For easy iteration over all benchmarks in the hierarchy of benchmarks later.
  push @benchmarks_preorder, $benchmark;

  # Read subdirectories.  
  my $rootdir = $results_folder . $benchmark->path;
  opendir my $dh, "$rootdir";
  my @subdirs = grep { -d "$rootdir/$_" && ! /^\.\.?$/} readdir ($dh);
  closedir $dh;

  # If subdirectory exists, recurse.
  if (@subdirs) {
    @subdirs = sort @subdirs;
    # For each subdirectory, create a new benchmark as a child.
    foreach my $subdir (@subdirs) {
      push(@{ $benchmark->children }, Benchmark->new(
        name => $subdir,
        path => $benchmark->path . '/' . $subdir,
        inst_count => 0,
        map_count => 0,
				depth => $benchmark->depth + 1));
    }
    
    # For each child benchmark, recurse.
    foreach my $cb (@{ $benchmark->children }) {
      ReadBenchmarkFolder($cb, $results_folder, $stat_extension);
    }
    
    # After the subtree is processed, create aggregate data.
    foreach my $cb (@{ $benchmark->children }) {
      $benchmark->inst_count($benchmark->inst_count + $cb->inst_count);
      $benchmark->map_count($benchmark->map_count + $cb->map_count);
    }
    
    foreach my $stat (@stats) {
    	#print $stat->abbr, "\n";
      foreach my $alg (@algs) {
        my $sum = 0;
				if (!$weighted_average) {
          foreach my $cb (@{ $benchmark->children }) {
            $sum += $cb->vals(GetKey($stat, $alg));
		  		}
	      	$benchmark->vals(GetKey($stat, $alg), $sum/(scalar @{$benchmark->children}));
				}
		
        elsif ($stat->type eq 'M') {
          foreach my $cb (@{ $benchmark->children }) {
            $sum += $cb->vals(GetKey($stat, $alg))*$cb->map_count;
          }
          $benchmark->vals(GetKey($stat, $alg), $sum/$benchmark->map_count);      
        }
        
        elsif ($stat->type eq 'Q' || $stat->type eq 'I') {
          foreach my $cb (@{ $benchmark->children }) {
            $sum += $cb->vals(GetKey($stat, $alg))*$cb->inst_count;
          }
          $benchmark->vals(GetKey($stat, $alg), $sum/$benchmark->inst_count);        
        }
        
        else {
          $benchmark->vals(GetKey($stat, $alg), $benchmark->inst_count);          
        }
      }
    }
  }
  
  # Otherwise, read the files for each algorithm.
  else {
    foreach my $alg (@algs) {
      my $resfile = $results_folder . $benchmark->path . '/' . $alg->name . $stat_extension;
#      print "$resfile \n";

      # Read the file for the algorithm and fill Benchmark->vals      
      open(STAT, "$resfile") or die "Cannot open file $resfile !\n";
      $benchmark->map_count(1);
      
      foreach my $line (<STAT>)  {
		    chomp $line;
		    foreach my $stat (@stats) {
		      my $description = $stat->desc;
      		if ($line =~ /$description.*:\s*(\S*)\s*$/) {     
      		  $benchmark->vals(GetKey($stat, $alg), $1);
      		  
      		  # Read the instance count.
      		  if ($stat->type eq 'I') {
      		    $benchmark->inst_count($1 + 0); # Just a hack to make it integer,
      		                                    # doesn't seem necessary at all.
      		  }
      		}
		    }
	    }
	    
	    foreach my $stat (@stats) {
	    	if (!(defined $benchmark->vals(GetKey($stat, $alg)))) {
	    		print $resfile, " : Cannot read stat ", $stat->abbr, " for algorithm ", $alg->name, "\n";
	    	}
	    }
	    
	    # For averaged results, replace it with sum.
	    #foreach my $stat (@stats) {
	    #  if ($stat->input_averaged) {
	    #    my $sum = $benchmark->vals(GetKey($stat, $alg)) * $benchmark->inst_count;
	    #    $benchmark->vals(GetKey($stat, $alg), $sum);  
	    #  }
	    #}
    }
  } 
}

sub ReadBenchmarks {
  my $results_folder = shift;
  my $stat_extension = shift;
  $results_root = Benchmark->new(name => 'all', path => '', inst_count => 0, map_count => 0, depth => 0);
  ReadBenchmarkFolder($results_root, $results_folder, $stat_extension);
  my @benchmarks_postorder = reverse @benchmarks_preorder;
}

sub CreateSubdirectories {
  my $root = shift;
  my $directories_at_leaf_nodes = shift || 0;
  
  system "mkdir -p $root";
  
  foreach my $b (@benchmarks_preorder) {
    my $directory = $root . $b->path;
    if ($directories_at_leaf_nodes == 1 || scalar @{$b->children} != 0) {
      #print "$directory\n";
      mkdir $directory;
    }
  }
}

sub PrintAlgStatTable {
  my $benchmark = shift;
  my $filename = shift;

  open(OUT, ">$filename");
  
  printf OUT "%-10s", "";
  foreach my $stat (@stats) {
    if ($stat->table == 1) {
      printf OUT "\t%10s", $stat->abbr;
    }
  }
  print OUT "\n";
  
  foreach my $alg (@algs) {
    printf OUT "%-10s", $alg->abbr;      
    foreach my $stat (@stats) {
      if ($stat->table == 1) {
#        my $str = sprintf "%6.*f", $precision, $benchmark->vals(GetKey($stat, $alg));
        my $str = sprintf $stat->format, $benchmark->vals(GetKey($stat, $alg));
        printf OUT "\t%10s", $str;
      }
    }
    print OUT "\n";
  }
  close OUT;
}

sub PrintAlgStatTables {
  my $directory = shift;
  CreateSubdirectories($directory);
  foreach my $benchmark (@benchmarks_preorder) {
    PrintAlgStatTable($benchmark, $directory . $benchmark->path . ".summary");
  }
}

sub PrintAlgorithmByMapStatSummaryTable {
  my $stat = shift;
  my $filename = shift;

  open(OUT, ">$filename");
  
  printf OUT "%-10s", "";
  foreach my $alg (@algs) {
    printf OUT "\t%10s", $alg->abbr;
  }
  print OUT "\n";
  
  foreach my $benchmark (@benchmarks_preorder) {
    if (scalar @{$benchmark->children} != 0) { # Don't print it for maps.
      my $pre = '';
	  for (my $i = 1; $i < $benchmark->depth; $i++) {
		$pre = $pre . " ";# "  ";
	  }
	  if ($benchmark->depth > 0) {
		$pre = $pre . " ";#"|-";
	  }
	  
	  $pre = $pre . $benchmark->name;
      printf OUT "%-10s", $pre;      
	  
#      printf OUT "%-10s", $benchmark->name;      
      foreach my $alg (@algs) {
        my $str = sprintf $stat->format, $benchmark->vals(GetKey($stat, $alg));
   #      $r_values->[$a][$d];
        printf OUT "\t%10s", $str;
      }
      print OUT "\n";
    }
  }
  close OUT;  
}

sub PrintAlgorithmByMapStatSummaryTables {
  my $directory = shift;
  mkdir $directory;
  
  foreach my $stat (@stats) {
#    print $stat;
    PrintAlgorithmByMapStatSummaryTable($stat, $directory . "/" .$stat->abbr);
  }
}

sub AddInverseRatioStat {
  my ($b_alg_abbr, $b_stat_abbr, $abbr, $table, $format) = (@_);
  my $b_alg = $algs_h{$b_alg_abbr};
  my $b_stat = $stats_h{$b_stat_abbr};
  
  AddStatistic("", $abbr, $b_stat->type, 0, $table, $format);
  my $new_stat = $stats_h { $abbr };
  
  foreach my $benchmark (@benchmarks_preorder) {
    my $base_val = $benchmark->vals(GetKey($b_stat, $b_alg));
    foreach my $alg (@algs) {
      my $alg_val = $benchmark->vals(GetKey($b_stat, $alg));
      
      my $new_val = 0;
      if ($alg_val > 0) {
        $new_val = $base_val / $alg_val;
      }
      
      $benchmark->vals(GetKey($new_stat, $alg), $new_val);       
    }
  }
}

sub AddStatCombination {
  my ($b_stat_abbr1, $b_stat_abbr2, $op, $abbr, $table, $format) = (@_);
  my $b_stat1 = $stats_h{$b_stat_abbr1};
  my $b_stat2 = $stats_h{$b_stat_abbr2};
  
  AddStatistic("", $abbr, $b_stat1->type, 0, $table, $format);
  my $new_stat = $stats_h { $abbr };
  
  foreach my $benchmark (@benchmarks_preorder) {
    foreach my $alg (@algs) {
      my $val1 = $benchmark->vals(GetKey($b_stat1, $alg));
      my $val2 = $benchmark->vals(GetKey($b_stat2, $alg));
      my $new_val = 0;

      if ($op eq '+') {
        $new_val = $val1 + $val2;
      }
      if ($op eq '-') {
        $new_val = $val1 - $val2;
      }
      if ($op eq '*') {
        $new_val = $val1 * $val2;
      }
      if ($op eq '/' && $val1 > 0) {
        $new_val = $val1 / $val2;
      }
      
      $benchmark->vals(GetKey($new_stat, $alg), $new_val);       
    }
  }
}

1;

