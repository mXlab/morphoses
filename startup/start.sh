#!/bin/sh

bash ~/morphoses/startup/logger1.sh >logger1.txt&
bash ~/morphoses/startup/logger2.sh >logger2.txt&
bash ~/morphoses/startup/logger3.sh >logger3.txt&
bash ~/morphoses/startup/behaviors.sh