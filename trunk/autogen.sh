#!/bin/sh
echo "Running autoreconf..."
autoreconf -i || exit 1
echo "All done"
