!/bin/bash
#
# Examples:
#  (1) when starting from ssh shell:
#		nohup /path/to/watchdog.sh >> /path/to/test.log &
#  (2) when starting from /etc/rc.local:
#		/path/to/watchdog.sh >> /path/to/test.log &
 
HPATH=$PWD/aincrad.py
echo "$HPATH"

while [ 1 ]
do
COUNT=`ps -ef | grep aincrad | grep -v grep | wc -l`
#echo "$COUNT"
 
if [ "$COUNT" -eq "1" ]
then
echo " Okay. aincrad still running at `date`"
else
echo " FAIL. aincrad stalled. Restarting at `date`"
exec python $HPATH &
fi
 
sleep 2
#./aincrad
done