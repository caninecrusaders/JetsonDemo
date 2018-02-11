#!/bin/bash
v4l2-ctl -d /dev/video0 \
--set-ctrl contrast=200 \
--set-ctrl saturation=200 \
--set-ctrl gain=0 \
--set-ctrl exposure_absolute=252 
