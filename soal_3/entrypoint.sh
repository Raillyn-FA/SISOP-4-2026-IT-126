#!/bin/bash

mkdir -p /logs
touch /logs/libraryit.log

chmod -R 775 /libraryit/ebooks
chmod -R 775 /libraryit/papers

chmod -R 750 /libraryit/sourcecode

chmod -R 770 /libraryit/docs

chown -R root:staff /libraryit

tail -f /logs/libraryit.log &
smbd --foreground --no-process-group
