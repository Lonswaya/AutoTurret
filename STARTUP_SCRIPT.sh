# This is the script that mounts a USB and uses it to automatically start any program specified by the startup script on the drive
# Useful for any embedded system where the executable will be compiled on another device, and no keyboard/monitor IO is needed
# In order to allow this script to start on boot, add the line "sh ~/AutoTurret/STARTUP_SCRIPT.sh &" to your /etc/rc.local
# After that, to allow execution permissons without logging in, ensure this script has execute access (sudo chmod -x ~/AutoTurret/STARTUP_SCRIPT.sh)
# Also, this script runs again if you log in, so TODO: do not run more than once for a boot
USB_PATH="/usb/media/"

# Mount the USB that we expect to find (bottom left on Raspberry Pi B
sudo mount -t vfat -o rw /dev/sda2 $USB_PATH
# If that doesn't work, try it again with the other possible place it is (hacky, I know)
sudo mount -t vfat -o rw /dev/sdb2 $USB_PATH


# run autorun script, if found in the attached USB
AUTORUN_FILENAME="autorun.sh"
AUTORUN_FILE="$USB_PATH$AUTORUN_FILENAME"
AUTORUN_LOG="/home/pi/AutoTurret/AUTORUN_LOG.txt"
DATE=$(date +"%y-%m-%d-%T")
SUCCESS_AUTORUNSTARTED="AUTORUN FOUND, BEGINNING TURRET SEQUENCE"
ERROR_NOTFOUND="NO AUTORUN FOUND, CANCELLING AUTO-TURRET SEQUENCE"

if [ -f $USB_PATH ]
then
        sh $AUTORUN_FILE
		echo $DATE $SUCCESS_AUTORUNSTARTED >> $AUTORUN_LOG
else
        if [ ! -f $AUTORUN_LOG ]
        then
                # Create log
                touch $AUTORUN_LOG
        fi
        echo $DATE $ERROR_NOTFOUND >> $AUTORUN_LOG
fi


