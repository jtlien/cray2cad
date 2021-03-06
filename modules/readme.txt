

  The Cray-2 was designed on a module basis.  A modules is described by a
three primary files - a boolean source file (for example jabol, the name of
the file was formed by the two character module name + bol).  The boolean file
is a set of boolean equations that logically describe the module.  The 
second file was the package file (for example japkg).   The package file
has the format of a line for each chip location in the module.  Modules were
made of a stack of eight boards.   The first two characters of each line
of the package file describe the location of a chip.  Chips on the top board
had a location that begins with A,B or C.   The next board D,E, or F, the next
board G,H,or I on down to the last board that has locations beginning with
V,W or X.   Each board has locations for 96 chips (32 beginning with each of
3 characters).   The second characters of the location was one of A-X,1-8 to
specify one of the 32 possible locations.   Three sets of 32 locations gives
96 locations of each board.  The location is followed by a dash and the
dash is followed by a list of the outputs (boolean terms in the boolean file)
that are outputs at that location.
  The third file is the connector file (for example jacon).  It describes the
three 12 pin (24 differential pair) connectors on each board of the module.
ZA,ZB and ZC are on the top board.  ZD,ZE and ZF are on the next board etc.
Each of the three connectors on each board has 12 pins (24 differential pairs)
of output or input signals.
  The parspin program can convert these files into the source files for
KICAD and verilog outputs for simulation.  The package file is converted first
into a mapped package file using pkgmap.

          pkgmap japkg japkgout

          parspin japkgout jabol jacon jabrc


    The jabrc file in this example is the reconstructed boolean output file.
It shows show the boolean equations are mapped to the chips on the board.

   The parspin program outputs verilog output for the module and kicad files
 for creating board artwork.


