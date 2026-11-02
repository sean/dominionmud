#!/bin/sh
#
# CircleMUD 3.0 automaint script
# Originally by Fred C. Merkel
# Copyright (c) 1993 The Trustees of The Johns Hopkins University
# All Rights Reserved
# See license.doc for more information
#
# Run from the top of the source tree with the server stopped.  Deletes
# idle, never-played and deleted characters from their accounts (see
# util/purgeplay.c) and logs who went to lib/etc/DELETED.

echo "automaint cleaning player files: `date`" >> lib/etc/DELETED
bin/purgeplay -d lib/accounts >> lib/etc/DELETED
