#/bin/bash

#################
#
# Input - Output for UPX
# $1 Input
# $2 Output
# $3 Config
################

cp $1 $2
/home/menendez/projects/upx/upx-3.91-srcBubbles/src/upx.out $2 $3
#/media/bigdisk/projects/upx/upx-3.91-srcBubbles/src/upx.out $2 $3
