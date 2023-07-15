#!/bin/sh
netcat -l -u -p 8000 | ./demod | aplay -f s16_le -r 8000


