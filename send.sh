
export DEVKITPRO="/opt/devkitpro"
export DEVKITPPC=$DEVKITPRO"/devkitPPC"
export WIILOAD="tcp:192.168.2.3"

exec $DEVKITPPC/bin/wiiload pcsx.dol
