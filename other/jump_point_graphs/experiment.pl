#!/usr/bin/env perl

my $exec = "./SG-Grid";

sub GetMaps {
  my $mappath = shift;

	my @maps = ();
	opendir(DIR,"$mappath") || "Can't open dir";
	@FILES= readdir(DIR);
	foreach my $filename (@FILES) {
		chomp $filename;
		if($filename =~ /^(.*)\.map$/){
			my $mapname = $filename;
			push(@maps, "$mapname");
		}
	}

	closedir(DIR);
	
	@maps = sort @maps;
	return @maps;
}

sub GetSubdirectoryNames {
	my $rootdir = $_[0];
  opendir my $dh, "$rootdir";
  my @subdirs = grep { -d "$rootdir/$_" && ! /^\.\.?$/} readdir ($dh);
  closedir $dh;
	return @subdirs;
}

sub RunExperiment {
	my $map = shift;
	system "$exec $map";	
  my $num = $? >> 8;
  
  my $i = 1;
  while ($num > 0) {
  	system "$exec $map $i";	
    $num = $? >> 8;
    $i ++;  
  }
}

sub RunBenchmark {
  my $benchmark = shift;
  my @maps = GetMaps("$benchmark");
  foreach $map (@maps) {
	  RunExperiment("$benchmark/$map");
  }
  
  my @subdirs = GetSubdirectoryNames($benchmark);
  if (@subdirs) {
      foreach my $dir (@subdirs) {
      RunBenchmark("$benchmark/$dir");
    }
  }
}

RunBenchmark("maps");
#RunBenchmark("maps/movingai");
#RunBenchmark("maps/GPPC");




