#!/bin/sh
#
#   COPYRIGHT 2018 UNIVERSITY OF ROCHESTER
#
# Verifies that the binary program file specified by the first command-line
# argument does not contain the instruction "wrpkru" (0x 0F 01 EF) anywhere
# in its .text segment.
#
# PRECONDITIONS:
#  * The helper script "opcode.awk" must be present in the current directory.
#  * You must have read/write access to the /tmp directory.
#
# LIMITATIONS:
#   Only scans the .text section of the file. If there is additional code
#   anywhere else, this won't scan it.
#

AWKSCRIPT="opcode.awk"

#
# Extract the .text section from the program as raw binary machine code.
#
# NOTE: the objcopy utility doesn't know how to print to stdout, so we must
# use a temporary file.
#
base=`basename $1`
objcopy_tmpfile=`mktemp /tmp/$base.XXXXXXXX`
echo "Using temporary file for objcopy output: $objcopy_tmpfile"
objcopy -O binary -j .text $1 $objcopy_tmpfile
echo "objcopy returned: $?"

#
# Scan the .text section for the "wrpkru" bytes using the Awk helper script.
#
# The "od" command prints out the raw binary machine code as a string of
# human-readable hexadecimal bytes. The Awk script then scans this for
# "0f 01 ef" (case-insensitive).
#
# If the "wrpkru" instruction is found, the Awk script will print a message
# indicating where in the file it was found.
#
echo "----------"
od -An -t x1 -v $objcopy_tmpfile | awk -f $AWKSCRIPT
echo "----------"
echo -n "If nothing was printed between the dashed lines, the 'wrpkru' "
echo "instruction was not found in this program."
echo "awk returned: $?"

#
# Delete the temporary file.
#
echo "Deleting temporary file..."
rm $objcopy_tmpfile
echo "rm returned: $?"

