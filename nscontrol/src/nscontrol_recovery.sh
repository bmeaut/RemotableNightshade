#!/bin/bash

# Script is run in the case of Nightshade crashes and on upgrades that can not currently be handled cleanly. 
# Named semaphores around the database are removed by the cleanupsem utility and the shared memory file
# is removed. To avoid duplicate DB enteries when Nightshade restarts, the DB file is also removed.

/sbin/service lighttpd stop  
/usr/bin/cleanupsem NShadeDB
rm /dev/shm/nightshadeSM
rm /dev/shm/NShadeDB.fdb
/sbin/service lighttpd start

exit 0


