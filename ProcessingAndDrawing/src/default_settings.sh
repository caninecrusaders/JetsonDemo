#!/bin/bash
v4l2-ctl -d /dev/video$1 \
--set-ctrl brightness=133 \
--set-ctrl contrast=5 \
--set-ctrl saturation=83 \
--set-ctrl white_balance_temperature_auto=1 \
--set-ctrl white_balance_temperature=4500 \
--set-ctrl power_line_frequency=2 \
--set-ctrl sharpness=25 \
--set-ctrl backlight_compensation=0 \
--set-ctrl exposure_auto=1 \
--set-ctrl exposure_absolute=156
# --set-ctrl white_balance_temperature=9000 \
# --set-ctrl exposure_absolute=5
# --set-ctrl brightness=100 \