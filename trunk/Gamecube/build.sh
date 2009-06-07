# For GameCube type "sh build.sh gc" or "./build.sh gc"
# To clean type "./build.sh clean"

export DEVKITPRO="/opt/devkitpro"
export DEVKITPPC=$DEVKITPRO"/devkitPPC"


if [ $# -gt 0 ]
then
	if [ $1 = "clean" ]
	then
		make clean
		make -f Makefile.Wii clean
	else
		if [ $1 = "gc" ]
		then
			make
		fi
	fi
else
	make -f Makefile.Wii
fi

cp -v pcsx.dol /media/disk-2/apps/0dev_wiisx/boot.dol
