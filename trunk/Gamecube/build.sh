# For GameCube type "sh build.sh gc" or "./build.sh gc"
# To clean type "./build.sh clean"

export DEVKITPRO="/opt/devkitpro"
export DEVKITPPC=$DEVKITPRO"/devkitPPC"
export WIILOAD="tcp:192.168.2.3"

make $@ && exec /opt/devkitpro/devkitPPC/bin/wiiload pcsx.dol
