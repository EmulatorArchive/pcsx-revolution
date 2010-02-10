# For GameCube type "sh build.sh gc" or "./build.sh gc"
# To clean type "./build.sh clean"

export DEVKITPRO="/opt/devkitpro"
export DEVKITPPC=$DEVKITPRO"/devkitPPC"
export WIILOAD="tcp:192.168.2.3"

make -f Makefile.gekko $@ && exec $DEVKITPPC/bin/wiiload ./Gamecube/pcsx.dol
