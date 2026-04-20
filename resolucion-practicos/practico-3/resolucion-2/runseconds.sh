#!/bin/bash
# Run for 120 seconds
end=$((SECONDS + 120))

while [ $SECONDS -lt $end ]; do
    : # this command does nothing
done

echo "Done!"
