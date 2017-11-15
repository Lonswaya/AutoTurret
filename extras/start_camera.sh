pivid -o - -t 0 -n -vf | cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554/stream}' :demux=h264
