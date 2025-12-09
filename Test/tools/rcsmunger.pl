#!/usr/bin/env perl -p
#
# @(#)$Id: rcsmunger.pl,v 1.9 2015/11/02 23:54:32 jleffler Exp $
#
# Remove the keywords around the values of RCS keywords

use strict;
use warnings;

# Beware of RCS hacking at RCS keywords!
# Convert date field to ISO 8601 (ISO 9075) notation
s%\$(Date:) (\d\d\d\d)/(\d\d)/(\d\d) (\d\d:\d\d:\d\d) \$%\$$1 $2-$3-$4 $5 \$%go;
# Remove keywords
s/\$([A-Z][a-z]+|RCSfile): ([^\$]+) \$/$2/go;
