#!/bin/sh
netcat -l -u -p 8000 | ./app | aplay -f s16_le -r 8000


