#!/usr/bin/perl
use Cwd;

##  -----------------------------------------------------------------------------
##  FILE NAME        : convert_fbcfg_to_cpuwrites.pl
##  PROJECT          : Tamar
##  DESCRIPTION      : Script to convert fabric configuration into APB's writes
##  -----------------------------------------------------------------------------
##  copyright (c) 2015, QuickLogic Corporation
##  -----------------------------------------------------------------------------
##  HISTORY:
##  DATE        VERSION     AUTHOR      DESCRIPTION
##  2015/01/08  0.1         jshen       Initial version
##  -----------------------------------------------------------------------------

# usage file
$help  = sprintf("Usage: convert_fbcfg_to_cpuwrites.pl <-c|uvm|r|flash> <fabric_cfg_file>\n");
$help .= sprintf(" Script to convert Fabric config into various formats\n");
$help .= sprintf(" Options: -c     : Generate C writes\n");
$help .= sprintf("          -uvm   : Generate UVM writes\n");
$help .= sprintf("          -r     : Generate UVM reads\n");
$help .= sprintf("          -flash : Generate flash image (content only, without addr)\n");
$help .= sprintf("          -spi   : Generate spi writes \n");
$help .= sprintf("          -swd   : Generate SWD writes \n");
$help .= sprintf("          -pifwr : Generate PIF writes \n");
$help .= sprintf("          -pifrd : Generate PIF reads \n");
$help .= sprintf("          -ffeat : Generate FFEAT writes \n");
$help .= sprintf("          -h     : Generate .h file \n");

if ($#ARGV !=1 ) {
    print $help;
    exit;
}

$op = shift(@ARGV); # expecting -c or -uvm
$infile = shift(@ARGV);
my $pwd = cwd(); #-- get current directory
&parse_fabric_config($infile);
if (! $op eq "-ffeat" ) {
&print_header($infile);
}
if ($op eq "-uvm") {
    &print_uvm_writes($infile);
} elsif ( $op eq "-c" ) {
    &print_c_writes($infile);
} elsif ( $op eq "-h" ) {
    &print_h_file($infile);
} elsif ( $op eq "-r" ) {
    &print_uvm_reads($infile);
} elsif ( $op eq "-flash" ) {
    &print_flash_image($infile);
} elsif ( $op eq "-spi" ) {
    &print_spi_writes($infile);
} elsif ( $op eq "-swd" ) {
    &print_swd_writes($infile);
} elsif ( $op eq "-pifwr" ) {
    &print_pif_writes($infile);
} elsif ( $op eq "-pifrd" ) {
    &print_pif_reads($infile);
} elsif ( $op eq "-ffeat" ) {
    &print_ffeat_writes($infile);
} elsif ( $op eq "-jlink" ) {
    &print_jlink_writes($infile);
}else {  # unrecognized option, print the help
    print $help;
    exit;
}

# global perl variables used by all subroutines
%cfg;  # global hash table to hold the config bits
%apb;  # global hash table to hold the apb writes
%apbr; # global hash table to hold the apb reads

# parse the fabric configuration into a hash table
sub parse_fabric_config {
 my($infile) = @_;
 my($wl);
 my($bl);
 my($i,$j,$k);
 open INFILE, $infile || die "CANNOT OPEN $infile!";
 while(<INFILE>) {
     if(/^\s*task get_wl_pat(\d+)/) { # find the wordline
	 $wl = $1;
	 #print "found wl = $wl\n";
     }
     if(/cfgsin(\d+)/) { # get the shift register
	 $bl = $1;
	 if(/45\s*'b([Xx01]+)/) {  # get the 45 bits
	     $bs = $1;
	     $cfg{$wl}{$bl} = $bs;
	     #print "bs = $bs\n";
	     #print "found wl = $wl, $bl, $cfg{$wl}{$bl}\n";
	     #cfgsin0[0:44] =  45 'bxxx0010x0110x011000110000010x0110x01100011000;
	 }
     }
 } 
 close INFILE;

# parse the cfg hash and covert to apb hash
 for $k (0..44) { # for each bit column    
     for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	 my($val) = "";
	 my($rval) = "";
	 for ( $j=31; $j>=0; $j-- ) {  # create the first 32 bit word

	     # for config writes
	     $bb = substr($cfg{$i}{$j},$k,1);  # concatenate the next bit to the string
	     $bb =~ s/x/0/g; # convert all x into 0
	     $val .= $bb;  # concentate next bit onto 32-bit value

	     # for config reads
	     # banks 0,8,16,24 only has 43 bits, so the first two bits are discarded when doing config reads
	     if ( $j == 0 || $j == 8 || $j == 16 || $j == 24 ) {
		 if ( $k==44 || $k ==43) {
		     $bb = "1"; # assume bit is 1 then - dummy bit
		 } else {
		     $bb = substr($cfg{$i}{$j},($k+2),1);  # next bit for string, adjusted by 2
		 }
	     } else {
		 $bb = substr($cfg{$i}{$j},$k,1);  # concatenate the next bit to the string
	     }
	     $bb =~ s/x/0/g; # convert all x into 0
	     $rval .= $bb;  # concentate next bit onto 32-bit value

	 }     
	 $hexval = unpack ("H*", pack ("B*", $val));
	 $hexrval = unpack ("H*", pack ("B*", $rval));
	 $apb{$i}{$k} = $hexval; # save the 32-bit value into a new hash for write configs
	 $apbr{$i}{$k} = $hexrval; # save the 32-bit value into a new hash for read configs
#	 printf "wl=$i, k=$k %08d , $val \n", $apb{$i}{$k},$val;
#	 printf "wl=$i, k=$k %08d , $rval \n", $apbr{$i}{$k},$rval;
     }
 }

}


# subroutine to print pretty header
sub print_header {
    printf "// generated on %s : convert_fbcfg_to_cpuwrites.pl %s/%s\n",&getDate(), $pwd, $infile;
}


# subroutine to print out the corresponding UVM writes
sub print_uvm_writes {
    my($i,$k);
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\/\/wordline = $i\n";
	printf "\t`uvm_info(get_type_name(),\"Loading wordline = %0d\",UVM_MEDIUM);\n", $i;
	for $k (0..44) {
	    printf "\tahb_write(32'h40014ffc, 32'h%s); \/\/ wl=%d, bit=%d;\n",$apb{$i}{$k},$i,$k;
	}	
    }

}

# subroutine to print out the corresponding SPI writes
sub print_spi_writes {
    my($i,$k);
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\/\/wordline = $i\n";
	printf "\t`uvm_info(get_type_name(),\"Loading wordline = %0d\",UVM_MEDIUM);\n", $i;
	for $k (0..44) {
	    printf "\tspitlc_ahb_write_seq(32'h40014ffc, 32'h%s); \/\/ wl=%d, bit=%d;\n",$apb{$i}{$k},$i,$k;
	}	
    }

}

# subroutine to print out the corresponding SWD writes
sub print_swd_writes {
    my($i,$k);
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\/\/wordline = $i\n";
	printf "\t`uvm_info(get_type_name(),\"Loading wordline = %0d\",UVM_MEDIUM);\n", $i;
	for $k (0..44) {
	    printf "\tswd_write_addr(32'h40014ffc, 32'h%s); \/\/ wl=%d, bit=%d;\n",$apb{$i}{$k},$i,$k;
	}	
    }
}


# subroutine to print out the corresponding UVM reads
sub print_uvm_reads {
    my($i,$k);
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\/\/wordline = $i\n";
	printf "\t`uvm_info(get_type_name(),\"Reading wordline = %0d\",UVM_MEDIUM);\n", $i;
	for $k (0..44) {
	    printf "\tahb_compare(32'h40014ffc, 32'h%s); \/\/ wl=%d, bit=%d;\n",$apbr{$i}{$k},$i,$k;
	}	
    }
}


# subroutine to print out the corresponding C writes
sub print_c_writes {
    my($i,$k);

    # print setup in the C test
    # printf "\n  PMU_General_Purpose_0 = 0x200; // set APB_FB_EN =1 for configuration\n";
    # printf "  CFG_CTL_CFG_CTL = 0xbdff; // set CFG_CTL for full configuration\n" ;

    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\n\/\/wordline = $i\n";
	for $k (0..44) {
	    printf "  CFG_CTL_CFG_DATA = 0x%s; \/\/ wl=%d, bit=%d;\n", $apb{$i}{$k},$i,$k;
	}	
    }

    #printf "\n  PMU_General_Purpose_0 = 0x0; //set APB_FB_EN = 0 for normal mode\n\n"; 

}

# subroutine to print out a .h file
sub print_h_file {
    my($i,$k, $l);
	print_header();

	print "uint32_t\taxFPGABitStream[] = {\n\t";
	
	$l = 0;
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
		for $k (0..44) {
			printf " 0x%s", $apb{$i}{$k};
			if ($i != 0 || $k != 44) {
				print ",";
			}
			$l = $l+1;
			if ($l == 10) {
				print "\n\t";
				$l = 0;
			}
		}	
    }

    print "\n};\n";

}

# subroutine to print out the corresponding PIF writes
sub print_pif_writes {
    my($i,$k);

    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\n\/\/wordline = $i\n";
	print " write_pif_byte( 8'h00 ); \/\/ send the write command 0x00\n";
	print " write_pif_2byte( 16'h002d ); \/\/ send the byte count = 0x002d \n";
	for $k (0..44) {
	    my $aa = $apb{$i}{$k};
	    my $aa_hi = substr($aa,0,4) ; # upper 16 bits of 32 bit value
	    my $aa_lo = substr($aa,4,4) ; # lower 16 bits of 32 bit value
	    printf " write_pif_2byte(16'h%s); \/\/  (hi) wl=%d, bit=%d;\n", $aa_hi, $i, $k;
	    printf " write_pif_2byte(16'h%s); \/\/  (lo) wl=%d, bit=%d;\n", $aa_lo, $i, $k;
	}	
	print " end_pif_write();\n";
    }
    # extra pif clocks to close the programming sequence
    print " end_pif_write();\n";
    print " end_pif_write();\n";
    print " end_pif_write();\n";

}

# subroutine to print out the corresponding PIF reads
sub print_pif_reads {
    my($i,$k);

    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\n\t\n\/\/wordline = $i\n";
	print " write_pif_byte( 8'h01 ); \/\/ send the read command \n";
	print " write_pif_2byte( 16'h002d ); \/\/ send the byte count = 0x002d \n";
	for $k (0..44) {
	    printf " read_pif_word(32'h%s); \/\/  wl=%d, bit=%d;\n", $aa = $apbr{$i}{$k}, $i, $k;
	}	
    }
}


# subroutine to print out the corresponding FFEAT writes
sub print_ffeat_writes {
    my($i,$k);

    # print setup in the FFEAT test

    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	for $k (0..44) {
	    printf "WRITE 0x40014ffc 0x%s\n", $apb{$i}{$k};
	}	
    }


}

# subroutine to print out the corresponding FFEAT writes 
sub print_jlink_writes {
    my($i,$k);

	printf "w4 0x40004c4c 0x00000120
	w4 0x40004610 0x00000002
	w4 0x40004088 0x0000003f
	w4 0x40004044 0x00000007
	w4 0x4000404c 0x00000006
	w4 0x40004064 0x00000001
	w4 0x40004070 0x00000000
	w4 0x4000411c 0x00000006
	w4 0x40005310 0x1acce551
	w4 0x40004054 0x00000001
	sleep 100
	w4 0x40014000 0x0000bdff
    sleep 100\n";
			
                                     
    # print setup in the FFEAT test  

    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	for $k (0..44) {
	    printf "w4 0x40014ffc, 0x%s\n", $apb{$i}{$k};
		if ($k eq 43) {
			printf "mem32 0x40014000, 0x1\n";
		}
	}
	printf "mem32 0x40014000, 0x1\n";	
    }
	
	printf "mem32 0x40014000, 0x1\n";
	printf"sleep 100
	mem32 0x40014000, 0x1
	w4 0x40014000, 0x00000000
	mem32 0x40014000, 0x1
	w4 0x400047f0, 0x00000000
	sleep 100\n";
			
	printf"w4 0x400047f4, 0x00000000
	w4 0x40004088, 0x0
	w4 0x40004094, 0x0
	w4 0x400047f8, 0x00000090
	w4 0x40004040, 0x0000029F
	w4 0x40004048, 0x00000001
	w4 0x4000404c, 0X0000003f
	w4 0x40005484, 0X000008c7
	w4 0x40004020, 0X00000202
	w4 0x40004034, 0X00000210
	w4 0x40004064, 0X00000001
	w4 0x40004070, 0X00000001
	w4 0x40004C10 0x110
	w4 0x40004C14 0x110
	w4 0x40004C18 0x110
	w4 0x40004C28 0x110
	w4 0x40004C2C 0x110
	w4 0x40004C30 0x110
	w4 0x40004D80 0xC001C70
	mem32 0x40004D80 1
	sleep 100
	w4 0x40004c4c 0x000009a0
	sleep 100";	


}
# subroutine to print out the corresponding flash image
sub print_flash_image {
    my($i,$k, $j);
    printf "\/\/This flash image contains %d bytes $i\n", 422*45*4;
    for ($i=421; $i>=0; $i--) {  # count from 421 down to 0 word lines
	print "\/\/wordline = $i\n";
	for $k (0..44) {
	    for ($j=3; $j>=0; $j--) {
  	        printf "%s", (substr $apb{$i}{$k}, $j*2, 2 );
                if ($j eq 3) { 
  	            printf "  \/\/ wl=%d, bit=%d;\n", $i,$k;
                } else {
  	            printf "\n";
	        }	
	    }	
	}	
    }
}

# routine to getDate
sub getDate {

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
    my $nice_timestamp = sprintf ( "%04d/%02d/%02d %02d:%02d",
                                   $year+1900,$mon+1,$mday,$hour,$min);
    return $nice_timestamp;
}

