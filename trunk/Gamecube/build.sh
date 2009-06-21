# For GameCube type "sh build.sh gc" or "./build.sh gc"
# To clean type "./build.sh clean"

export DEVKITPRO="/opt/devkitpro"
export DEVKITPPC=$DEVKITPRO"/devkitPPC"

make $@

cp -v pcsx.dol /media/disk-2/apps/0dev_wiisx/boot.dol
