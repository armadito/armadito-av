#!/usr/bin/perl
use Getopt::Long;
use Data::Dumper;

sub Exec{
 	 my $cmd = shift;
         print "Command: $cmd\n";
	 
	 my $res = `$cmd 2>&1`; 
	 # print $res."\n";
         my $exitvalue = `echo -n $?`;
	 unless($exitvalue eq 0){
                print "Error when executing \"$cmd\" ; exit value = $exitvalue \n";
		exit(1);
	}
	return $res;
}

sub ParseConf{

        my $Path = shift;
	my $whole_conf = ();
	
	open(FILE, $Path) or die "Can't open $Path : $!";
	
	foreach my $line (<FILE>) 
	{
		if( $line !~ /^\s+$/ms && $line !~ /^\s*\#/ms) 
		{
			$line =~ s/\s+$//msg;
			my @vars = split(";",$line);
			my $conf = {
			    project_name => $vars[0],
			    license => $vars[1],
                            path => $vars[2],
			};

			push(@$whole_conf, $conf);
		}
	}
	
	close(FILE);
	return $whole_conf;
}

sub FindSourcesFiles{
        my $dir = shift;
	
	die "$dir not found." unless(-d $dir);	

	my $cmd = 'find '.$dir.' -regex ".*\.\(c\|h\|cpp\|java\|js\)"';
	my $res = &Exec($cmd);
	my @files = split(/\n/, $res);
	
	return \@files;
}


sub GetHeaderForLicense
{
   my $license = shift;
   my $project_name = shift;    
   my $copyright = shift;
   my $header = "";

   if( $license eq "GPLv3" ){
     
$header = 
"\/\*\*\*

$copyright

This file is part of $project_name.

$project_name is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

$project_name is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with $project_name.  If not, see <http://www.gnu.org/licenses/>.

\*\*\*\/\n\n";

   }elsif( $license eq "LGPLv3" ){

$header = 
"\/\*\*\*

$copyright

This file is part of $project_name.

$project_name is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

$project_name is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with $project_name.  If not, see <http://www.gnu.org/licenses/>.

\*\*\*\/\n\n";

   }
   elsif( $license eq "MSPL" ){
	$header = 
"\/\*\*\*

$copyright

This file is part of $project_name.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

\*\*\*\/\n\n";

   }

   return $header;
}

# Memory hungry, but this is just a fast written-script
sub InsertHeader
{
   my $Path = shift;
   my $header = shift;
   my $buffer = "";

   open(FSRC, "<".$Path) or die "Can't open $Path : $!";
   binmode FSRC;
   $buffer = do { local $/; <FSRC> };
   close(FSRC);

   $buffer =~ s/\/.*?ARMADITO PDF ANALYZER.*?\*\///ms;
   $buffer =~ s/^\/\*.*?This file is part of $project_name.*?\*\/\n\n//msg;

   $buffer = $header.$buffer;

   open(FOUT, ">".$Path) or die "Can't open $Path : $!";
   binmode FOUT;
   print FOUT $buffer;
   close(FOUT);
}

my $conf_file = "a6o_gpl_headers.conf";
my $verbose = 1;

GetOptions ("conf=s" => \$conf_file,
      "verbose"  => \$verbose)
or die("Error in command line arguments\n");

print "conf=$conf v=$verbose\n" if($verbose);

# https://www.gnu.org/licenses/gpl-howto.html
# LGPLv3 pour core/
# GPLv3 pour modules/clamav
# GPLv3 pour modules/moduleH1
# GPLv3 pour modules/modulePDF
# GPLv3 pour gui/
# MSPL https://opensource.org/licenses/MS-PL

my $headers = {
  "GPLv3" => "",
  "LGPLv3" => "",
  "MSPL" => ""
};

my $copyright = 'Copyright (C) 2015, 2016 Teclib\'';

my $conf = &ParseConf($conf_file);
#print Dumper($conf)."\n" if($verbose);

foreach my $c (@$conf)
{
   my $sources = &FindSourcesFiles($c->{path});
   foreach my $src (@$sources)
   {   
       if($src =~ m/languages\/.*\.js$/msi || $src =~ m/Gruntfile.js$/msi)
       {
         print "Exception : $src\n";
	 next;
       }

       &InsertHeader($src, GetHeaderForLicense($c->{license}, $c->{project_name}, $copyright));
   }
}

#print $files."\n";
