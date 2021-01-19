#!/usr/bin/env perl

use warnings;
use strict;
use Data::Dumper;
use Chart::Gnuplot;
use List::Util qw[min max];

require "ResultData.pl";

our($results_root, @stats, @algs, @benchmarks_preorder, @benchmarks_postorder, %stats_h, %algs_h, %benchmarks_h, $weighted_average);

$weighted_average = 1;

# Input
my $results_directory = "experiments/results";
my $tables_directory = "experiments/tables";
my $stat_extension = ".stat";

# Output
my $tables_alg_stat_directory = "$tables_directory/alg-stat";
my $tables_stats_directory = "$tables_directory/stats";
my $tables_runtime_suboptimality_directory = "$tables_directory/runtime-subopt";

#my $tables_varying_rspc_directory = "$tables_directory/varying-RSPC";
#my $tables_varying_bound_directory = "$tables_directory/varying-bound";

# STATS
AddStatistic("Number of instances", "Inst", 'I', 0, 0, "%.0f");
#AddStatistic("Number of solutions", "Solved", 'C', 0);
#AddStatistic("Number of suboptimal solutions", "nSubopt", 'C', 0);
#AddStatistic("Average suboptimality", "Subopt", 'Q', 1);

AddStatistic("Preprocessing time", "PT", 'M', 0, 1, "%.2f");

AddStatistic("Memory", "Mem", 'M', 0, 1, "%.2f");
AddStatistic("Freespace memory", "A-Mem", 'M', 0, 0, "%.2f");
AddStatistic("Arc memory", "F-Mem", 'M', 0, 0, "%.2f");

AddStatistic("Average query time", "Query", 'Q', 1, 1, "%.4f");
AddStatistic("Average initialize time", "Connect", 'Q', 1, 1, "%.4f");
AddStatistic("Average search time", "Search", 'Q', 1, 1, "%.4f");
AddStatistic("Average finalize time", "Refine", 'Q', 1, 1, "%.4f");

AddStatistic("Average number of stalled nodes", "Stall", 'Q', 1, 1, "%.2f");
AddStatistic("Average number of expanded nodes", "Exp", 'Q', 1, 1, "%.2f");
AddStatistic("Average number of stalled or expanded nodes", "StallExp", 'Q', 1, 0, "%.2f");
AddStatistic("Average number of generated nodes", "Touch", 'Q', 1, 0, "%.2f");
AddStatistic("Average number of relaxed arcs", "Settled", 'Q', 1, 1, "%.2f");

AddStatistic("Number of nodes", "Nodes", 'M', 0, 1, "%.0f");
AddStatistic("Number of arcs", "Arcs", 'M', 0, 1, "%.0f");
AddStatistic("Number of core nodes", "Core nodes", 'M', 0, 0, "%.0f");
AddStatistic("Number of contracted nodes", "Contracted", 'M', 0, 0, "%.0f");
AddStatistic("Number of core arcs", "Core arcs", 'M', 0, 0, "%.0f");
AddStatistic("Number of ascending arcs", "Asc arcs", 'M', 0, 0, "%.0f");
AddStatistic("Number of same level local arcs", "Loc arcs", 'M', 0, 0, "%.0f");

AddStatistic("Number of shortcuts in hierarchy", "SCut", 'M', 1, 0, "%.0f");
AddStatistic("Number of shortcuts marked for R-refine", "R-Arcs", 'M', 1, 0, "%.0f");

AddAlgorithm("G_A", "A*");

#CH dijkstra
AddAlgorithm("CH_DSM", "CH-DM");
#AddAlgorithm("CH_ASM", "CH-M");
AddAlgorithm("CH_ASP", "CH");
#AddAlgorithm("CH-R_CFR_ASP", "CH-R");

#AddAlgorithm("RCH_CFR_AS", "RCH");
#AddAlgorithm("RCH_FR_AS", "RCH-F");

#AddAlgorithm("SUB_SG_A"           , "SG");
AddAlgorithm("SUB_SG_AdC"         , "SG-dC");

AddAlgorithm("SUB-CH_SG_ASP"      , "SG-CH");
#AddAlgorithm("SUB-CH_SG_ASPdC"      , "SG-CH-dC");
AddAlgorithm("SUB-CH-R_SG_CFR_ASP", "SG-CH-R");
#AddAlgorithm("SUB-CH-R_SG_CFR_ASPdC", "SG-CH-RdC");
#AddAlgorithm("SUB-RCH_SG_CFR_AS"  , "SG-RCH");
#AddAlgorithm("SUB-RCH_SG_FR_AS"   , "SG-RCH-F");
AddAlgorithm("SUB-N_SG_CFR_AS"    , "N-SG");
AddAlgorithm("SUB-N_SG_FR_AS"     , "N-SG-F");

#AddAlgorithm("CH-SL_SG_ASP"       , "CH-SL");

#AddAlgorithm("SUB_JPDgM_A"        , "JPD");
#AddAlgorithm("SUB_JPDgM_AdC"      , "JPD-dC");
#AddAlgorithm("SUB-CH_JPDgM_ASP"   , "JPD-CH");
#AddAlgorithm("SUB-CH_JPDgM_ASPdC" , "JPD-CH-dC");


#AddAlgorithm("SUB_JP_A"           , "JP");
#AddAlgorithm("SUB_JP_AdC"         , "JP-dC");
#AddAlgorithm("SUB-CH_JP_ASP"      , "JP-CH");
#AddAlgorithm("SUB-CH_JP_ASPdC"    , "JP-CH-dC");



ReadBenchmarks($results_directory, $stat_extension);

AddInverseRatioStat("A*", "Query", "SpUp-Astr", 1, "%.2f");
#AddInverseRatioStat("CH", "Query", "SpUp-CH", 0);
#AddInverseRatioStat("CH", "Mem", "Mem-CH", 0);
#AddStatCombination("Connect", "Search", '+', "DQuery", 1);
#AddInverseRatioStat("CH", "DQuery", "DSpUp-CH", 0);
#AddStatCombination("SpUp-CH", "Mem-CH", '*', "MemQT-CH", 0);
#AddStatCombination("DSpUp-CH", "Mem-CH", '*', "DMemQT-CH", 0);
#AddInverseRatioStat("CH", "Refine", "RSp-UP-CH", 0);

PrintAlgStatTables($tables_alg_stat_directory);
PrintAlgorithmByMapStatSummaryTables($tables_stats_directory);

