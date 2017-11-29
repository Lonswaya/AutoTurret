# This is the script that mounts a USB and uses it to automatically start any program specified by the startup script on the drive
# Useful for any embedded system where the executable will be compiled on another device, and no keyboard/monitor IO is needed
# In order to allow this script to start on boot, add the line "sh ~/AutoTurret/STARTUP_SCRIPT.sh &" to your /etc/rc.local
# After that, to allow execution permissons without logging in, ensure this script has execute access (sudo chmod a+x STARTUP_SCRIPT.sh)
USB_PATH="/usb/media/"
USB_PREMOUNT_PATH1="/dev/sda2"
USB_PREMOUNT_PATH2="/dev/sdb1"
USB_PREMOUNT_PATH3="/dev/sdb2"
AUTORUN_LOG="/home/pi/AutoTurret/AUTORUN_LOG.txt"
ERROR_NOUSB="NO USB FOUND/REGISTERED, RUNNING DEFAULT STARTUP SCRIPTS"
DATE=$(date +"%y-%m-%d-%T")
START_HAMACHI_COMMAND="sh /home/pi/AutoTurret/extras/start_hamachi.sh"
# We have to log in as a user in order to use vlc
START_CAMERA_COMMAND="sudo su pi -c '/home/pi/AutoTurret/extras/start_camera.sh'"

echo "INITIALIZING STARTUP SCRIPT"	


if [ ! -e $AUTORUN_LOG ]
then
	# Create log
    touch $AUTORUN_LOG
fi

# Mount the USB that we expect to find (bottom left on Raspberry Pi B
if [ -e $USB_PREMOUNT_PATH1 ]
then
	sudo mount -t vfat -o rw $USB_PREMOUNT_PATH1 $USB_PATH
elif [ -e $USB_PREMOUNT_PATH2 ]
then
	sudo mount -t vfat -o rw $USB_PREMOUNT_PATH2 $USB_PATH
elif [ -e $USB_PREMOUNT_PATH3 ]
then
	sudo mount -t vfat -o rw $USB_PREMOUNT_PATH3 $USB_PATH
else
	echo $DATE $ERROR_NOUSB >> $AUTORUN_LOG
	# DEFAULT STARTUP SCRIPTS
	sh $START_HAMACHI_FILE
	sh $START_CAMERA_FILE
	exit

fi


# run autorun script, if found in the attached USB
AUTORUN_FILENAME="autorun.sh"
AUTORUN_FILE="$USB_PATH$AUTORUN_FILENAME"
SUCCESS_AUTORUNSTARTED="AUTORUN FOUND, BEGINNING AUTORUN SEQUENCE"
ERROR_NOTFOUND="NO AUTORUN FOUND, CANCELLING AUTORUN SEQUENCE"
if [ -e $AUTORUN_FILE ]
then
        sh $AUTORUN_FILE
	echo $DATE $SUCCESS_AUTORUNSTARTED >> $AUTORUN_LOG
else
        echo $DATE $ERROR_NOTFOUND >> $AUTORUN_LOG
fi

