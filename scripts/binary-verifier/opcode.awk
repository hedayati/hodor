#
#   COPYRIGHT 2018 UNIVERSITY OF ROCHESTER
#
# Helper script for "verify-binary.sh". Checks that a hex dump of a program's
# .text section is free of the instruction "wrpkru" (0x 0F 01 EF).
#
# Input is expected to be in the format produced by the command:
#   od -An -t x1 -v <input file containing raw machine code>
# ...namely, hexadecimal bytes separated by spaces and grouped by newlines
# (we don't care how long the lines are), with a leading space at the
# beginning of each line. For example:
#   01 02 03 04 05 06 07
#   08 0a 0b 0c 0d 0e 0f
# Either uppercase or lowercase hexadecimal digits are accepted. ("od" uses
# lowercase letters on my system, but I don't know if that's consistent.)
#

# Use a space, or a newline followed by a space, as record separator
BEGIN {
  RS = "( )|(\n )"
}

# This action (block of statements) executes for every record encountered,
# i.e. every byte in the hexdump.
{
  # Keep track of the last three bytes seen
  # Normalize hexadecimal format to use lowercase letters
  prev3 = tolower(prev2);
  prev2 = tolower(prev1);
  prev1 = tolower($0);

  # If we see this sequence of bytes:
  #   0f 01 ef
  # we have found the instruction "wrpkru".
  if (prev3 == "0f" && prev2 == "01" && prev1 == "ef")
    printf "WRPKRU (0f 01 ef) found: bytes %d-%d\n", NR-4, NR-1
}
