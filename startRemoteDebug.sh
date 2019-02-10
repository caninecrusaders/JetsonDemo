# Kill gdbserver if it's running
ssh nvidia@192.168.0.104 killall gdbserver &> /dev/null
# Compile myprogram and launch gdbserver, listening on port 9091
ssh \
  -L9091:localhost:9091 \
  nvidia@192.168.0.104 \
  "bash -l -c 'cd /4980/JetsonDemo/ProcessingAndDrawing && make clean && make all && make run'"