#!/bin/bash
# tests the jacktrip version command

OUTPUTSTRING=$(command jacktrip -v)

#this regular expression match the word "version,"
#an optional colon and space(s), followed by at least one number
regExp='.*version\:?\s*([0-9]+).*'

echo "${OUTPUTSTRING,,}"
#converts the jacktrip -v command output to lower case
#for case insensitive matching and applies the regular expression to it
if [[ "${OUTPUTSTRING,,}" =~ $regExp ]]; then
  echo "version test passed"
else
  echo "version test failed"
fi
