#!/bin/bash
# Run for 120 seconds
end=$((SECONDS + 120))

# SECONDS is an internal shell variable that tracks the seconds elapsed since
# this script was started
while [ $SECONDS -lt $end ]; do
    : # this command does nothing
done

echo "Done!"
